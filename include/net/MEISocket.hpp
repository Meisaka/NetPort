#pragma once

#include "network_common.hpp"
#include "UDPSocket.hpp"
#include <memory>

#define MEIF_DATA    (0x00U)
#define MEIF_CONTROL (0x40U)
#define MEIF_ISCONTROL(a)  (((a) & MEIF_CONTROL) != 0)

namespace net {

class MEISystem;
class IMEITransfer;
struct IPacketRef;

struct PacketHeader {
	uint32_t pflags;
	uint32_t session;
	uint32_t sequence;
};
struct PacketParsedHeader {
	uint32_t length;
	uint16_t flags;
	uint16_t code;
	uint32_t session;
	uint32_t sequence;
};

struct PacketData {
	uint8_t data[2048];
};
struct Packet {
	uint32_t pksz;
	uint32_t mark;
	uint32_t ch;
	PacketData *ptr;

	void alloc() {
		if(!ptr) {
			ptr = new PacketData();
		}
	}
	// clear the packet settings
	void reset() {
		alloc();
		memset(ptr, 0, sizeof(PacketData));
		mark = pksz = ch = 0;
	}
	void rewind() {
		mark = ch = 0;
	}

	Packet() : ptr(0), pksz(0), mark(0), ch(0) {
	}
	~Packet() {
		if(ptr) {
			PacketData *p = ptr;
			pksz = 0;
			ptr = nullptr;
			delete p;
		}
	}
	Packet(Packet const &) = delete;
	Packet& operator=(Packet const &) = delete;

	// insert value into the packet at the current position
	// and move forward by amount written.
	template<typename T> void insert(T const &t) {
		uint32_t nx;
		if(!ptr) return;
		nx = mark + sizeof(T);
		if(nx > (sizeof(PacketData) - 4)) return; // epic failure
		*((T*)(ptr->data + mark)) = t;
		mark = nx;
		if(mark > pksz) pksz = mark;
	}
	template<> void insert(PacketParsedHeader const &t) {
		uint32_t nx;
		if(!ptr) return;
		if(ch < mark) complete();
		nx = mark + sizeof(PacketHeader);
		if(nx >(sizeof(PacketData) - 4)) return; // epic failure
		PacketHeader *v = (PacketHeader*)(ptr->data + mark);
		v->pflags = ((t.code & 0xfff) << 20) | ((t.flags & 0x7f) << 13) | (t.length & 0x1fff);
		v->session = t.session;
		v->sequence = t.sequence;
		mark = nx;
		if(mark > pksz) pksz = mark;
	}
	// read <type> from the current position and move forward.
	template<class T> T fetch() {
		uint32_t nx;
		if(!ptr) return 0;
		nx = mark + sizeof(T);
		if(nx > pksz) return 0;
		T *v = (T*)(ptr->data + mark);
		mark = nx;
		return *v;
	}
	template<> PacketParsedHeader fetch() {
		uint32_t nx;
		if(!ptr) return {0, };
		nx = mark + sizeof(PacketHeader);
		if(nx > pksz) return {0, };
		PacketHeader *v = (PacketHeader*)(ptr->data + mark);
		mark = nx;
		return {
			v->pflags & 0x1fff,
			(v->pflags >> 13) & 0x7f,
			(v->pflags >> 20) & 0xfff,
			v->session,
			v->sequence
		};
	}
	template<typename T> Packet& operator<<(T const &v) {
		insert(v);
		return *this;
	}
	void complete() {
		if(!ptr) return;
		if(ch >= mark) return;
		uint32_t cl = mark - (ch + sizeof(PacketHeader));
		if(cl > 0x1fff) cl = 0x1fff;
		PacketHeader *ph = (PacketHeader*)(ptr->data + ch);
		ph->pflags = (ph->pflags & 0xffffe000u) | (cl);
		while(mark & 0x3) { ptr->data[mark++] = 0; }
		*((uint32_t*)(ptr->data + mark)) = 0xff8859ea;
		mark += sizeof(uint32_t);
		ch = mark;
		if(mark > pksz) pksz = mark;
	}
};

class MEISocket
{
	friend class MEISystem;
public:
	enum ConnectionState : uint32_t {
		MN_INIT = 0,
		MN_INIT_WAIT,
		MN_RECONNECT,
		MN_CONNECTED,
		MN_PONG_WAIT,
		MN_IDLE,
		MN_LOSS_WAIT,
		MN_SHUTDOWN,
		MN_SHUTDOWN_WAIT,
		MN_CLOSE_WAIT,
		MN_DELETE
	};
public:
	NETPORTEX MEISocket();
	NETPORTEX MEISocket(const address&);
	NETPORTEX ~MEISocket();
	NETPORTEX MEISocket(const MEISocket&) = delete;
	NETPORTEX MEISocket(MEISocket &&);
	NETPORTEX MEISocket& operator=(const MEISocket&) = delete;
	NETPORTEX MEISocket& operator=(MEISocket&&);

	NETPORTEX int send(Packet &);
	NETPORTEX int recv(Packet &);

private:
	std::shared_ptr<IMEITransfer> mtc;
};

}
