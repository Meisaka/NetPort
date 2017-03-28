
#include "net/MEISocket.hpp"
#include <deque>
#include <list>

namespace net {

struct IPacketRef {
	uint32_t pksz;
	PacketData *ptr;
	uint32_t wait;
	uint32_t attempt;
	uint64_t seq;

	bool operator==(const IPacketRef & that) {
		return this == &that;
	}
	IPacketRef(Packet &p, uint64_t sq) {
		pksz = p.pksz;
		ptr = p.ptr;
		attempt = 0;
		wait = 0;
		seq = sq;
		p.pksz = 0;
		p.ptr = nullptr;
	}
	IPacketRef(IPacketRef const &) = delete;
	IPacketRef& operator=(IPacketRef const &) = delete;
	IPacketRef& operator=(IPacketRef &&p) {
		pksz = p.pksz;
		ptr = p.ptr;
		attempt = p.attempt;
		wait = p.wait;
		seq = p.seq;
		p.pksz = 0;
		p.ptr = nullptr;
		return *this;
	}
	IPacketRef(IPacketRef &&p) {
		*this = std::move(p);
	}
};

class IMEITransfer {
public:
	MEISystem *sys;
	address remote;
	bool controlpending;
	bool ackpending;
	uint32_t attempt;
	uint64_t framewait;
	MEISocket::ConnectionState cstate;
	uint32_t lsess;
	uint32_t rsess;
	std::list<IPacketRef> rt;
	uint32_t lseq;
	uint32_t lseqh;
	uint32_t rseq;
	uint32_t rseqh;

public:
	IMEITransfer() {
		lseq = arc4random();
		lseqh = arc4random();
		sys = nullptr;
	}
	~IMEITransfer() {
	}
	uint64_t seqlocal() {
		uint64_t r = (((uint64_t)lseqh) << 32) | lseq;
		if(!++lseq) ++lseqh;
		return r;
	}
	uint64_t cseqlocal() const {
		uint64_t r = (((uint64_t)lseqh) << 32) | lseq;
		return r;
	}
	uint64_t cpseqlocal(uint32_t t) const {
		uint64_t r = (((uint64_t)lseqh) << 32) | t;
		return r;
	}
	uint64_t cseqremote() const {
		uint64_t r = (((uint64_t)rseqh) << 32) | rseq;
		return r;
	}
	uint64_t cpseqremote(uint32_t t) const {
		uint64_t r = (((uint64_t)rseqh) << 32) | t;
		return r;
	}

	int send(IPacketRef &);
	int send(Packet &);
	void connect();
	void HandleInterval();
	void HandleRecv(Packet &pem);
	void Enter(MEISocket::ConnectionState c);
	void EnterW(MEISocket::ConnectionState c, uint32_t ms);

};

} // namespace net
