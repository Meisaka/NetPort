/* Portable arc4random.c based on arc4random.c from OpenBSD.
* Portable version by Chris Davis, adapted for Libevent by Nick Mathewson
* Copyright (c) 2010 Chris Davis, Niels Provos, and Nick Mathewson
* Copyright (c) 2010-2012 Niels Provos and Nick Mathewson
*
* Note that in Libevent, this file isn't compiled directly.  Instead,
* it's included from evutil_rand.c
*/

/*
* Copyright (c) 1996, David Mazieres <dm@uun.org>
* Copyright (c) 2008, Damien Miller <djm@openbsd.org>
*
* Permission to use, copy, modify, and distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*
* THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
* WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
* ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
* WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
* ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
* OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/*
* Arc4 random number generator for OpenBSD.
*
* This code is derived from section 17.1 of Applied Cryptography,
* second edition, which describes a stream cipher allegedly
* compatible with RSA Labs "RC4" cipher (the actual description of
* which is a trade secret).  The same algorithm is used as a stream
* cipher called "arcfour" in Tatu Ylonen's ssh package.
*
* Here the stream cipher has been modified always to include the time
* when initializing the state.  That makes it impossible to
* regenerate the same random sequence twice, so this can't be used
* for encryption, but will generate good random numbers.
*
* RC4 is a registered trademark of RSA Laboratories.
*/

#include <stdint.h>

#ifdef WIN32
#include <Windows.h>
#include <wincrypt.h>
#include <process.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/time.h>
#include <stdlib.h>
#endif

#include <mutex>

