
#include "config.h"
#ifdef CPPELEVEN
#include "net/MEISystem.hpp"
#else
#include "net/MEISystem.h"
#endif

#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <memory>

namespace net {

	class IMEIControl
	{
	public:
		static unsigned long InitSystem(unsigned long id, ADDRTYPE af);
		static void DeinitSystem(unsigned long id);
		static IMEIControl& instance();
		~IMEIControl();

		bool LookupSystem(unsigned long id);
		void ProcessReceives();
		void ProcessControl();
	private:
		IMEIControl();

		std::atomic<bool> runctlthread;
		std::atomic<bool> runrecvthread;
		std::thread receivethread;
		std::thread controlthread;
	};

	static std::unique_ptr<IMEIControl> imei;

	IMEIControl & IMEIControl::instance()
	{
		if(imei) {
			return *imei.get();
		} else {
			imei.reset(new IMEIControl());
			return *imei.get();
		}
	}
	// receive and process packets (generates receive events)
	void IMEIControl::ProcessReceives()
	{
		std::cout << "Receive thread has started\n";
		while(runrecvthread) {
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
		std::cout << "Receive thread has ended\n";
	}
	// calculate time based values, manage connections, and send control messages.
	void IMEIControl::ProcessControl()
	{
		std::cout << "Control thread has started\n";
		while(runctlthread) {
			std::this_thread::sleep_for(std::chrono::milliseconds(250));
			std::cout << "Control thread tick\n";
		}
		std::cout << "Control thread has ended\n";
	}
	void MEIProcessReceive(IMEIControl *session)
	{
		session->ProcessReceives();
		std::terminate();
	}
	void MEIProcessControl(IMEIControl *session)
	{
		session->ProcessControl();
		std::terminate();
	}

	IMEIControl::IMEIControl()
	{
		runrecvthread = true;
		runctlthread = true;
#ifdef CPPELEVEN
		receivethread = std::thread(MEIProcessReceive, this);
		controlthread = std::thread(MEIProcessControl, this);
#else
#pragma message("not implemented")
#endif
	}

	IMEIControl::~IMEIControl()
	{
		//trans.Close();
		runrecvthread = false;
		runctlthread = false;
#ifdef CPPELEVEN
		std::this_thread::yield();
		if(receivethread.joinable()) {
			receivethread.detach();
		}
		if(controlthread.joinable()) {
			controlthread.detach();
		}
#else
#endif
	}

	unsigned long IMEIControl::InitSystem(unsigned long id, ADDRTYPE af)
	{
		if(IMEIControl::instance().LookupSystem(id)) {

		} else {
		}
		return 0;
	}
	void IMEIControl::DeinitSystem(unsigned long id)
	{

	}
	bool IMEIControl::LookupSystem(unsigned long id)
	{
		return false;
	}

	MEISystem::MEISystem() {
		systemid = 0;
	}
	MEISystem::~MEISystem() {
		IMEIControl::DeinitSystem(systemid);
	}
	bool MEISystem::init(ADDRTYPE af) {
		this->systemid = IMEIControl::InitSystem(systemid, af);
		return true;
	}

	void MEISystem::ProcessReceive()
	{
	}

	void MEISystem::ProcessControl()
	{
	}
}
