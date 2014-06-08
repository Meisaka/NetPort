
#include "net/network.h"
#include "network_os.h"

namespace network {

	Socket::Socket()
	{
		rhcount = nullptr;
		handle = INVALID_SOCKET;
	}

	Socket::Socket(socket_t v)
	{
		handle = v;
		rhcount = new unsigned long;
		*rhcount = 1;
	}

	Socket::~Socket()
	{
		if(rhcount != nullptr) {
			if(--*rhcount == 0) {
				delete rhcount; rhcount = nullptr;
				Socket::close();
			}
		}
	}
/*
	Socket::Socket(const Socket &rs)
	{
		if(rs.rhcount != nullptr) {
			if(rhcount == nullptr) {
				rhcount = rs.rhcount;
				*rhcount++;
			} else {
				if(*rhcount-- == 0) {
					delete rhcount;
					Socket::close();
				}
				rhcount = rs.rhcount;
			}
			handle = rs.handle;
		} else {
		}
	}
	Socket & Socket::operator=(const Socket &rs)
	{
		if(rs.rhcount != nullptr) {
			if(rhcount == nullptr) {
				rhcount = rs.rhcount;
				*rhcount++;
			} else {
				if(*rhcount-- == 0) {
					delete rhcount;
					Socket::close();
				}
				rhcount = rs.rhcount;
			}
			handle = rs.handle;
		} else {
		}
		return *this;
	}
*/
	Socket::Socket(Socket &&rs)
	{
		if(rs.rhcount != nullptr) {
			if(rhcount == nullptr) {
				rhcount = rs.rhcount;
				rs.rhcount = nullptr;
			} else {
				delete rs.rhcount; rs.rhcount = nullptr;
			}
		} else {
		}
		handle = rs.handle;
		rs.handle = INVALID_SOCKET;
	}

	Socket & Socket::operator=(Socket &&rs)
	{
		if(rs.rhcount != nullptr) {
			if(rhcount == nullptr) {
				rhcount = rs.rhcount;
				rs.rhcount = nullptr;
			} else {
				delete rs.rhcount; rs.rhcount = nullptr;
			}
		} else {
		}
		handle = rs.handle;
		rs.handle = INVALID_SOCKET;
		return *this;
	}

	socket_t Socket::release_handle()
	{
		socket_t h = handle;
		if(rhcount != nullptr) {
			if(--*rhcount == 0) {
				delete rhcount; rhcount = nullptr;
			}
		}
		handle = INVALID_SOCKET;
		return h;
	}

	Socket::operator bool() const
	{
		return (handle != INVALID_SOCKET);
	}

	bool Socket::set_nonblocking(bool enable)
	{
		if(!*this) { return false; }
#ifdef WIN32
		unsigned long iMode = (enable ? 1 : 0);
		ioctlsocket(handle, FIONBIO, &iMode);
#else
		int flags = fcntl(handle, F_GETFL, 0);
		if(enable) {
			flags |= O_NONBLOCK;
		} else {
			flags ^= O_NONBLOCK;
		}
		fcntl(handle, F_SETFL, flags);
#endif
		return true;
	}

	void Socket::set_nonblocking(socket_t handle, bool enable)
	{
#ifdef WIN32
		unsigned long iMode = (enable ? 1 : 0);
		ioctlsocket(handle, FIONBIO, &iMode);
#else
		int flags = fcntl(handle, F_GETFL, 0);
		if(enable) {
			flags |= O_NONBLOCK;
		} else {
			flags ^= O_NONBLOCK;
		}
		fcntl(handle, F_SETFL, flags);
#endif
	}

	void Socket::close()
	{
		if(!*this) { return; }
		shutdown(handle, SHUT_RDWR);
#ifdef WIN32
		closesocket(handle);
#else
		::close(handle);
#endif
		handle = INVALID_SOCKET;
	}

}
