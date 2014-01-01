#pragma once

#include "network_common.h"

namespace network {
	class TCPConnection : public INetworkStream {
	public:
		TCPConnection(void);
		TCPConnection(int,int);
		void Init(ADDRTYPE af);
		~TCPConnection(void);

		bool Bind(const NetworkAddress &);
		bool Connect(const NetworkAddress &);
		bool Listen(int);
		void Close();
		bool SetNoDelay(bool enable);
		bool SetNonBlocking();
		bool Accept(TCPConnection *);
		bool Select(bool rd, bool wr, bool er);
		bool Select(bool rd, bool wr, bool er, long sec, long microsec);
		virtual int Send(const char *, int);
		virtual int Recv(char *, int);
		bool IsConnected();
		bool IsListening();
		const NetworkAddress& GetRemote() { return raddr; }
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
