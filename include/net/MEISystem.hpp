#pragma once

#include "network_common.hpp"
#include "UDPSocket.hpp"

namespace net {

	class MEIEvent {
	public:
		NETPORTEX MEIEvent();
		NETPORTEX ~MEIEvent();
	};

	class IMEIControl;

	class MEISystem
	{
		friend class IMEIControl;
	public:
		NETPORTEX MEISystem();
		NETPORTEX ~MEISystem();

		NETPORTEX bool init(ADDRTYPE);
		NETPORTEX bool bind(const NetworkAddress &);
		NETPORTEX bool listen();
		NETPORTEX void close();

		//bool Connect(const NetworkAddress &);

		NETPORTEX bool is_listening();

		NETPORTEX void ProcessReceive();
		NETPORTEX void ProcessControl();

	private:
		unsigned long systemid;
	};

}
