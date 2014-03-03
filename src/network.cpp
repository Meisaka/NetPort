
#include "net/network.h"
#include "network_os.h"

namespace network {
	static const char* RadixLookup =
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

	int NetworkAddress::Length() const {
		switch(af) {
		case NETA_IPv4:
			return sizeof(struct sockaddr_in);
		case NETA_IPv6:
			return sizeof(struct sockaddr_in6);
		}
		return 0;
	}

	void NetworkAddress::Port(unsigned short p)
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

	std::string NetworkAddress::ToString() const
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
			ret.append(n_itoan(aa6[i],cvb,10,16)); ret.append(":");
			}
			ret.append(n_itoan(aa6[i],cvb,10,16));
			ret.append("]:");
			ret.append(n_itoan(ntohs(((struct sockaddr_in6*)&addr)->sin6_port),cvb,10,10));
			return ret;
			}
		}
		return ret;
	}

	void NetworkAddress::IP4(const char * txta)
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
	void NetworkAddress::IP4(unsigned char i1,unsigned char i2,unsigned char i3,unsigned char i4)
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
	void NetworkAddress::IP4(unsigned long i)
	{
		addr.sa_family = AF_INET;
		af = NETA_IPv4;
	#ifdef WIN32
		((struct sockaddr_in*)&addr)->sin_addr.S_un.S_addr = htonl(i);
	#else
		((struct sockaddr_in*)&addr)->sin_addr.s_addr = htonl(i);
	#endif
	}

	int sleep(int msec) {
	#ifdef WIN32
		Sleep(msec);
		return 0;
	#else
		return usleep(msec * 1000);
	#endif
	}

	void Initialize() {
	#ifdef WIN32
		WSADATA wsd;
		WSAStartup(MAKEWORD(2,2), &wsd);
	#else
	#endif
	}
}
