#pragma once
#ifndef INCL_NETWORKCOM
#define INCL_NETWORKCOM

#include <string>

namespace network {
	enum ADDRTYPE {
		NETA_UNDEF= 0,
		NETA_IPv4 = 2,
		NETA_IPv6 = 23,
	};

	enum CONNECTIONSTATE {
		SCS_CLOSED = 0,
		SCS_LISTEN,
		SCS_CONNECTING,
		SCS_CONNECTED,
		SCS_REMCLOSING,
		SCS_ERROR,
	};

	struct net_sockaddr {
		unsigned short sa_family;
		char sa_data[14];
	};

	class INetworkStream {
	public:
		INetworkStream() {}
		virtual ~INetworkStream() {}
		virtual int Send(const char *, int) = 0;
		virtual int Recv(char *, int) = 0;
	};

	struct NetworkAddress {
	public:
		struct net_sockaddr addr;
		ADDRTYPE af;
		void Port(unsigned short p);
		void IP4(const char *);
		void IP4(unsigned char i1,unsigned char i2,unsigned char i3,unsigned char i4);
		void IP4(unsigned long i);
		std::string ToString() const;
		int Length() const;
	};
}
#endif
