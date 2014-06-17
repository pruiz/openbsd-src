/*	$OpenBSD: arc4random.c,v 1.34 2014/06/17 00:37:07 matthew Exp $	*/

/*
 * Copyright (c) 1996, David Mazieres <dm@uun.org>
 * Copyright (c) 2008, Damien Miller <djm@openbsd.org>
 * Copyright (c) 2013, Markus Friedl <markus@openbsd.org>
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
 * ChaCha based random number generator for OpenBSD.
 */

#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#include <sys/mman.h>

#include "thread_private.h"

#define KEYSTREAM_ONLY
#include "chacha_private.h"

#ifdef __GNUC__
#define inline __inline
#else				/* !__GNUC__ */
#define inline
#endif				/* !__GNUC__ */

#define KEYSZ	32
#define IVSZ	8
#define BLOCKSZ	64
#define RSBUFSZ	(16*BLOCKSZ)

static struct {
	size_t		rs_have;	/* valid bytes at end of rs_buf */
	size_t		rs_count;	/* bytes till reseed */
	chacha_ctx	rs_chacha;	/* chacha context for random keystream */
} *rs;
static u_char *rs_buf;		/* keystream blocks */

static inline void _rs_rekey(u_char *dat, size_t datlen);

static inline void
_rs_init(u_char *buf, size_t n)
{
	if (n < KEYSZ + IVSZ)
		return;

	if (rs == NULL) {
		if ((rs = mmap(NULL, sizeof(*rs), PROT_READ|PROT_WRITE,
		    MAP_ANON, -1, 0)) == MAP_FAILED)
			abort();
		if (minherit(rs, sizeof(*rs), MAP_INHERIT_ZERO) == -1)
			abort();
	}
	if (rs_buf == NULL) {
		if ((rs_buf = mmap(NULL, RSBUFSZ, PROT_READ|PROT_WRITE,
		    MAP_ANON, -1, 0)) == MAP_FAILED)
			abort();
		if (minherit(rs_buf, RSBUFSZ, MAP_INHERIT_ZERO) == -1)
			abort();
	}

	chacha_keysetup(&rs->rs_chacha, buf, KEYSZ * 8, 0);
	chacha_ivsetup(&rs->rs_chacha, buf + KEYSZ);
}

static void
_rs_stir(void)
{
	u_char rnd[KEYSZ + IVSZ];

	/* XXX */
	(void) getentropy(rnd, sizeof rnd);

	if (!rs)
		_rs_init(rnd, sizeof(rnd));
	else
		_rs_rekey(rnd, sizeof(rnd));
	explicit_bzero(rnd, sizeof(rnd));

	/* invalidate rs_buf */
	rs->rs_have = 0;
	memset(rs_buf, 0, RSBUFSZ);

	rs->rs_count = 1600000;
}

static inline void
_rs_stir_if_needed(size_t len)
{
	if (!rs || rs->rs_count <= len)
		_rs_stir();
	if (rs->rs_count <= len)
		rs->rs_count = 0;
	else
		rs->rs_count -= len;
}

static inline void
_rs_rekey(u_char *dat, size_t datlen)
{
#ifndef KEYSTREAM_ONLY
	memset(rs_buf, 0,RSBUFSZ);
#endif
	/* fill rs_buf with the keystream */
	chacha_encrypt_bytes(&rs->rs_chacha, rs_buf, rs_buf, RSBUFSZ);
	/* mix in optional user provided data */
	if (dat) {
		size_t i, m;

		m = MIN(datlen, KEYSZ + IVSZ);
		for (i = 0; i < m; i++)
			rs_buf[i] ^= dat[i];
	}
	/* immediately reinit for backtracking resistance */
	_rs_init(rs_buf, KEYSZ + IVSZ);
	memset(rs_buf, 0, KEYSZ + IVSZ);
	rs->rs_have = RSBUFSZ - KEYSZ - IVSZ;
}

static inline void
_rs_random_buf(void *_buf, size_t n)
{
	u_char *buf = (u_char *)_buf;
	size_t m;

	_rs_stir_if_needed(n);
	while (n > 0) {
		if (rs->rs_have > 0) {
			m = MIN(n, rs->rs_have);
			memcpy(buf, rs_buf + RSBUFSZ - rs->rs_have, m);
			memset(rs_buf + RSBUFSZ - rs->rs_have, 0, m);
			buf += m;
			n -= m;
			rs->rs_have -= m;
		}
		if (rs->rs_have == 0)
			_rs_rekey(NULL, 0);
	}
}

static inline void
_rs_random_u32(u_int32_t *val)
{
	_rs_stir_if_needed(sizeof(*val));
	if (rs->rs_have < sizeof(*val))
		_rs_rekey(NULL, 0);
	memcpy(val, rs_buf + RSBUFSZ - rs->rs_have, sizeof(*val));
	memset(rs_buf + RSBUFSZ - rs->rs_have, 0, sizeof(*val));
	rs->rs_have -= sizeof(*val);
}

u_int32_t
arc4random(void)
{
	u_int32_t val;

	_ARC4_LOCK();
	_rs_random_u32(&val);
	_ARC4_UNLOCK();
	return val;
}

void
arc4random_buf(void *buf, size_t n)
{
	_ARC4_LOCK();
	_rs_random_buf(buf, n);
	_ARC4_UNLOCK();
}

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
u_int32_t
arc4random_uniform(u_int32_t upper_bound)
{
	u_int32_t r, min;

	if (upper_bound < 2)
		return 0;

	/* 2**32 % x == (2**32 - x) % x */
	min = -upper_bound % upper_bound;

	/*
	 * This could theoretically loop forever but each retry has
	 * p > 0.5 (worst case, usually far better) of selecting a
	 * number inside the range we need, so it should rarely need
	 * to re-roll.
	 */
	for (;;) {
		r = arc4random();
		if (r >= min)
			break;
	}

	return r % upper_bound;
}
