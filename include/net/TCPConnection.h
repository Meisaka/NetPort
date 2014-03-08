#pragma once

#include "network_common.h"

namespace network {
	class TCPConnection : public INetworkStream {
	public:
		TCPConnection(void);
		TCPConnection(int,int, CONNECTIONSTATE = SCS_CLOSED);
		bool Init(ADDRTYPE);
		~TCPConnection(void);

		bool Bind(const NetworkAddress &);
		bool Connect(const NetworkAddress &);
		bool Listen(int);
		void Close();
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
		int Send(const char *, int);
		int Send(const std::string &);
		int Recv(char *, int);
		int Recv(std::string &, int);
		bool IsConnected();
		bool IsListening();
		const NetworkAddress& GetRemote() { return raddr; }
		int Handle() const { return handle; };
	private:
		void CheckState();
		CONNECTIONSTATE state;
		bool bound;
		NetworkAddress laddr;
		NetworkAddress raddr;
		int af;
		int handle;
	};
}
