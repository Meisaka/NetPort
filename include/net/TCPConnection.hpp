#pragma once

#include "network_common.hpp"

namespace network {
	class TCPConnection : public INetworkStream {
	public:
		TCPConnection(void);
		TCPConnection(socket_t, int);
		TCPConnection(socket_t, int, CONNECTIONSTATE);
		bool Init(ADDRTYPE);
		~TCPConnection(void);

		bool Bind(const NetworkAddress &);
		bool Connect(const NetworkAddress &);
		bool Listen(int);
		void Close();
		/* close an unassociated socket handle */
		static void Close(socket_t &);
		/* Enable or disable TCP keepalive */
		bool SetKeepAlive(bool);

		/* Enable or disable TCP keepalives and set options
		* options:
		* enable
		* time - before sending keepalives (seconds)
		* interval - time between keepalives (seconds)
		* probes - maximum number of keepalive to send before dropping the connection (Linux specific)
		*/
		bool SetKeepAlive(bool, unsigned long, unsigned long, unsigned long);
		bool SetNoDelay(bool enable);
		bool SetNonBlocking(bool enable);
		bool Accept(TCPConnection *);
		bool Select(bool rd, bool wr, bool er);
		bool Select(bool rd, bool wr, bool er, long sec, long microsec);
		// send data to unassociated socket
		static int Send(socket_t handle, const char *, int);
		int Send(const char *, int);
		int Send(const std::string &);
		// receive data from an unassociated socket
		static int Recv(socket_t handle, char *, int);
		int Recv(char *, int);
		int Recv(std::string &, int);
		bool IsConnected();
		bool IsListening();
		const NetworkAddress& GetRemote() { return raddr; }
		socket_t Handle() const { return handle; };
	private:
		void CheckState();
		CONNECTIONSTATE state;
		bool bound;
		NetworkAddress laddr;
		NetworkAddress raddr;
		int af;
		socket_t handle;
	};
}
