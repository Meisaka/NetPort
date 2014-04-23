#pragma once
#ifndef INCL_NETWORK
#define INCL_NETWORK

#include "network_common.hpp"
#include "TCPConnection.hpp"
#include "UDPSocket.hpp"

namespace network {
	NETPORTEX void initialize();
	NETPORTEX int sleep(int msec);

	struct TimeValue {
		unsigned long seconds;
		unsigned long nanoseconds;
	};
	NETPORTEX void get_time(TimeValue &);
}
#endif
