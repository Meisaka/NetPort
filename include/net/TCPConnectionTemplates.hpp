#ifndef TCPCONNECTIONTEMPLATES_HPP_INCLUDED
#define TCPCONNECTIONTEMPLATES_HPP_INCLUDED

#include "net/TCPConnection.hpp"

namespace network {
    extern template TCPConnection TCPConnection::accept<TCPConnection>();
    extern template socket_t TCPConnection::accept<socket_t>();
}

#endif // TCPCONNECTIONTEMPLATES_HPP_INCLUDED
