#pragma once

#include "network_common.hpp"
#include "UDPSocket.hpp"

namespace network {

	class MEIEvent {
	public:
		MEIEvent();
		~MEIEvent();
	};

	class IMEIControl;

	class MEISystem
	{
		friend class IMEIControl;
	public:
		MEISystem();
		~MEISystem();

		bool Init(ADDRTYPE);
		bool Bind(const NetworkAddress &);
		bool Listen();
		void Close();

		//bool Connect(const NetworkAddress &);

		bool IsListening();

		void ProcessReceive();
		void ProcessControl();

	private:
		unsigned long systemid;
	};

}
