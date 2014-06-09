#include "net/TCPConnection.hpp"
#include "network_os.h"

namespace network {
    template<>
	TCPConnection TCPConnection::accept<TCPConnection>()
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

    template<>
	socket_t TCPConnection::accept<socket_t>()
	{
		if(this->state != SCS_LISTEN) { return INVALID_SOCKET; }
		socket_t h;
		socklen_t sas = sizeof(sockaddr);
		address radd;
		h = ::accept(this->handle, (struct sockaddr*)&radd.addr, &sas);
		return h;
	}
}