namespace net {

std::mutex _ARC4_MUTEX;

#define _ARC4_LOCK()  std::unique_lock<std::mutex> _ARC4_SCOPE_LOCK(_ARC4_MUTEX)
#define _ARC4_UNLOCK() _ARC4_SCOPE_LOCK.unlock()

/* Add platform entropy 32 bytes (256 bits) at a time. */
#define ADD_ENTROPY 32

/* Re-seed from the platform RNG after generating this many bytes. */
#define BYTES_BEFORE_RESEED 1600000

static int rs_initialized = 0;
static struct arc4_stream {
	unsigned char i;
	unsigned char j;
	unsigned char s[256];
} rs;
static int arc4_count = 0;
static int arc4_seeded_ok = 0;

static void arc4_memclear(void *b, size_t sz) {
#ifdef WIN32
	SecureZeroMemory(b, sz);
#else
	memset(b, 0, sz);
#endif
}

static inline void
arc4_init(void)
{
	size_t n;
	for(n = 0; n < 256; n++)
		rs.s[n] = n;
	rs.i = 0;
	rs.j = 0;
}

static inline void
arc4_addrandom(const uint8_t *dat, size_t datlen)
{
	size_t n;
	uint8_t si;
	rs.i--;
	for(n = 0; n < 256; n++) {
		rs.i = (rs.i + 1);
		si = rs.s[rs.i];
		rs.j = (rs.j + si + dat[n % datlen]);
		rs.s[rs.i] = rs.s[rs.j];
		rs.s[rs.j] = si;
	}
	rs.j = rs.i;
}

#ifdef WIN32
#define TRY_SEED_WIN32
static int
arc4_seed_win32(void)
{
	/* This is adapted from Tor's crypto_seed_rng() */
	static int provider_set = 0;
	static HCRYPTPROV provider;
	unsigned char buf[ADD_ENTROPY];
	if(!provider_set) {
		if(!CryptAcquireContext(&provider, NULL, NULL, PROV_RSA_FULL,
			CRYPT_VERIFYCONTEXT)) {
			if(GetLastError() != (DWORD)NTE_BAD_KEYSET)
				return -1;
		}
		provider_set = 1;
	}
	if(!CryptGenRandom(provider, sizeof(buf), buf))
		return -1;
	arc4_addrandom(buf, sizeof(buf));
	arc4_memclear(buf, sizeof(buf));
	arc4_seeded_ok = 1;
	return 0;
}
#endif

#ifndef WIN32
#define TRY_SEED_URANDOM
static char *arc4random_urandom_filename = NULL;
static int arc4_seed_urandom_helper_(const char *fname)
{
	unsigned char buf[ADD_ENTROPY];
	int fd;
	size_t n;
	fd = evutil_open_closeonexec(fname, O_RDONLY, 0);
	if(fd<0)
		return -1;
	n = read_all(fd, buf, sizeof(buf));
	close(fd);
	if(n != sizeof(buf))
		return -1;
	arc4_addrandom(buf, sizeof(buf));
	evutil_memclear_(buf, sizeof(buf));
	arc4_seeded_ok = 1;
	return 0;
}
static int
arc4_seed_urandom(void)
{
	/* This is adapted from Tor's crypto_seed_rng() */
	static const char *filenames[] = {
		"/dev/srandom", "/dev/urandom", "/dev/random", NULL
	};
	int i;
	if(arc4random_urandom_filename)
		return arc4_seed_urandom_helper_(arc4random_urandom_filename);
	for(i = 0; filenames[i]; ++i) {
		if(arc4_seed_urandom_helper_(filenames[i]) == 0) {
			return 0;
		}
	}
	return -1;
}
#endif

static int
arc4_seed(void)
{
	int ok = 0;
	/* We try every method that might work, and don't give up even if one
	* does seem to work.  There's no real harm in over-seeding, and if
	* one of these sources turns out to be broken, that would be bad. */
#ifdef TRY_SEED_WIN32
	if(0 == arc4_seed_win32())
		ok = 1;
#endif
#ifdef TRY_SEED_URANDOM
	if(0 == arc4_seed_urandom())
		ok = 1;
#endif
#ifdef TRY_SEED_PROC_SYS_KERNEL_RANDOM_UUID
	if(arc4random_urandom_filename == NULL &&
		0 == arc4_seed_proc_sys_kernel_random_uuid())
		ok = 1;
#endif
#ifdef TRY_SEED_SYSCTL_BSD
	if(0 == arc4_seed_sysctl_bsd())
		ok = 1;
#endif
	return ok ? 0 : -1;
}

static inline uint8_t
arc4_getbyte(void)
{
	uint8_t si, sj;
	rs.i = (rs.i + 1);
	si = rs.s[rs.i];
	rs.j = (rs.j + si);
	sj = rs.s[rs.j];
	rs.s[rs.i] = sj;
	rs.s[rs.j] = si;
	return (rs.s[(si + sj) & 0xff]);
}

static inline uint32_t
arc4_getword(void)
{
	uint32_t val;
	val = arc4_getbyte() << 24;
	val |= arc4_getbyte() << 16;
	val |= arc4_getbyte() << 8;
	val |= arc4_getbyte();
	return val;
}

static int
arc4_stir(void)
{
	int i;
	if(!rs_initialized) {
		arc4_init();
		rs_initialized = 1;
	}
	arc4_seed();
	if(!arc4_seeded_ok)
		return -1;
	/*
	* Discard early keystream, as per recommendations in
	* "Weaknesses in the Key Scheduling Algorithm of RC4" by
	* Scott Fluhrer, Itsik Mantin, and Adi Shamir.
	* http://www.wisdom.weizmann.ac.il/~itsik/RC4/Papers/Rc4_ksa.ps
	*
	* Ilya Mironov's "(Not So) Random Shuffles of RC4" suggests that
	* we drop at least 2*256 bytes, with 12*256 as a conservative
	* value.
	*
	* RFC4345 says to drop 6*256.
	*
	* At least some versions of this code drop 4*256, in a mistaken
	* belief that "words" in the Fluhrer/Mantin/Shamir paper refers
	* to processor words.
	*
	* We add another sect to the cargo cult, and choose 12*256.
	*/
	for(i = 0; i < 12 * 256; i++)
		(void)arc4_getbyte();
	arc4_count = BYTES_BEFORE_RESEED;
	return 0;
}

static void
arc4_stir_if_needed(void)
{
	if(arc4_count <= 0 || !rs_initialized) {
		arc4_stir();
	}
}

uint32_t arc4random(void)
{
	uint32_t val;
	_ARC4_LOCK();
	arc4_count -= 4;
	arc4_stir_if_needed();
	val = arc4_getword();
	_ARC4_UNLOCK();
	return val;
}

#ifndef ARC4RANDOM_NOUNIFORM
/*
* Calculate a uniformly distributed random number less than upper_bound
* avoiding "modulo bias".
*
* Uniformity is achieved by generating new random numbers until the one
* returned is outside the range [0, 2**32 % upper_bound).  This
* guarantees the selected random number will be inside
* [2**32 % upper_bound, 2**32) which maps back to [0, upper_bound)
* after reduction modulo upper_bound.
*/
uint32_t
arc4random_uniform(uint32_t upper_bound)
{
	uint32_t r, min;
	if(upper_bound < 2)
		return 0;
#if (UINT_MAX > 0xffffffffUL)
	min = 0x100000000UL % upper_bound;
#else
	/* Calculate (2**32 % upper_bound) avoiding 64-bit math */
	if(upper_bound > 0x80000000)
		min = 1 + ~upper_bound;		/* 2**32 - upper_bound */
	else {
		/* (2**32 - (x * 2)) % x == 2**32 % x when x <= 2**31 */
		min = ((0xffffffff - (upper_bound * 2)) + 1) % upper_bound;
	}
#endif
	/*
	* This could theoretically loop forever but each retry has
	* p > 0.5 (worst case, usually far better) of selecting a
	* number inside the range we need, so it should rarely need
	* to re-roll.
	*/
	for(;;) {
		r = arc4random();
		if(r >= min)
			break;
	}
	return r % upper_bound;
}
#endif

} // namespace
