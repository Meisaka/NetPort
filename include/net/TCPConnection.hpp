#pragma once

#include "network_common.hpp"

namespace network {
	class TCPConnection : public NetworkStream, Socket {
	public:
		TCPConnection(void);
		TCPConnection(socket_t, int);
		TCPConnection(socket_t, int, CONNECTIONSTATE);
		TCPConnection(const TCPConnection&) = delete;
		TCPConnection & operator=(const TCPConnection&) = delete;
		bool init(ADDRTYPE);
		~TCPConnection(void);

		bool bind(const NetworkAddress &);
		bool connect(const NetworkAddress &);
		bool listen(int);
		
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
		bool accept(TCPConnection *);
		bool select(bool rd, bool wr, bool er);
		bool select(bool rd, bool wr, bool er, long sec, long microsec);
		int send(const char *, int);
		int send(const std::string &);
		int recv(char *, int);
		int recv(std::string &, int);
		bool is_connected() const;
		bool is_listening() const;
		const NetworkAddress& remote() const { return raddr; }
	private:
		void CheckState();
		CONNECTIONSTATE state;
		bool bound;
		NetworkAddress laddr;
		NetworkAddress raddr;
	};
}
