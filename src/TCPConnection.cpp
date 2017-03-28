#include "net/TCPConnection.hpp"
#include "network_os.h"
#include "config.h"

#include <iostream>

namespace net {
	TCPConnection::TCPConnection(void) : bound(false), state(SCS_CLOSED) {}

	TCPConnection::TCPConnection(socket_t hndl, int afn)
		: bound(false), state(SCS_CLOSED), Socket(hndl)
	{
		TCPConnection::CheckState();
	}

	TCPConnection::TCPConnection(TCPConnection && rc)
	{
		(Socket&)*this = (Socket&&)rc;
		state = rc.state; rc.state = SCS_CLOSED;
		laddr = rc.laddr;
		raddr = rc.raddr;
		bound = rc.bound; rc.bound = false;
	}
	TCPConnection & TCPConnection::operator=(TCPConnection && rc)
	{
		(Socket&)*this = (Socket&&)rc;
		state = rc.state; rc.state = SCS_CLOSED;
		laddr = rc.laddr;
		raddr = rc.raddr;
		bound = rc.bound; rc.bound = false;
		return *this;
	}

	TCPConnection::TCPConnection(const NetworkAddress &la)
	{
		bind(la);
	}

	TCPConnection::TCPConnection(socket_t hndl, int afn, CONNECTIONSTATE scs)
		: bound(false), state(scs), Socket(hndl)
	{
		TCPConnection::CheckState();
	}

	TCPConnection::TCPConnection(socket_t hndl, const NetworkAddress &la, bool lsn)
		: Socket(hndl)
	{
		bound = true;
		laddr = la;
		state = lsn ? SCS_LISTEN : SCS_CLOSED;
	}

	TCPConnection::TCPConnection(socket_t hndl, const NetworkAddress &la, const NetworkAddress &ra)
		: state(SCS_CONNECTED), Socket(hndl)
	{
		bound = true;
		laddr = la;
		raddr = ra;
	}

	void TCPConnection::CheckState()
	{
		if(handle == INVALID_SOCKET) { return; }
#ifdef WIN32
		unsigned long i;
		socklen_t l = sizeof(unsigned long);
		if(!getsockopt(handle, SOL_SOCKET, 0x00700c, (char*)&i, &l)) {
			state = SCS_CONNECTED;
		}
#else
#endif
	}

