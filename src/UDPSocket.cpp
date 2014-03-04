
#include "network_os.h"
#include "net/UDPSocket.hpp"

namespace network {
	UDPSocket::UDPSocket()  : bound(false), handle(0), af(NETA_UNDEF) { }
	UDPSocket::~UDPSocket() {
		Close();
	}

	bool UDPSocket::Init(ADDRTYPE afn) {
		if(handle) {
			Close();
		}
		this->af = afn;
		int sc;
		sc = socket(afn, SOCK_DGRAM, IPPROTO_UDP);
		if(INVALID_SOCKET == sc) {
			return false;
		}
		handle = sc;
		return true;
	}

	bool UDPSocket::Bind(NetworkAddress & local) {
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
	void UDPSocket::Close() {
		if(!handle) { return; }
	#ifdef WIN32
		closesocket(handle);
	#else
		close(handle);
	#endif
		handle = 0;
	}
	bool UDPSocket::IsBound() const {
		return this->bound;
	}
	bool UDPSocket::SetNonBlocking()
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

	int UDPSocket::SendTo(const NetworkAddress & r, const char * b, size_t l) {
		if(!handle) { return -1; }
		return sendto(handle, b, l, 0, (struct sockaddr*)&r.addr, r.Length());
	}
	int UDPSocket::SendTo(const NetworkAddress & r, const std::string & s) {
		if(!handle) { return -1; }
		return sendto(handle, s.data(), s.length(), 0, (struct sockaddr*)&r.addr, r.Length());
	}
	int UDPSocket::RecvFrom(NetworkAddress & r, char * b, size_t l) {
		if(!handle) { return -1; }
		r.af = this->af;
		socklen_t x = r.Length();
		int i = recvfrom(handle, b, l, 0, (struct sockaddr*)&r.addr, &x);
		if(i < 0) {
#ifdef WIN32
			if(WSAGetLastError() == WSAEWOULDBLOCK) {
#else
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
				return 0;
			}
#ifdef WIN32
			if(WSAGetLastError() == WSAEMSGSIZE) {
				return l;
			}
#endif
		}
		return i;
	}
	int UDPSocket::RecvFrom(NetworkAddress & r, std::string & s) {
		if(!handle) { return -1; }
		r.af = this->af;
		socklen_t x = r.Length();
		char f[2000]; // MTU of most networks is limited to 1500, so this should be enough space ;)
		int i = recvfrom(handle, f, 2000, 0, (struct sockaddr*)&r.addr, &x);
		if(i > 0) {
			s.assign(f, i);
		} else if(i < 0) {
#ifdef WIN32
			if(WSAGetLastError() == WSAEWOULDBLOCK) {
#else
			if(errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
				return 0;
			}
#ifdef WIN32
			if(WSAGetLastError() == WSAEMSGSIZE) {
				return 2000;
			}
#endif
		}
		return i;
	}
}
