
#include "net/MEISocket.hpp"
#include "net/MEISystem.hpp"
#include "IMEI.hpp"

namespace net {

MEISocket::MEISocket()
	: mtc(new IMEITransfer())
{
}

MEISocket::~MEISocket()
{
}

template<> void Packet::insert(PacketParsedHeader const &t) {
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

template<> PacketParsedHeader Packet::fetch() {
	uint32_t nx;
	if(!ptr) return{0, };
	nx = mark + sizeof(PacketHeader);
	if(nx > pksz) return{0, };
	PacketHeader *v = (PacketHeader*)(ptr->data + mark);
	mark = nx;
	return{
		v->pflags & 0x1fff,
		(uint16_t)((v->pflags >> 13) & 0x7f),
		(uint16_t)((v->pflags >> 20) & 0xfff),
		v->session,
		v->sequence
	};
}

void Packet::complete() {
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

// get the packet from the network layer
int MEISocket::recv(Packet &p) {
	if(!mtc->sys) return -1;
	return 0;
}

int IMEITransfer::send(IPacketRef &p)
{
	if(!sys) return -1;
	if(!p.ptr || !p.pksz) return -1;
	p.attempt++;
	p.wait = 500;
	if(sys->udpsck.send_to(remote, (char*)p.ptr, p.pksz) < 0) {
		p.wait = 100;
		return -1;
	}
	return 0;
}
// complete() the packet and send to the network layer
int MEISocket::send(Packet &p) {
	return mtc->send(p);
}
int IMEITransfer::send(Packet &p) {
	if(!sys) return -1;
	if(!p.ptr) return -1;
	p.complete();
	if(p.pksz < 1 || p.pksz > sizeof(PacketData)) return -1;
	rt.push_back(IPacketRef(p, seqlocal()));
	IPacketRef &pr = rt.back();
	send(pr);
	return 0;
}

void IMEITransfer::HandleInterval() {
	auto sitr = rt.begin();
	auto sitre = rt.end();
	while(sitr != sitre) {
		if(!--sitr->wait) {
			if(sitr->attempt > 5) {
				fprintf(stderr, "SCK: Retry Error\n");
				sitr = rt.erase(sitr);
				continue;
			}
			send(*sitr);
		}
		sitr++;
	}
}

void IMEITransfer::connect() {
	Packet cnpck;
	PacketParsedHeader mh = {0, MEIF_CONTROL, 0xF01, lsess, lseq};
	cnpck.reset();
	cnpck << mh;
	cnpck.insert<uint32_t>(lseqh);
	send(cnpck);
}

void IMEITransfer::HandleRecv(Packet &pem) {
	PacketParsedHeader mh = pem.fetch<PacketParsedHeader>();
	Packet per;
	if(MEIF_ISCONTROL(mh.flags)) {
		fprintf(stderr, "MEIPKT CTRL C=%3x L=%d\n", mh.code, mh.length);
		switch(mh.code) {
		case 0xf01:
			rseq = mh.sequence;
			rseqh = pem.fetch<uint32_t>();
			per.reset();
			mh.code = 0xf02;
			mh.flags = MEIF_CONTROL;
			mh.sequence = this->lseq;
			mh.session = this->rsess;
			per.insert(mh);
			per.insert<uint32_t>(this->lsess);
			per.insert<uint32_t>(this->lseqh);
			per.complete();
			{ IPacketRef ipr(per, cseqlocal()); send(ipr); }
			break;
		case 0xf02:
			rseq = mh.sequence;
			rsess = pem.fetch<uint32_t>();
			rseqh = pem.fetch<uint32_t>();
			auto sitr = rt.begin();
			auto sitre = rt.end();
			while(sitr != sitre) {
				if((0xf0100000 | (MEIF_CONTROL << 13)) == (*((uint32_t*)sitr->ptr->data) & 0xffffe000)) {
					fprintf(stderr, "MEIPKT SESSION CONNECT SUCCESS\n");
					sitr = rt.erase(sitr);
					continue;
				}
				sitr++;
			}
			break;
		}
	} else {
		fprintf(stderr, "MEIPKT L=%d [%08x]\n", mh.length, mh.session);
	}
}
void IMEITransfer::Enter(MEISocket::ConnectionState c) {

}
void IMEITransfer::EnterW(MEISocket::ConnectionState c, uint32_t ms) {

}

} // namespace
