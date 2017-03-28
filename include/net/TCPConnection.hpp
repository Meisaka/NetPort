#pragma once

#include "network_common.hpp"

namespace net {
class TCPConnection : public NetworkStream, public Socket {
public:
	NETPORTEX TCPConnection(void);
	TCPConnection(const TCPConnection&) = delete;
	TCPConnection & operator=(const TCPConnection&) = delete;
	NETPORTEX TCPConnection(TCPConnection &&);
	NETPORTEX TCPConnection & operator=(TCPConnection &&);
	NETPORTEX ~TCPConnection(void);

	// depricated
	NETPORTEX TCPConnection(socket_t, int);
	// depricated
	NETPORTEX TCPConnection(socket_t, int, CONNECTIONSTATE);

	// new socket with bind
	NETPORTEX TCPConnection(const NetworkAddress &);

	// these are more for specialized / internal use
	// existing bound or listening socket
	NETPORTEX TCPConnection(socket_t, const NetworkAddress &la, bool lsn);
	// existing connected socket
	NETPORTEX TCPConnection(socket_t, const NetworkAddress &la, const NetworkAddress &ra);

	// new tcp socket (unbound)
	NETPORTEX bool init(ADDRTYPE);

	// bind a socket
	NETPORTEX bool bind(const NetworkAddress &);

	// listen on socket
	NETPORTEX bool listen(int);

	// new tcp socket (unbound)
	NETPORTEX bool init(const NetworkAddress &);

	// connect, makes new socket if none exists
	NETPORTEX bool connect(const NetworkAddress &);

	// bind and listen
	NETPORTEX bool listen(const NetworkAddress &, int);

	/* Enable or disable TCP keepalive */
	NETPORTEX bool set_keepalive(bool);

	/* Enable or disable TCP keepalives and set options
	* options:
	* enable
	* time - before sending keepalives (seconds)
	* interval - time between keepalives (seconds)
	* probes - maximum number of keepalive to send before dropping the connection (Linux specific)
	*/
	NETPORTEX bool set_keepalive(bool, unsigned long, unsigned long, unsigned long);

	// true disables nagle
	NETPORTEX bool set_nodelay(bool enable);

	// accepts new connections
	NETPORTEX TCPConnection accept();

	// test socket for read, write, or error status using select(3)
	NETPORTEX int select(bool rd, bool wr, bool er) const;
	// test socket for read, write, or error status with timeout
	NETPORTEX int select(bool rd, bool wr, bool er, long sec, long microsec) const;

	NETPORTEX int send(const char *, int);
	NETPORTEX int send(const std::string &);
	NETPORTEX int recv(char *, int);
	NETPORTEX int recv(std::string &, int);

	// true for received data or new incomming connection waiting
	NETPORTEX bool is_available() const;

	NETPORTEX bool is_connected() const;
	NETPORTEX bool is_listening() const;

	// address of remote (connected sockets)
	const NetworkAddress& remote() const { return raddr; }
private:
	void CheckState();
	CONNECTIONSTATE state;
	bool bound;
	NetworkAddress laddr;
	NetworkAddress raddr;
};

} // namespace net
