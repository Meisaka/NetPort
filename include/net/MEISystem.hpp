#pragma once

#include "network_common.hpp"
#include "UDPSocket.hpp"
#include <memory>

namespace net {

class MEIEvent {
public:
	NETPORTEX MEIEvent();
	NETPORTEX ~MEIEvent();
};

class IMEIControl;
class IMEITransfer;
class IMEISystem;
class MEISocket;

class MEISystem
{
	friend class IMEIControl;
	friend class IMEITransfer;
public:
	NETPORTEX MEISystem();
	NETPORTEX ~MEISystem();
	MEISystem(MEISystem&) = delete;
	MEISystem(MEISystem&&) = delete;
	MEISystem& operator=(MEISystem&) = delete;
	MEISystem& operator=(MEISystem&&) = delete;

	NETPORTEX bool init(ADDRTYPE);
	NETPORTEX bool bind(const NetworkAddress &);
	NETPORTEX bool listen();
	NETPORTEX void close();

	NETPORTEX bool MEISystem::connect(const NetworkAddress &, MEISocket &);

	NETPORTEX bool is_listening();

	NETPORTEX void ProcessReceive();
	NETPORTEX void ProcessControl();

private:
	net::UDPSocket udpsck;
	unsigned long systemid;
	std::unique_ptr<IMEISystem> mtc;
};

}
