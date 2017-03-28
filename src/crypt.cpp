
#include "net/network.h"
#include "net/crypt.hpp"
#include "network_os.h"

#include <cstring>

namespace net {
namespace crypt {

class hash_state {
public:
	size_t sz;
	virtual void init() = 0;
	virtual void add_message(uint8_t const *m, size_t nbytes) = 0;
	virtual void end_message() = 0;
	virtual ~hash_state() {}
	virtual uint32_t hash_len() const {
		return 0;
	}
	virtual const uint32_t* hash_data() const {
		return 0;
	}
};

static const uint32_t shk_2[] = {
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};
static const uint64_t shk5[] = {
	0x428a2f98d728ae22, 0x7137449123ef65cd, 0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
	0x3956c25bf348b538, 0x59f111f1b605d019, 0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
	0xd807aa98a3030242, 0x12835b0145706fbe, 0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
	0x72be5d74f27b896f, 0x80deb1fe3b1696b1, 0x9bdc06a725c71235, 0xc19bf174cf692694,
	0xe49b69c19ef14ad2, 0xefbe4786384f25e3, 0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
	0x2de92c6f592b0275, 0x4a7484aa6ea6e483, 0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
	0x983e5152ee66dfab, 0xa831c66d2db43210, 0xb00327c898fb213f, 0xbf597fc7beef0ee4,
	0xc6e00bf33da88fc2, 0xd5a79147930aa725, 0x06ca6351e003826f, 0x142929670a0e6e70,
	0x27b70a8546d22ffc, 0x2e1b21385c26c926, 0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
	0x650a73548baf63de, 0x766a0abb3c77b2a8, 0x81c2c92e47edaee6, 0x92722c851482353b,
	0xa2bfe8a14cf10364, 0xa81a664bbc423001, 0xc24b8b70d0f89791, 0xc76c51a30654be30,
	0xd192e819d6ef5218, 0xd69906245565a910, 0xf40e35855771202a, 0x106aa07032bbd1b8,
	0x19a4c116b8d2d0c8, 0x1e376c085141ab53, 0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
	0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb, 0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
	0x748f82ee5defb2fc, 0x78a5636f43172f60, 0x84c87814a1f0ab72, 0x8cc702081a6439ec,
	0x90befffa23631e28, 0xa4506cebde82bde9, 0xbef9a3f7b2c67915, 0xc67178f2e372532b,
	0xca273eceea26619c, 0xd186b8c721c0c207, 0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
	0x06f067aa72176fba, 0x0a637dc5a2c898a6, 0x113f9804bef90dae, 0x1b710b35131c471b,
	0x28db77f523047d84, 0x32caab7b40c72493, 0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
	0x4cc5d4becb3e42b6, 0x597f299cfc657e2a, 0x5fcb6fab3ad6faec, 0x6c44198c4a475817
};

static const char radix64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void base64l_be(uint32_t const *u, size_t n, char *b, size_t l) {
	int i;
	if(!n || !l) return;
	uint32_t lala = 0;
	char *be = b + l;
	for(i = 0, n++; n--; ) {
		b[0] = radix64[(*u) >> 26];
		b[1] = radix64[077 & ((*u) >> 20)];
		b[2] = radix64[077 & ((*u) >> 14)];
		b[3] = radix64[077 & ((*u) >> 8)];
		b += 4; lala = *(u++);
		if(b >= be) break;
		if(!n--) {
			b[0] = radix64[077 & (lala >> 2)];
			b[1] = radix64[077 & (lala << 4)];
			b[2] = '=';
			b[3] = '=';
			break;
		}
		b[0] = radix64[077 & (lala >> 2)];
		b[1] = radix64[077 & ((lala << 4) | ((*u) >> 28))];
		b[2] = radix64[077 & ((*u) >> 22)];
		b[3] = radix64[077 & ((*u) >> 16)];
		b += 4; lala = *(u++);
		if(b >= be) break;
		if(!n--) {
			b[0] = radix64[077 & (lala >> 10)];
			b[1] = radix64[077 & (lala >> 4)];
			b[2] = radix64[077 & (lala << 2)];
			b[3] = '=';
			break;
		}
		b[0] = radix64[077 & (lala >> 10)];
		b[1] = radix64[077 & (lala >> 4)];
		b[2] = radix64[077 & ((lala << 2) | ((*u) >> 30))];
		b[3] = radix64[077 & ((*u) >> 24)];
		b += 4; lala = *(u++);
		if(b >= be) break;
		b[0] = radix64[077 & (lala >> 18)];
		b[1] = radix64[077 & (lala >> 12)];
		b[2] = radix64[077 & (lala >> 6)];
		b[3] = radix64[077 & (lala)];
		b += 4;
		if(b >= be) break;
		if(!n--) {
			break;
		}
	}
}

class sha1_state : public hash_state {
public:
	uint32_t hash[5];
private:
	uint64_t len;
	uint8_t mc[64];
public:
	uint32_t hash_len() const {
		return sizeof(hash);
	}
	const uint32_t* hash_data() const {
		return &hash[0];
	}
	void init() {
		hash[0] = 0x67452301;
		hash[1] = 0xefcdab89;
		hash[2] = 0x98badcfe;
		hash[3] = 0x10325476;
		hash[4] = 0xc3d2e1f0;
		len = 0;
	}

