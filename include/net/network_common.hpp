#pragma once
#ifndef INCL_NETWORKCOM
#define INCL_NETWORKCOM

#include <string>
#include <memory>
#include <stdint.h>

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

namespace net {
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
NETPORTEX int send(socket_t handle, const char *, int);
/* receive data from a native socket */
NETPORTEX int recv(socket_t handle, char *, int);
/* close a native socket handle */
NETPORTEX void close(socket_t &handle);

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
	NETPORTEX socket_t get_handle() const { return handle; }
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

enum QUERYTYPE : uint32_t {
	QUERY_READ = 1 << 0,
	QUERY_WRITE = 1 << 1,
	QUERY_OOB = 1 << 2,
	QUERY_PRI = 1 << 3,
	// returned only
	QUERY_ERROR = 1 << 4,
	// returned only
	QUERY_HUP = 1 << 5,
	// returned only
	QUERY_INVAL = 1 << 6,
};

class QueryCallback {
public:
	virtual void Event(Socket &, QUERYTYPE type) = 0;
};

class INetworkQuery;
class NetworkQuery {
public:

	NETPORTEX NetworkQuery();
	NETPORTEX ~NetworkQuery();
	NETPORTEX void Add(Socket &, QUERYTYPE type, QueryCallback * eco = nullptr);
	NETPORTEX void Remove(Socket &);
	NETPORTEX void Update();
	NETPORTEX int QueryBlock();
	NETPORTEX int QueryWait(int msec);
protected:
	std::unique_ptr<INetworkQuery> mtc;
};

typedef struct NetworkAddress {
public:
	struct net_sockaddr addr;
	ADDRTYPE af;
	int type;
	int proto;
	NETPORTEX NetworkAddress();
	NETPORTEX NetworkAddress(const std::string &host);
	NETPORTEX NetworkAddress(const std::string &host, unsigned short p);
	NETPORTEX NetworkAddress(const std::string &host, const std::string &service);

	NETPORTEX void resolve(const std::string &host);
	NETPORTEX void resolve(const std::string &host, const std::string &service);
	NETPORTEX void port(unsigned short p);
	NETPORTEX void ip4(const char *);
	NETPORTEX void ip4(const std::string &);
	NETPORTEX void ip4(const char *, unsigned short p);
	NETPORTEX void ip4(const std::string &, unsigned short p);
	NETPORTEX void ip4(unsigned char i1, unsigned char i2, unsigned char i3, unsigned char i4);
	NETPORTEX void ip4(unsigned long i);

	NETPORTEX void ip6(const std::string &);
	NETPORTEX void ip6(const std::string &, unsigned short p);
	NETPORTEX std::string to_string() const;
	NETPORTEX int length() const;
	NETPORTEX bool operator<(const net::NetworkAddress& rh) const;
} address;

static int arc4_stir(void);
uint32_t arc4random(void);
uint32_t arc4random_uniform(uint32_t upper_bound);

} // namespace net

namespace std {
template<>
struct less<net::NetworkAddress>
	: public binary_function<net::NetworkAddress, net::NetworkAddress, bool>
{	// functor for operator<
	NETPORTEX bool operator()(const net::NetworkAddress& _Left, const net::NetworkAddress& _Right) const {
		return _Left < _Right;
	}
};
} // namespace std

#endif
