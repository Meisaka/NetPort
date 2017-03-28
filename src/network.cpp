
#include "net/network.h"
#include "network_os.h"

#include <cstring>

namespace net {
static const char RadixLookup[] =
"0123456789"
"abcdefghijklmnopqrstuvwxyz"
"ABCDEFGHIJKLMNOPQRSTUVWXYZ/=";

char * n_itoan(int v, char* buf, int bufn, int radix)
{
	if(buf == NULL || bufn < 2 || radix < 1 || radix > 64) { return NULL; }
	int k,i,ss;
	char cc;
	if(!v) { buf[0] = '0'; buf[1] = 0; return buf; }
	ss = 0;
	if(v < 0) { buf[0] = '-'; ss++; v = -v; }
	for(i=ss; ((i+1) < bufn) && v; i++, v /= radix) {
		cc = RadixLookup[v % radix];
		for(k = i; k > ss; buf[k] = buf[(k--)-1]);
		buf[ss] = cc;
	}
	buf[i] = 0;
	return buf;
}

void close(socket_t &h)
{
	if(h == INVALID_SOCKET) { return; }
	shutdown(h, SHUT_RDWR);
#ifdef WIN32
	closesocket(h);
#else
	::close(h);
#endif
	h = INVALID_SOCKET;
}

int send(socket_t handle, const char * buf, int buflen)
{
	return ::send(handle, buf, buflen, 0);
}

int recv(socket_t handle, char * buf, int buflen)
{
	int i = ::recv(handle, buf, buflen, 0);
	if(i <= 0) {
#ifdef WIN32
		if(WSAGetLastError() == WSAEWOULDBLOCK) {
#else
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
#endif
			return 0;
		}
	}
	return i;
}

NetworkAddress::NetworkAddress()
{
	addr.sa_family = 0;
	af = NETA_UNDEF;
	type = 0;
	proto = 0;
}

NetworkAddress::NetworkAddress(const std::string &host)
{
	resolve(host);
}

void NetworkAddress::resolve(const std::string &host)
{
	struct addrinfo hint;
	struct addrinfo *res, *itr;
	int rc;
	hint.ai_socktype = 0;
	hint.ai_family = PF_UNSPEC;
	hint.ai_protocol = 0;
	hint.ai_flags = 0;
	rc = getaddrinfo(host.c_str(), nullptr, &hint, &res);
	if(!rc) {
		type = res->ai_socktype;
		proto = res->ai_protocol;
		af = static_cast<ADDRTYPE>((unsigned short)res->ai_family);
		std::memcpy(&addr, res->ai_addr, res->ai_addrlen > sizeof(net_sockaddr) ? sizeof(net_sockaddr) : res->ai_addrlen);
	}
	freeaddrinfo(res);
}

NetworkAddress::NetworkAddress(const std::string &host, const std::string &service)
{
	resolve(host, service);
}

void NetworkAddress::resolve(const std::string &host, const std::string &service)
{
	struct addrinfo hint;
	struct addrinfo *res, *itr;
	int rc;
	hint.ai_socktype = 0;
	hint.ai_family = PF_UNSPEC;
	hint.ai_protocol = 0;
	hint.ai_flags = 0;
	rc = getaddrinfo(host.c_str(), service.c_str(), &hint, &res);
	if(!rc) {
		type = res->ai_socktype;
		proto = res->ai_protocol;
		af = static_cast<ADDRTYPE>((unsigned short)res->ai_family);
		std::memcpy(&addr, res->ai_addr, res->ai_addrlen > sizeof(net_sockaddr) ? sizeof(net_sockaddr) : res->ai_addrlen);
	}
	freeaddrinfo(res);
}

NetworkAddress::NetworkAddress(const std::string &host, unsigned short port)
{
	resolve(host);
	NetworkAddress::port(port);
}

int NetworkAddress::length() const {
	switch(af) {
	case NETA_IPv4:
		return sizeof(struct sockaddr_in);
	case NETA_IPv6:
		return sizeof(struct sockaddr_in6);
	}
	return 0;
}

void NetworkAddress::port(unsigned short p)
{
	switch(af) {
	case NETA_IPv4:
		((struct sockaddr_in*)&addr)->sin_port = htons(p);
		return;
	case NETA_IPv6:
		((struct sockaddr_in6*)&addr)->sin6_port = htons(p);
		return;
	}
}

std::string NetworkAddress::to_string() const
{
	int i,k;
	char cvb[10];
	std::string ret;
	switch(af) {
	case NETA_IPv4:
	{
		ret.clear();
		cvb[3] = 0;
		in_addr ra4 = ((struct sockaddr_in*)&addr)->sin_addr;
		unsigned char* aa = (unsigned char*)&ra4;
		for(i=3, k = aa[0]; i && k; i--, k /= 10) { cvb[i-1] = '0' + (k % 10); }
		if(i==3) { ret.append("0."); } else {ret.append(cvb + i); ret.append("."); }
		for(i=3, k = aa[1]; i && k; i--, k /= 10) { cvb[i-1] = '0' + (k % 10); }
		if(i==3) { ret.append("0."); } else {ret.append(cvb + i); ret.append("."); }
		for(i=3, k = aa[2]; i && k; i--, k /= 10) { cvb[i-1] = '0' + (k % 10); }
		if(i==3) { ret.append("0."); } else {ret.append(cvb + i); ret.append("."); }
		for(i=3, k = aa[3]; i && k; i--, k /= 10) { cvb[i-1] = '0' + (k % 10); }
		if(i==3) { ret.append("0"); } else {ret.append(cvb + i); }
		ret.append(":");
		ret.append(n_itoan(ntohs(((struct sockaddr_in*)&addr)->sin_port),cvb,10,10));
		return ret;
	}
	case NETA_IPv6:
	{
		ret.clear();
		in6_addr ra6 = ((struct sockaddr_in6*)&addr)->sin6_addr;
		unsigned short* aa6 = (unsigned short*)&ra6;
		ret.assign("[");
		for(i = 0; i < 7; i++) {
			ret.append(n_itoan(ntohs(aa6[i]), cvb, 10, 16)); ret.append(":");
		}
		ret.append(n_itoan(ntohs(aa6[i]), cvb, 10, 16));
		ret.append("]:");
		ret.append(n_itoan(ntohs(((struct sockaddr_in6*)&addr)->sin6_port),cvb,10,10));
		return ret;
	}
	}
	return ret;
}

void NetworkAddress::ip4(const char * txta, unsigned short p)
{
	NetworkAddress::ip4(txta);
	NetworkAddress::port(p);
}
void NetworkAddress::ip4(const char * txta)
{
	addr.sa_family = AF_INET;
	af = NETA_IPv4;
	struct in_addr ra4;
	unsigned char ipev;
	int i;
	int sect;
	unsigned char* aa = (unsigned char*)&ra4;
	ipev = 0;
	for(i = 0, sect = 0; txta[i] > 0 && sect < 4; i++) {
		if(txta[i] >= '0' && txta[i] <= '9') {
			ipev = (ipev * 10) + (txta[i] - '0');
		} else {
			aa[sect] = ipev;
			sect++;
			ipev = 0;
		}
	}
	if(sect < 4) aa[sect] = ipev;
	((struct sockaddr_in*)&addr)->sin_addr = ra4;
}
void NetworkAddress::ip4(const std::string & txtd)
{
	NetworkAddress::ip4(txtd, 0);
}
void NetworkAddress::ip4(const std::string & txtd, unsigned short p)
{
	addr.sa_family = AF_INET;
	af = NETA_IPv4;
	struct in_addr ra4;
	unsigned char ipev;
	int i;
	int sect;
	unsigned char* aa = (unsigned char*)&ra4;
	ipev = 0;
	for(i = 0, sect = 0; i < txtd.length() && sect < 4; i++) {
		if(txtd[i] >= '0' && txtd[i] <= '9') {
			ipev = (ipev * 10) + (txtd[i] - '0');
		} else {
			aa[sect] = ipev;
			sect++;
			ipev = 0;
		}
	}
	if(sect < 4) aa[sect] = ipev;
	((struct sockaddr_in*)&addr)->sin_addr = ra4;
	((struct sockaddr_in*)&addr)->sin_port = htons(p);
}
void NetworkAddress::ip4(unsigned char i1,unsigned char i2,unsigned char i3,unsigned char i4)
{
	addr.sa_family = AF_INET;
	af = NETA_IPv4;
	struct in_addr ra4;
	unsigned char* aa = (unsigned char*)&ra4;
	aa[0] = i1;
	aa[1] = i2;
	aa[2] = i3;
	aa[3] = i4;
	((struct sockaddr_in*)&addr)->sin_addr = ra4;
}
void NetworkAddress::ip4(unsigned long i)
{
	addr.sa_family = AF_INET;
	af = NETA_IPv4;
#ifdef WIN32
	((struct sockaddr_in*)&addr)->sin_addr.S_un.S_addr = htonl(i);
#else
	((struct sockaddr_in*)&addr)->sin_addr.s_addr = htonl(i);
#endif
}
void NetworkAddress::ip6(const std::string &txtd, unsigned short p)
{
	addr.sa_family = AF_INET6;
	af = NETA_IPv6;
	in6_addr ra6 = ((struct sockaddr_in6*)&addr)->sin6_addr;
	unsigned short* aa6 = (unsigned short*)&ra6;
	int i, sect;
	int xpand, ltype;
	ltype = 0;
	xpand = 0;
	unsigned short ipev = 0;
	char cc;
	for(i = 0, sect = 0; i < txtd.length() && sect < 8; i++) {
		cc = txtd[i];
		if(cc >= '0' && cc <= '9') {
			ipev = (ipev << 4) + (cc - '0');
			ltype = 1;
		} else if(cc >= 'A' && cc <= 'F') {
			ipev = (ipev << 4) + (cc - 'A') + 10;
			ltype = 1;
		} else if(cc >= 'a' && cc <= 'f') {
			ipev = (ipev << 4) + (cc - 'a') + 10;
			ltype = 1;
		} else if(cc == ':') {
			aa6[sect] = htons(ipev);
			ipev = 0;
			sect++;
			if(2 == ltype) {
				if(!xpand) {
					xpand = sect - 1;
				} else {
					return; // invalid string
				}
			}
			ltype = 2;
		} else {
			ltype = 0;
		}
	}
	aa6[sect] = htons(ipev);
	if(xpand && xpand < 8) {
		int xofs = 7 - sect;
		for(i = 7; i - xofs >= xpand; i--) {
			aa6[i] = aa6[i - xofs];
		}
		for(; i >= xpand; i--) {
			aa6[i] = 0xfefe;
		}
	}
	((struct sockaddr_in6*)&addr)->sin6_addr = ra6;
	((struct sockaddr_in6*)&addr)->sin6_port = htons(p);
}
int sleep(int msec) {
#ifdef WIN32
	Sleep(msec);
	return 0;
#else
	return usleep(msec * 1000);
#endif
}

void get_time(TimeValue & rtv) {
#ifdef WIN32
	LARGE_INTEGER ticks, freq;
	unsigned long long leftovers;
	QueryPerformanceFrequency(&freq); 
	QueryPerformanceCounter(&ticks);
	rtv.seconds = ticks.QuadPart / freq.QuadPart;
	leftovers = ticks.QuadPart % freq.QuadPart;
	rtv.nanoseconds = (leftovers * 1000000000) / freq.QuadPart;
#else
#ifdef LINUX
	struct timespec tvs;
	clock_gettime(4, &tvs); /* since 2.6.28 */
	rtv.seconds = tvs.tv_sec;
	rtv.nanoseconds = tvs.tv_nsec;
#else
	rtv.seconds = 0;
	rtv.nanoseconds = 0;
#endif
#endif
}

void initialize() {
#ifdef WIN32
	WSADATA wsd;
	WSAStartup(MAKEWORD(2,2), &wsd);
#else
#endif
}

bool NetworkAddress::operator<(const net::NetworkAddress& _Right) const
{
	if((af < _Right.af)
		|| (proto < _Right.proto)
		|| (type < _Right.type)
		) return true;
	if(af != _Right.af) return false;
	if(af == net::NETA_IPv4) {
		return ((uint32_t*)addr.sa_data)[0] < ((uint32_t*)_Right.addr.sa_data)[0];
	}
	if(af == net::NETA_IPv6) {
		return (((uint32_t*)addr.sa_data)[0] < ((uint32_t*)_Right.addr.sa_data)[0])
			|| (((uint32_t*)addr.sa_data)[1] < ((uint32_t*)_Right.addr.sa_data)[1])
			|| (((uint32_t*)addr.sa_data)[2] < ((uint32_t*)_Right.addr.sa_data)[2])
			|| (((uint32_t*)addr.sa_data)[3] < ((uint32_t*)_Right.addr.sa_data)[3]);
	}
	return false;
}

struct INetworkQueryType {
	Socket *sck;
	short type;
	QueryCallback * eco;
};

#ifdef WIN32
#define NATIVEPOLL_RD POLLRDNORM
#define NATIVEPOLL_WR POLLWRNORM
#define NATIVEPOLL_PR POLLPRI
#define NATIVEPOLL_OB POLLRDBAND
#define NATIVEPOLL_IV POLLNVAL
#define NATIVEPOLL_HU POLLHUP
#define NATIVEPOLL_ER POLLERR
#else
#endif
static short QueryTypeToNative(QUERYTYPE type) {
	return ((type & QUERYTYPE::QUERY_READ) ? NATIVEPOLL_RD : 0)
		| ((type & QUERYTYPE::QUERY_WRITE) ? NATIVEPOLL_WR : 0)
		| ((type & QUERYTYPE::QUERY_OOB) ? NATIVEPOLL_OB : 0)
		| ((type & QUERYTYPE::QUERY_PRI) ? NATIVEPOLL_PR : 0)
		;
}
static QUERYTYPE NativeToQueryType(short type) {
	return (QUERYTYPE) (((type & NATIVEPOLL_RD) ? QUERYTYPE::QUERY_READ : 0)
		| ((type & NATIVEPOLL_WR) ? QUERYTYPE::QUERY_WRITE : 0)
		| ((type & NATIVEPOLL_OB) ? QUERYTYPE::QUERY_OOB : 0)
		| ((type & NATIVEPOLL_PR) ? QUERYTYPE::QUERY_PRI : 0)
		| ((type & NATIVEPOLL_IV) ? QUERYTYPE::QUERY_INVAL : 0)
		| ((type & NATIVEPOLL_HU) ? QUERYTYPE::QUERY_HUP : 0)
		| ((type & NATIVEPOLL_ER) ? QUERYTYPE::QUERY_ERROR : 0)
		);
}

class INetworkQuery {
public:
#ifdef WIN32
	std::vector<WSAPOLLFD> plist;
#else
#endif
	std::vector<INetworkQueryType> qlist;
	bool inval;
	void Rebuild() {
		if(inval) {
			inval = false;
			plist.resize(qlist.size());
			auto si = qlist.cbegin();
			auto se = qlist.cend();
			auto di = plist.begin();
			auto de = plist.end();
			while((si != se) && (di != de)) {
				di->events = si->type;
				di->fd = si->sck->get_handle();
				si++; di++;
			}
		}
	}
	int IPoll(int msec) {
		Rebuild();
		int r;
#ifdef WIN32
		r = WSAPoll(plist.data(), plist.size(), msec);
#else
		r = ::poll(plist.data(), plist.size(), msec);
#endif
		if(r) {
			if(inval) return -1;
			size_t i = 0;
			size_t sz = plist.size();
			auto ptr = plist.data();
			auto iptr = qlist.data();
			for(; i < sz; i++) {
				auto sptr = iptr + i;
				if(ptr[i].revents && sptr->eco) {
					sptr->eco->Event(*(sptr->sck), NativeToQueryType(ptr[i].revents));
				}
			}
		}
		return r;
	}
};

NetworkQuery::NetworkQuery() : mtc(new INetworkQuery()){
	mtc->inval = true;
}
NetworkQuery::~NetworkQuery() {

}
void NetworkQuery::Update() {
	mtc->inval = true;
	mtc->Rebuild();
}
void NetworkQuery::Add(Socket &s, QUERYTYPE type, QueryCallback * eco) {
	mtc->qlist.push_back({&s, QueryTypeToNative(type), eco});
	mtc->inval = true;
}
void NetworkQuery::Remove(Socket &s) {
	auto i = mtc->qlist.begin();
	auto e = mtc->qlist.end();
	bool le = false;
	while(i != e) {
		if(i->sck == &s) {
			le = true;
			i = mtc->qlist.erase(i);
			e = mtc->qlist.end();
			continue;
		}
		i++;
	}
	if(le) mtc->inval = true;
}
int NetworkQuery::QueryBlock() {
	return mtc->IPoll(-1);
}
int NetworkQuery::QueryWait(int msec) {
	return mtc->IPoll(msec);
}

} // namespace net
