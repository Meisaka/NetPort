#pragma once

#include "network_common.hpp"

namespace network {
	class UDPSocket : public Socket {
	public:
		UDPSocket();
		~UDPSocket();

		bool init(ADDRTYPE af);
		bool bind(NetworkAddress &);

		bool is_bound() const;

		int send_to(const NetworkAddress &, const char *, size_t);
		int send_to(const NetworkAddress &, const std::string &);
		int recv_from(NetworkAddress &, char *, size_t);
		int recv_from(NetworkAddress &, std::string &);
	private:
		bool bound;
		NetworkAddress laddr;
	};
}
