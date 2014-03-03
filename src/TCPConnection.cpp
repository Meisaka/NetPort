#include "net/TCPConnection.h"
#include "network_os.h"

namespace network {
	TCPConnection::TCPConnection(void) : bound(false), state(SCS_CLOSED), handle(0), af(0) {}

	TCPConnection::TCPConnection(int hndl, int afn) : bound(false), state(SCS_CLOSED), handle(hndl), af(afn)
	{
		TCPConnection::CheckState();
	}

	void TCPConnection::CheckState()
	{
		if(!handle) { return; }
		unsigned long i;
		socklen_t l = sizeof(unsigned long);
		if(!getsockopt(handle, SOL_SOCKET, 0x00700c, (char*)&i, &l)) {
			state = SCS_CONNECTED;
		}
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
	bool TCPConnection::SetNonBlocking()
	{
		unsigned long iMode = 1;
		if(!handle) { return false; }
	#ifdef WIN32
		ioctlsocket(handle, FIONBIO, &iMode);
	#else
		ioctl(handle, FIONBIO, &iMode);
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
		if(i = select(handle+1, (rd ? &fd : NULL), (wr ? &fd : NULL), (er ? &fd : NULL), NULL)) {
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
		if(that->state != SCS_LISTEN) { return false; }
		int h;
		socklen_t sas = sizeof(sockaddr);
		this->laddr = that->laddr;
		this->raddr.af = that->laddr.af;
		h = accept(that->handle, (struct sockaddr*)&this->raddr.addr, &sas);
		if(h == INVALID_SOCKET) {
			return false;
		}
		this->handle = h;
		this->state = SCS_CONNECTED;
		this->bound = true;
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
		Close();
	}
}