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
	enum ADDRTYPE : unsigned short {
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

	/* send data to a native socket */
	int send(socket_t handle, const char *, int);
	/* receive data from a native socket */
	int recv(socket_t handle, char *, int);
	/* close a native socket handle */
	void close(socket_t &handle);
	/* make a native socket blocking/non-blocking */
	void set_nonblocking(socket_t,bool enable);

	class Socket {
	public:
		NETPORTEX Socket();
		NETPORTEX Socket(socket_t);
		NETPORTEX Socket(const Socket&) = delete;
		NETPORTEX Socket & operator=(const Socket&) = delete;
		NETPORTEX Socket(Socket &&);
		NETPORTEX Socket & operator=(Socket &&);
		NETPORTEX ~Socket();

		// true if valid socket handle
		NETPORTEX operator bool() const;

		NETPORTEX bool set_nonblocking(bool enable);
		NETPORTEX void close();
		socket_t get_handle() const { return handle; }
		NETPORTEX socket_t release_handle();
	protected:
		socket_t handle;
	private:
		unsigned long * rhcount;
	};

	class NetworkStream {
	public:
		NetworkStream() = default;
		virtual ~NetworkStream() = default;
		virtual int send(const char *, int) = 0;
		virtual int send(const std::string &) = 0;
		virtual int recv(char *, int) = 0;
		virtual int recv(std::string &, int) = 0;
	};

	typedef struct NetworkAddress {
	public:
		struct net_sockaddr addr;
		ADDRTYPE af;
		int type;
		int proto;
		NetworkAddress();
		NetworkAddress(const std::string &host);
		NetworkAddress(const std::string &host, unsigned short p);
		NetworkAddress(const std::string &host, const std::string &service);

		void resolve(const std::string &host);
		void resolve(const std::string &host, const std::string &service);
		void port(unsigned short p);
		void ip4(const char *);
		void ip4(const std::string &);
		void ip4(const char *, unsigned short p);
		void ip4(const std::string &, unsigned short p);
		void ip4(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4);
		void ip4(unsigned long i);

		void ip6(const std::string &);
		void ip6(const std::string &, unsigned short p);
		std::string to_string() const;
		int length() const;
	} address;

}
#endif
