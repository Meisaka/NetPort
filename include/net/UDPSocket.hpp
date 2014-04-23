#pragma once

#include "network_common.hpp"

namespace network {
	class UDPSocket {
	public:
		UDPSocket();
		~UDPSocket();

		bool Init(ADDRTYPE af);
		bool Bind(NetworkAddress &);
		void Close();
		bool SetNonBlocking(bool enable);
		bool IsBound() const;

		int SendTo(const NetworkAddress &, const char *, size_t);
		int SendTo(const NetworkAddress &, const std::string &);
		int RecvFrom(NetworkAddress &, char *, size_t);
		int RecvFrom(NetworkAddress &, std::string &);
	private:
		bool bound;
		NetworkAddress laddr;
		ADDRTYPE af;
		socket_t handle;
	};
}
