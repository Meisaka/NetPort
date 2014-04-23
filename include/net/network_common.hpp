#pragma once
#ifndef INCL_NETWORKCOM
#define INCL_NETWORKCOM

#include <string>

#ifdef NETPORT_SHARED
	#if defined(_MSC_VER)
		#define NETPORTEX __declspec(dllexport)
	#elif defined(__GNUC__) || defined(__clang__)
		#define NETPORTEX __attribute__((visibility("default")))
	#else
		#define NETPORTEX
	#endif
#else
	#define NETPORTEX
#endif

namespace network {
#ifdef WIN32
#if defined(_WIN64)
	typedef unsigned __int64 socket_t;
#else
	typedef unsigned int socket_t;
#endif
#else
	typedef int socket_t;
#endif
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
		char sa_data[48];
	};

	class INetworkStream {
	public:
		INetworkStream() {}
		virtual ~INetworkStream() {}
		virtual int Send(const char *, int) = 0;
		virtual int Send(const std::string &) = 0;
		virtual int Recv(char *, int) = 0;
		virtual int Recv(std::string &, int) = 0;
	};

	struct NetworkAddress {
	public:
		struct net_sockaddr addr;
		ADDRTYPE af;
		void Port(unsigned short p);
		void IP4(const char *);
		void IP4(const std::string &);
		void IP4(const char *, unsigned short p);
		void IP4(const std::string &, unsigned short p);
		void IP4(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4);
		void IP4(unsigned long i);
		//void IP6(const char *);
		void IP6(const std::string &);
		//void IP6(const char *, unsigned short p);
		void IP6(const std::string &, unsigned short p);
		std::string ToString() const;
		int Length() const;
	};
	typedef struct NetworkAddress Address;
}
#endif
