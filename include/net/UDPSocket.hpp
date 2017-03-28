#pragma once

#include "network_common.hpp"

namespace net {
class UDPSocket : public Socket {
public:
	NETPORTEX UDPSocket();
	NETPORTEX ~UDPSocket();

	NETPORTEX bool init(ADDRTYPE af);
	NETPORTEX bool bind(const NetworkAddress &);
	NETPORTEX bool connect(const NetworkAddress &remote);

	NETPORTEX bool is_bound() const;

	NETPORTEX int send_to(const NetworkAddress &, const char *, size_t);
	NETPORTEX int send_to(const NetworkAddress &, const std::string &);
	NETPORTEX int recv_from(NetworkAddress &, char *, size_t);
	NETPORTEX int recv_from(NetworkAddress &, std::string &);
private:
	bool bound;
	NetworkAddress laddr;
};

} // namespace net
