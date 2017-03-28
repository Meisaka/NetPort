#pragma once
#ifndef INCL_NETCRYPTCOM
#define INCL_NETCRYPTCOM

#include "network_common.hpp"

namespace net {
namespace crypt {

class hash_state;
struct hash_deleter {
	void operator()(hash_state *p);
};

enum HASH_TYPE {
	HA_MD2,
	HA_MD3,
	HA_MD5,
	HA_SHA1,
	HA_SHA224,
	HA_SHA256,
	HA_SHA384,
	HA_SHA512,
	HA_SHA512_224,
	HA_SHA512_256,
	HA_SHA3_224,
	HA_SHA3_256,
	HA_SHA3_384,
	HA_SHA3_512,
};

class Hash {
public:
	std::unique_ptr<hash_state, hash_deleter> hs;

	Hash(HASH_TYPE);
	~Hash();

	void Full(const uint8_t *m, size_t len);
	void PartialBegin();
	void PartialAdd(const uint8_t *m, size_t len);
	void PartialFinish();

	void tohex(std::string &);
	std::string tohex();
	void tobase64(std::string &);
	std::string tobase64();
};

} // ns crypt
} // ns net
#endif