	void block_update() {
		uint32_t w[80];
		uint32_t a, b, c, d, e, f;
		int i;
		for(i = 0; i < 16; i++) {
			w[i] = (mc[(i * 4)] << 24) | (mc[(i * 4) + 1] << 16) | (mc[(i * 4) + 2] << 8) | mc[(i * 4) + 3];
		}
		for(; i < 80; i++) {
			f = w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16];
			w[i] = (f << 1) | (f >> 31);
		}
		a = hash[0]; b = hash[1]; c = hash[2]; d = hash[3]; e = hash[4];
		for(i = 0; i < 20; i++) {
			f = d ^ (b & (c ^ d));

			f += ((a << 5) | (a >> (32 - 5))) + e + w[i] + 0x5a827999;
			e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = f;
		}
		for(; i < 40; i++) {
			f = b ^ c ^ d;

			f += ((a << 5) | (a >> (32 - 5))) + e + w[i] + 0x6ed9eba1;
			e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = f;
		}
		for(; i < 60; i++) {
			f = (b & c) |(d &(b | c));

			f += ((a << 5) | (a >> (32 - 5))) + e + w[i] + 0x8f1bbcdc;
			e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = f;
		}
		for(; i < 80; i++) {
			f = b ^ c ^ d;

			f += ((a << 5) | (a >> (32 - 5))) + e + w[i] + 0xca62c1d6;
			e = d; d = c; c = (b << 30) | (b >> 2); b = a; a = f;
		}
		for(i = 0; i < 80; i++) w[i] = i; // clear the message buffer
		hash[0] += a;
		hash[1] += b;
		hash[2] += c;
		hash[3] += d;
		hash[4] += e;
	}
	void add_message(uint8_t const *m, size_t nbytes) {
		if(!m || !nbytes) return;
		uint64_t cl = this->len;
		while(nbytes--) {
			mc[(cl++) & 0x3f] = *(m++);
			if(!(cl & 0x3f)) block_update();
		}
		len = cl;
	}
	void end_message() {
		uint64_t cl = this->len;
		mc[(cl++) & 0x3f] = 0x80;
		while((cl & 0x3f) != 56) {
			if(!(cl & 0x3f)) block_update();
			mc[(cl++) & 0x3f] = 0x00;
		}
		len <<= 3;
		mc[56] = (uint8_t)(len >> 56);
		mc[57] = (uint8_t)(len >> 48);
		mc[58] = (uint8_t)(len >> 40);
		mc[59] = (uint8_t)(len >> 32);
		mc[60] = (uint8_t)(len >> 24);
		mc[61] = (uint8_t)(len >> 16);
		mc[62] = (uint8_t)(len >> 8);
		mc[63] = (uint8_t)(len);
		block_update();
	}
};

void hash_deleter::operator()(hash_state *p) {
	if(!p) return;
	p->~hash_state();
	memset(p, 0xECECECEC, p->sz);
	free(p);
}

template<typename T>
std::unique_ptr<hash_state, hash_deleter> make_hash() {
	hash_state *u = (hash_state *)malloc(sizeof(T));
	new(u) T();
	u->sz = sizeof(T);
	return std::unique_ptr<hash_state, hash_deleter>(u);
}

Hash::Hash(HASH_TYPE ht) {
	switch(ht) {
	case HA_SHA1: hs = std::move(make_hash<sha1_state>());
	}
}
Hash::~Hash() {

}

void Hash::Full(const uint8_t *m, size_t len) {
	if(!hs) return;
	hs->init();
	hs->add_message(m, len);
	hs->end_message();
}
void Hash::PartialBegin() {
	if(!hs) return;
	hs->init();
}
void Hash::PartialAdd(const uint8_t *m, size_t len) {
	if(!hs) return;
	hs->add_message(m, len);
}
void Hash::PartialFinish() {
	if(!hs) return;
	hs->end_message();
}

void Hash::tohex(std::string &s) {
	if(!hs) return;
}
std::string Hash::tohex() {
	std::string n;
	tohex(n);
	return std::move(n);
}
void Hash::tobase64(std::string &s) {
	if(!hs) return;
	size_t sz = hs->hash_len();
	size_t hz = sz / 4;
	sz = (sz * 4);
	int ex = sz % 3;
	sz = sz / 3;
	if(ex) sz += 4 - ex;
	s.resize(sz);
	base64l_be(hs->hash_data(), hz, &s[0], s.size());
}
std::string Hash::tobase64() {
	std::string n;
	tobase64(n);
	return std::move(n);
}

} // ns crypt
} // ns net
