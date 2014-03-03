#pragma once
#ifndef INCL_NETWORK
#define INCL_NETWORK

#include "network_common.h"
#include "TCPConnection.h"
#include "UDPSocket.hpp"

namespace network {
	void Initialize();
	int sleep(int msec);
}
#endif
