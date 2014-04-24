#pragma once

#include "network_common.hpp"

namespace network {
	class TCPConnection : public NetworkStream, public Socket {
	public:
		TCPConnection(void);
		TCPConnection(const TCPConnection&) = delete;
		TCPConnection & operator=(const TCPConnection&) = delete;
		TCPConnection(TCPConnection &&) = default;
		TCPConnection & operator=(TCPConnection &&) = default;
		~TCPConnection(void);

		TCPConnection(socket_t, int); // depricated
		TCPConnection(socket_t, int, CONNECTIONSTATE); // depricated

		TCPConnection(const NetworkAddress &); // new socket with bind

		// these are more for specialized / internal use
		// existing bound or listening socket
		TCPConnection(socket_t, const NetworkAddress &la, bool lsn);
		// existing connected socket
		TCPConnection(socket_t, const NetworkAddress &la, const NetworkAddress &ra);

		bool init(ADDRTYPE); // new tcp socket (unbound)
		bool bind(const NetworkAddress &); // bind a socket
		bool listen(int); // listen on socket

		bool connect(const NetworkAddress &); // connect, makes new socket if none exists
		bool listen(const NetworkAddress &, int); // bind and listen
		
		/* Enable or disable TCP keepalive */
		bool set_keepalive(bool);

		/* Enable or disable TCP keepalives and set options
		* options:
		* enable
		* time - before sending keepalives (seconds)
		* interval - time between keepalives (seconds)
		* probes - maximum number of keepalive to send before dropping the connection (Linux specific)
		*/
		bool set_keepalive(bool, unsigned long, unsigned long, unsigned long);

		// true disables nagle
		bool set_no_delay(bool enable);

		// accepts new connections
		TCPConnection && accept();
		bool select(bool rd, bool wr, bool er) const;
		bool select(bool rd, bool wr, bool er, long sec, long microsec) const;

		int send(const char *, int);
		int send(const std::string &);
		int recv(char *, int);
		int recv(std::string &, int);

		bool is_available() const; // true for received data or new incomming connection waiting
		bool is_connected() const;
		bool is_listening() const;

		// address of remote (connected sockets)
		const NetworkAddress& remote() const { return raddr; }
	private:
		void CheckState();
		CONNECTIONSTATE state;
		bool bound;
		NetworkAddress laddr;
		NetworkAddress raddr;
	};
}
