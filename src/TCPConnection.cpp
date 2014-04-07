#include "net/TCPConnection.h"
#include "network_os.h"
#include "config.h"

namespace network {
	TCPConnection::TCPConnection(void) : bound(false), state(SCS_CLOSED), handle(0), af(0) {}

	TCPConnection::TCPConnection(int hndl, int afn) : bound(false), state(SCS_CLOSED), handle(hndl), af(afn)
	{
		TCPConnection::CheckState();
	}

	TCPConnection::TCPConnection(int hndl, int afn, CONNECTIONSTATE scs) : bound(false), state(scs), handle(hndl), af(afn)
	{
		TCPConnection::CheckState();
	}

	void TCPConnection::CheckState()
	{
		if(!handle) { return; }
#ifdef WIN32
		unsigned long i;
		socklen_t l = sizeof(unsigned long);
		if(!getsockopt(handle, SOL_SOCKET, 0x00700c, (char*)&i, &l)) {
			state = SCS_CONNECTED;
		}
#else
#endif
	}

	bool TCPConnection::SetNoDelay(bool enable)
	{
		unsigned long i = (enable ? 1 : 0);
		if(!handle) { return false; }
		if(setsockopt(handle, SOL_SOCKET, TCP_NODELAY, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
		return true;
	}

	bool TCPConnection::SetKeepAlive(bool enable)
	{
		unsigned long i = (enable ? 1 : 0);
		if(!handle) { return false; }
		if(setsockopt(handle, SOL_SOCKET, SO_KEEPALIVE, (char*)&i, sizeof(unsigned long))) {
			return false;
		}
		return true;
	}

	bool TCPConnection::SetKeepAlive(bool enable, unsigned long time, unsigned long intvl, unsigned long probes)
	{
		unsigned long i = (enable ? 1 : 0);
		if(!handle) { return false; }
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

	bool TCPConnection::SetNonBlocking(bool enable)
	{
		if(!handle) { return false; }
	#ifdef WIN32
		unsigned long iMode = (enable ? 1 : 0);
		ioctlsocket(handle, FIONBIO, &iMode);
	#else
		int flags = fcntl(handle, F_GETFL, 0);
		if (enable) {
			flags |= O_NONBLOCK;
		}
		else {
			flags ^= O_NONBLOCK;
		}
		fcntl(handle, F_SETFL, flags);
	#endif
		return true;
	}

	bool TCPConnection::Select(bool rd, bool wr, bool er)
	{
		if(!handle) { return false; }
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(handle, &fd);
		int i;
		struct timeval tv;
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		if(i = select(handle+1, (rd ? &fd : NULL), (wr ? &fd : NULL), (er ? &fd : NULL), &tv)) {
			if(i == -1) { return false; }
			return true;
		}
		return false;
	}
	int TCPConnection::Send(const char * buf, int buflen)
	{
		int i;
		if(!handle) { return -1; }
		if(state != SCS_CONNECTED) { return -1; }
		i = send(handle, buf, buflen, 0);
		if(i < 0) {
			TCPConnection::Close();
		}
		return i;
	}
	int TCPConnection::Send(const std::string &s)
	{
		int i;
		if(!handle) { return -1; }
		if(state != SCS_CONNECTED) { return -1; }
		i = send(handle, s.data(), s.length(), 0);
		if(i < 0) {
			TCPConnection::Close();
		}
		return i;
	}
	int TCPConnection::Recv(char * buf, int buflen)
	{
		int i;
		if(!handle) { return -1; }
		if(state != SCS_CONNECTED) { return -1; }
		i = recv(handle, buf, buflen, 0);
		if(i <= 0) {
#ifdef WIN32
			if(WSAGetLastError() == WSAEWOULDBLOCK) {
#else
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
				return 0;
			}
			TCPConnection::Close();
		}
		return i;
	}
	int TCPConnection::Recv(std::string &s, int buflen)
	{
		if(!handle) { return -1; }
		char *h = new char[buflen];
		int i = Recv(h, buflen);
		if(i > 0) {
			s.assign(h, i);
		}
		delete h;
		return i;
	}

	bool TCPConnection::IsConnected()
	{
		return (state == SCS_CONNECTED);
	}
	bool TCPConnection::IsListening()
	{
		return (state == SCS_LISTEN);
	}
	bool TCPConnection::Select(bool rd, bool wr, bool er, long sec, long microsec)
	{
		if(!handle) { return false; }
		struct timeval tv;
		tv.tv_sec = sec;
		tv.tv_usec = microsec;
		fd_set fd;
		FD_ZERO(&fd);
		FD_SET(handle,&fd);
		int i;
		if(i = select(handle+1, (rd ? &fd : NULL), (wr ? &fd : NULL), (er ? &fd : NULL), &tv)) {
			if(i == -1) { return false; }
			return true;
		}
		return false;
	}

	bool TCPConnection::Accept(TCPConnection * that)
	{
		if(this->state != SCS_LISTEN) { return false; }
		if(!that) { return false; }
		int h;
		socklen_t sas = sizeof(sockaddr);
		h = accept(this->handle, (struct sockaddr*)&that->raddr.addr, &sas);
		if(h == INVALID_SOCKET) {
			return false;
		}
		that->laddr = this->laddr;
		that->raddr.af = this->laddr.af;
		that->af = this->af;
		that->handle = h;
		that->state = SCS_CONNECTED;
		that->bound = true;
		return true;
	}

	bool TCPConnection::Init(ADDRTYPE afn)
	{
		if(handle) {
			Close();
		}
		this->af = afn;
		int sc;
		sc = socket(afn, SOCK_STREAM, IPPROTO_TCP);
		if(INVALID_SOCKET == sc) {
			return false;
		}

		handle = sc;
		state = SCS_CLOSED;
		return true;
	}

	bool TCPConnection::Bind(const NetworkAddress &local)
	{
		if(!handle) { return false; }
		long v = 1;
		if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(long))) {
			return false;
		}
		if(bind(handle, (struct sockaddr*)&local.addr, local.Length())) {
			return false;
		}
		laddr = local;
		bound = true;
		return true;
	}
	bool TCPConnection::Connect(const NetworkAddress &remote)
	{
		if(!handle) { return false; }
		long v = 1;
		if(setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, (char*)&v, sizeof(long))) {
			return false;
		}
		if(connect(handle, (struct sockaddr*)&remote.addr, remote.Length())) {
			return false;
		}
		raddr = remote;
		state = SCS_CONNECTED;
		bound = true;
		return true;
	}

	bool TCPConnection::Listen(int queue)
	{
		if(!handle) { return false; }
		if(!bound) { return false; }
		if(state != SCS_CLOSED) { return false; }
		if(listen(handle, queue)) {
			return false;
		}
		state = SCS_LISTEN;
		return true;
	}

	void TCPConnection::Close(int &h)
	{
		if(h == INVALID_SOCKET) { return; }
		shutdown(h, SHUT_RDWR);
	#ifdef WIN32
		closesocket(h);
	#else
		close(h);
	#endif
		h = 0;
	}
	void TCPConnection::Close()
	{
		if(!handle) { return; }
		if(state != SCS_CLOSED) {
			shutdown(handle, SHUT_RDWR);
		}
	#ifdef WIN32
		closesocket(handle);
	#else
		close(handle);
	#endif
		state = SCS_CLOSED;
		handle = 0;
	}

	TCPConnection::~TCPConnection(void)
	{
#ifndef COPYSAFE
		Close();
#endif
	}
}