	bool TCPConnection::set_nodelay(bool enable)
	{
		unsigned long i = (enable ? 1 : 0);
		if(handle == INVALID_SOCKET) { return false; }
		//if(setsockopt(handle, SOL_SOCKET, TCP_NODELAY, (char*)&i, sizeof(unsigned long))) {
		if(setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
		return true;
	}

	bool TCPConnection::set_keepalive(bool enable)
	{
		unsigned long i = (enable ? 1 : 0);
		if(handle == INVALID_SOCKET) { return false; }
		if(setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
		return true;
	}

	bool TCPConnection::set_keepalive(bool enable, unsigned long time, unsigned long intvl, unsigned long probes)
	{
		unsigned long i = (enable ? 1 : 0);
		if(handle == INVALID_SOCKET) { return false; }
		if(setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
#ifdef WIN32
		tcp_keepalive tcpka_set;
		tcpka_set.onoff = i;
		// these values are expected in milliseconds
		tcpka_set.keepaliveinterval = intvl * 1000;
		tcpka_set.keepalivetime = time * 1000;
		// probes is not settable on Windows

		if(WSAIoctl(handle, SIO_KEEPALIVE_VALS, &tcpka_set, sizeof(tcp_keepalive), 0, 0, 0, 0, 0)) {
			return false;
		}
#else
#ifdef LINUX
		// These options are Linux specific (not portable)
		if(setsockopt(handle, SOL_TCP, TCP_KEEPIDLE, &time, sizeof(unsigned long))) {
			return false;
		}
		if(setsockopt(handle, SOL_TCP, TCP_KEEPCNT, &probes, sizeof(unsigned long))) {
			return false;
		}
		if(setsockopt(handle, SOL_TCP, TCP_KEEPINTVL, &intvl, sizeof(unsigned long))) {
			return false;
		}
#endif
#endif
		return true;
	}

	int TCPConnection::select(bool rd, bool wr, bool er) const
	{
		if(handle == INVALID_SOCKET) { return false; }
		fd_set fdr;
		fd_set fdw;
		fd_set fde;
		FD_ZERO(&fdr);
		FD_ZERO(&fdw);
		FD_ZERO(&fde);
		FD_SET(handle, &fdr);
		FD_SET(handle, &fdw);
		FD_SET(handle, &fde);
		int i;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
#ifdef WIN32
		if(i = ::select(0, (rd ? &fdr : NULL), (wr ? &fdw : NULL), (er ? &fde : NULL), &tv)) {
#else
		if(i = ::select(handle + 1, (rd ? &fdr : NULL), (wr ? &fdw : NULL), (er ? &fde : NULL), &tv)) {
#endif
			if(i == -1) { return 0; }
			return (FD_ISSET(handle, &fdr) ? 1 : 0) | (FD_ISSET(handle, &fdw) ? 2 : 0) | (FD_ISSET(handle, &fde) ? 4 : 0);
		}
		return 0;
	}
	
	int TCPConnection::send(const char * buf, int buflen)
	{
		int i;
		if(handle == INVALID_SOCKET) { return -1; }
		if(state != SCS_CONNECTED) { return -1; }
		i = ::send(handle, buf, buflen, 0);
		if(i < 0) {
			Socket::close();
		}
		return i;
	}
	int TCPConnection::send(const std::string &s)
	{
		int i;
		if(handle == INVALID_SOCKET) { return -1; }
		if(state != SCS_CONNECTED) { return -1; }
		i = ::send(handle, s.data(), s.length(), 0);
		if(i < 0) {
			Socket::close();
		}
		return i;
	}
	
	int TCPConnection::recv(char * buf, int buflen)
	{
		int i;
		if(handle == INVALID_SOCKET) { return -1; }
		if(state != SCS_CONNECTED) { return -1; }
		i = net::recv(handle, buf, buflen);
		if(i < 0) {
			Socket::close();
		}
		return i;
	}
	int TCPConnection::recv(std::string &s, int buflen)
	{
		if(handle == INVALID_SOCKET) { return -1; }
		s.resize(buflen);
		int i = TCPConnection::recv((char*)s.data(), buflen);
		if(i > 0) {
			s.resize(i);
		}
		return i;
	}

	bool TCPConnection::is_available() const
	{
		return (handle != INVALID_SOCKET) && select(true, false, false, 0, 0);
	}

	bool TCPConnection::is_connected() const
	{
		return (handle != INVALID_SOCKET) && (state == SCS_CONNECTED);
	}
	bool TCPConnection::is_listening() const
	{
		return (handle != INVALID_SOCKET) && (state == SCS_LISTEN);
	}
	int TCPConnection::select(bool rd, bool wr, bool er, long sec, long microsec) const
	{
		if(handle == INVALID_SOCKET) { return false; }
		struct timeval tv;
		tv.tv_sec = sec;
		tv.tv_usec = microsec;
		fd_set fdr;
		fd_set fdw;
		fd_set fde;
		FD_ZERO(&fdr);
		FD_ZERO(&fdw);
		FD_ZERO(&fde);
		FD_SET(handle, &fdr);
		FD_SET(handle, &fdw);
		FD_SET(handle, &fde);
		int i;
#ifdef WIN32
		if(i = ::select(0, (rd ? &fdr : NULL), (wr ? &fdw : NULL), (er ? &fde : NULL), &tv)) {
#else
		if(i = ::select(handle + 1, (rd ? &fdr : NULL), (wr ? &fdw : NULL), (er ? &fde : NULL), &tv)) {
#endif
			if(i == -1) { return 0; }
			return (FD_ISSET(handle, &fdr) ? 1 : 0) | (FD_ISSET(handle, &fdw) ? 2 : 0) | (FD_ISSET(handle, &fde) ? 4 : 0);
		}
		return 0;
	}

	TCPConnection TCPConnection::accept()
	{
		TCPConnection rv;
		if(this->state != SCS_LISTEN) { return rv; }
		socket_t h;
		socklen_t sas = sizeof(sockaddr);
		address radd;
		h = ::accept(this->handle, (struct sockaddr*)&radd.addr, &sas);
		if(h == INVALID_SOCKET) {
			return rv;
		}
		return TCPConnection(h, this->laddr, radd);
	}

	bool TCPConnection::init(ADDRTYPE afn)
	{
		if(handle != INVALID_SOCKET) {
			Socket::close();
		}
		this->laddr.af = afn;
		socket_t sc;
		sc = socket(afn, SOCK_STREAM, IPPROTO_TCP);
		if(INVALID_SOCKET == sc) {
			return false;
		}

		handle = sc;
		state = SCS_CLOSED;
		return true;
	}
	bool TCPConnection::init(const NetworkAddress &type)
	{
		if(handle != INVALID_SOCKET) {
			Socket::close();
		}
		this->laddr.af = type.af;
		socket_t sc;
		sc = socket(type.af, SOCK_STREAM, IPPROTO_TCP);
		if(INVALID_SOCKET == sc) {
			return false;
		}

		handle = sc;
		state = SCS_CLOSED;
		return true;
	}

	bool TCPConnection::bind(const NetworkAddress &local)
	{
		if(INVALID_SOCKET == handle) {
			if(!init(local.af)) {
				return false;
			}
		}
		long v = 1;
		if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(long))) {
			return false;
		}
		if(::bind(handle, (struct sockaddr*)&local.addr, local.length())) {
			return false;
		}
		laddr = local;
		bound = true;
		return true;
	}
	bool TCPConnection::connect(const NetworkAddress &remote)
	{
		if(handle == INVALID_SOCKET) {
			if(!init(remote.af)) {
				return false;
			}
		}
		long v = 1;
		if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(long))) {
			return false;
		}
		if(::connect(handle, (struct sockaddr*)&remote.addr, remote.length())) {
			return false;
		}
		raddr = remote;
		state = SCS_CONNECTED;
		bound = true;
		return true;
	}

	bool TCPConnection::listen(const NetworkAddress &local, int queue)
	{
		if(handle == INVALID_SOCKET || !bound) {
			if(!bind(local)) {
				return false;
			}
		}
		if(::listen(handle, queue)) {
			return false;
		}
		state = SCS_LISTEN;
		return true;
	}

	bool TCPConnection::listen(int queue)
	{
		if(handle == INVALID_SOCKET) { return false; }
		if(!bound) { return false; }
		if(state != SCS_CLOSED) { return false; }
		if(::listen(handle, queue)) {
			return false;
		}
		state = SCS_LISTEN;
		return true;
	}


	TCPConnection::~TCPConnection(void)
	{
	}
}
