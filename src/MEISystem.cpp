
#include "config.h"
#include "net/MEISystem.hpp"
#include "net/MEISocket.hpp"
#include "IMEI.hpp"

#include <iostream>
#include <stdio.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <memory>
#include <list>
#include <forward_list>
#include <map>

namespace net {

struct IMEI_ECO : public QueryCallback {
	MEISystem *sys;
	IMEI_ECO(MEISystem *s) : sys(s) {}

	void Event(Socket &, QUERYTYPE type) {
		if(type & QUERYTYPE::QUERY_READ) sys->ProcessReceive();
	}
};

class IMEIControl
{
public:
	static IMEIControl& instance();
	~IMEIControl();

	int InitSystem(MEISystem *, ADDRTYPE af);
	void DeinitSystem(MEISystem *);
	bool LookupSystem(unsigned long id);
	void ProcessReceives();
	void ProcessControl();
private:
	IMEIControl();

	std::atomic<bool> runctlthread;
	std::atomic<bool> runrecvthread;
	std::thread receivethread;
	std::thread controlthread;
	std::atomic<int> controlmod;
	std::mutex receivelock;
	std::mutex controllock;
	std::forward_list<IMEI_ECO> systems;
	NetworkQuery systemquery;
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
	bool access = true;
	receivelock.lock();
	while(runrecvthread) {
		if(controlmod.load() && access) {
			access = false;
			receivelock.unlock();
		}
		if(!controlmod.load() && !access) {
			receivelock.lock();
			access = true;
		}
		if(access) {
			systemquery.Update();
			systemquery.QueryWait(1);
		}
	}
	receivelock.unlock();
	std::cout << "Receive thread has ended\n";
}
// calculate time based values, manage connections, and send control messages.
void IMEIControl::ProcessControl()
{
	std::cout << "Control thread has started\n";
	int wait;
	wait = 0;
	bool access = true;
	controllock.lock();
	while(runctlthread) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		if(controlmod.load() && access) {
			access = false;
			controllock.unlock();
		}
		if(!controlmod.load() && !access) {
			controllock.lock();
			access = true;
		}
		if(access) {
			auto sysitr = systems.begin();
			auto sysitre = systems.end();
			while(sysitr != sysitre) {
				sysitr->sys->ProcessControl();
				sysitr++;
			}
			wait++;
		}
		if(wait > 100) {
			//std::cout << "Control thread tick\n";
			wait -= 100;
		}
	}
	controllock.unlock();
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
	controlmod.store(0);
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
	std::this_thread::yield();
	receivelock.lock();
	controllock.lock();
	if(receivethread.joinable()) {
		receivethread.detach();
	}
	if(controlthread.joinable()) {
		controlthread.detach();
	}
	receivelock.unlock();
	controllock.unlock();
}

int IMEIControl::InitSystem(MEISystem *sys, ADDRTYPE af)
{
	controlmod++;
	controllock.lock();
	receivelock.lock();
	auto sysitr = systems.cbegin();
	auto sysitre = systems.cend();
	while(sysitr != sysitre) {
		if(sysitr->sys == sys) return -1;
		sysitr++;
	}
	systems.push_front(IMEI_ECO(sys));
	systemquery.Add(sys->udpsck, QUERYTYPE::QUERY_READ, &systems.front());
	receivelock.unlock();
	controllock.unlock();
	controlmod--;
	return 0;
}
void IMEIControl::DeinitSystem(MEISystem *sys)
{
	controlmod++;
	controllock.lock();
	receivelock.lock();
	auto sysitr = systems.begin();
	auto sysitrl = systems.before_begin();
	auto sysitre = systems.end();
	while(sysitr != sysitre) {
		if(sysitr->sys == sys) {
			systemquery.Remove(sys->udpsck);
			sysitr = systems.erase_after(sysitrl); continue;
		}
		sysitrl = sysitr++;
	}
	receivelock.unlock();
	controllock.unlock();
	controlmod--;
}
bool IMEIControl::LookupSystem(unsigned long id)
{
	return false;
}

class IMEISystem {
public:
	std::map<uint32_t, MEISocket*> mim;
	std::map<std::tuple<NetworkAddress, uint32_t>, MEISocket*> pas;
	typedef std::map<uint32_t, MEISocket*>::iterator mim_iterator;
	std::forward_list<std::weak_ptr<IMEITransfer>> mis;
};

MEISystem::MEISystem() {
	systemid = 0;
	mtc.reset(new IMEISystem());
}
MEISystem::~MEISystem() {
	IMEIControl::instance().DeinitSystem(this);
}
bool MEISystem::init(ADDRTYPE af) {
	udpsck.init(af);
	this->systemid = IMEIControl::instance().InitSystem(this, af);
	return true;
}
bool MEISystem::bind(const NetworkAddress &ba) {
	return udpsck.bind(ba);
}
void MEISystem::ProcessReceive()
{
	NetworkAddress a;
	Packet cr;
	cr.alloc();
	int r;
	r = udpsck.recv_from(a, (char*)cr.ptr, sizeof(PacketData));
	if(r > 0) {
		cr.pksz = r;
		std::cerr << "MEI: got data " << r << " from " << a.to_string() << '\n';
		PacketParsedHeader ph = cr.fetch<PacketParsedHeader>();
		cr.rewind();
		if(ph.code == 0xf01 && MEIF_ISCONTROL(ph.flags)) {
			auto mi = mtc->pas.find(std::tuple<NetworkAddress, uint32_t>(a, ph.session));
			MEISocket *nses;
			std::shared_ptr<IMEITransfer> ntr;
			if(mi != mtc->pas.end()) {
				std::cerr << "MEI: repeat session request " << ph.session << " from " << a.to_string() << '\n';
				nses = mi->second;
				ntr = nses->mtc;
			} else {
				std::cerr << "MEI: new session request " << ph.session << " from " << a.to_string() << '\n';
				nses = new MEISocket();
				ntr = nses->mtc;
				ntr->sys = this;
				ntr->remote = a;
				ntr->lsess = arc4random();
				ntr->rsess = ph.session;
				mtc->mis.push_front(nses->mtc);
				mtc->mim.insert(std::make_pair(ntr->lsess, nses));
				mtc->pas.insert(std::make_pair(std::tuple<NetworkAddress, uint32_t>(a, ph.session), nses));
			}
			ntr->HandleRecv(cr);
		} else {
			IMEISystem::mim_iterator mi = mtc->mim.find(ph.session);
			if(mi != mtc->mim.end()) {
				mi->second->mtc->HandleRecv(cr);
			} else {
				std::cerr << "MEI: NA session " << ph.session << " from " << a.to_string() << '\n';
			}
		}
	}
}

void MEISystem::ProcessControl()
{
	auto sitr = mtc->mis.begin();
	auto sitre = mtc->mis.end();
	while(sitr != sitre) {
		auto ptr = (*sitr).lock();
		if(ptr) {
			ptr->HandleInterval();
		}
		sitr++;
	}
}

void MEISystem::close() {
	udpsck.close();
}

bool MEISystem::connect(const NetworkAddress &a, MEISocket &cnt) {
	std::shared_ptr<IMEITransfer> ntr = cnt.mtc;
	ntr->remote = a;
	ntr->sys = this;
	uint32_t lsid = arc4random();
	ntr->lsess = lsid;
	mtc->mis.push_front(ntr);
	mtc->mim.insert(std::make_pair(lsid, &cnt));
	ntr->connect();
	return true;
}

} // namespace net
