/********************************************************************\
 *
 *      FILE:     rmd160.c
 *
 *      CONTENTS: A sample C-implementation of the RIPEMD-160
 *                hash-function.
 *      TARGET:   any computer with an ANSI C compiler
 *
 *      AUTHOR:   Antoon Bosselaers, ESAT-COSIC
 *                (Arranged for libc by Todd C. Miller)
 *      DATE:     1 March 1996
 *      VERSION:  1.0
 *
 *      Copyright (c) Katholieke Universiteit Leuven
 *      1996, All Rights Reserved
 *
\********************************************************************/

/*  header files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <rmd160.h>

/********************************************************************/

/* macro definitions */

/* collect four bytes into one word: */
#define BYTES_TO_DWORD(strptr)			\
    (((u_int32_t) *((strptr)+3) << 24) |	\
    ((u_int32_t) *((strptr)+2) << 16) |		\
    ((u_int32_t) *((strptr)+1) <<  8) |		\
    ((u_int32_t) *(strptr)))

/* ROL(x, n) cyclically rotates x over n bits to the left */
/* x must be of an unsigned 32 bits type and 0 <= n < 32. */
#define ROL(x, n)	(((x) << (n)) | ((x) >> (32-(n))))

/* the three basic functions F(), G() and H() */
#define F(x, y, z)	((x) ^ (y) ^ (z))
#define G(x, y, z)	(((x) & (y)) | (~(x) & (z)))
#define H(x, y, z)	(((x) | ~(y)) ^ (z))
#define I(x, y, z)	(((x) & (z)) | ((y) & ~(z)))
#define J(x, y, z)	((x) ^ ((y) | ~(z)))

/* the eight basic operations FF() through III() */
#define FF(a, b, c, d, e, x, s)	{			\
      (a) += F((b), (c), (d)) + (x);			\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define GG(a, b, c, d, e, x, s)	{			\
      (a) += G((b), (c), (d)) + (x) + 0x5a827999U;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define HH(a, b, c, d, e, x, s)	{			\
      (a) += H((b), (c), (d)) + (x) + 0x6ed9eba1U;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define II(a, b, c, d, e, x, s)	{			\
      (a) += I((b), (c), (d)) + (x) + 0x8f1bbcdcU;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define JJ(a, b, c, d, e, x, s)	{			\
      (a) += J((b), (c), (d)) + (x) + 0xa953fd4eU;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define FFF(a, b, c, d, e, x, s)	{		\
      (a) += F((b), (c), (d)) + (x);			\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define GGG(a, b, c, d, e, x, s)	{		\
      (a) += G((b), (c), (d)) + (x) + 0x7a6d76e9U;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define HHH(a, b, c, d, e, x, s)	{		\
      (a) += H((b), (c), (d)) + (x) + 0x6d703ef3U;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define III(a, b, c, d, e, x, s)	{		\
      (a) += I((b), (c), (d)) + (x) + 0x5c4dd124U;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}
#define JJJ(a, b, c, d, e, x, s)	{		\
      (a) += J((b), (c), (d)) + (x) + 0x50a28be6U;	\
      (a) = ROL((a), (s)) + (e);			\
      (c) = ROL((c), 10);				\
}

/********************************************************************/

void RMD160Init(context)
	RMD160_CTX *context;
{

	/* ripemd-160 initialization constants */
	context->state[0] = 0x67452301U;
	context->state[1] = 0xefcdab89U;
	context->state[2] = 0x98badcfeU;
	context->state[3] = 0x10325476U;
	context->state[4] = 0xc3d2e1f0U;
	context->length[0] = context->length[1] = 0;
}

/********************************************************************/

void RMD160Transform(state, block)
	u_int32_t state[5];
	const u_int32_t block[16];
{
	u_int32_t aa = state[0],  bb = state[1],  cc = state[2],
	    dd = state[3],  ee = state[4];
	u_int32_t aaa = state[0], bbb = state[1], ccc = state[2],
	    ddd = state[3], eee = state[4];

	/* round 1 */
	FF(aa, bb, cc, dd, ee, block[ 0], 11);
	FF(ee, aa, bb, cc, dd, block[ 1], 14);
	FF(dd, ee, aa, bb, cc, block[ 2], 15);
	FF(cc, dd, ee, aa, bb, block[ 3], 12);
	FF(bb, cc, dd, ee, aa, block[ 4],  5);
	FF(aa, bb, cc, dd, ee, block[ 5],  8);
	FF(ee, aa, bb, cc, dd, block[ 6],  7);
	FF(dd, ee, aa, bb, cc, block[ 7],  9);
	FF(cc, dd, ee, aa, bb, block[ 8], 11);
	FF(bb, cc, dd, ee, aa, block[ 9], 13);
	FF(aa, bb, cc, dd, ee, block[10], 14);
	FF(ee, aa, bb, cc, dd, block[11], 15);
	FF(dd, ee, aa, bb, cc, block[12],  6);
	FF(cc, dd, ee, aa, bb, block[13],  7);
	FF(bb, cc, dd, ee, aa, block[14],  9);
	FF(aa, bb, cc, dd, ee, block[15],  8);

	/* round 2 */
	GG(ee, aa, bb, cc, dd, block[ 7],  7);
	GG(dd, ee, aa, bb, cc, block[ 4],  6);
	GG(cc, dd, ee, aa, bb, block[13],  8);
	GG(bb, cc, dd, ee, aa, block[ 1], 13);
	GG(aa, bb, cc, dd, ee, block[10], 11);
	GG(ee, aa, bb, cc, dd, block[ 6],  9);
	GG(dd, ee, aa, bb, cc, block[15],  7);
	GG(cc, dd, ee, aa, bb, block[ 3], 15);
	GG(bb, cc, dd, ee, aa, block[12],  7);
	GG(aa, bb, cc, dd, ee, block[ 0], 12);
	GG(ee, aa, bb, cc, dd, block[ 9], 15);
	GG(dd, ee, aa, bb, cc, block[ 5],  9);
	GG(cc, dd, ee, aa, bb, block[ 2], 11);
	GG(bb, cc, dd, ee, aa, block[14],  7);
	GG(aa, bb, cc, dd, ee, block[11], 13);
	GG(ee, aa, bb, cc, dd, block[ 8], 12);

	/* round 3 */
	HH(dd, ee, aa, bb, cc, block[ 3], 11);
	HH(cc, dd, ee, aa, bb, block[10], 13);
	HH(bb, cc, dd, ee, aa, block[14],  6);
	HH(aa, bb, cc, dd, ee, block[ 4],  7);
	HH(ee, aa, bb, cc, dd, block[ 9], 14);
	HH(dd, ee, aa, bb, cc, block[15],  9);
	HH(cc, dd, ee, aa, bb, block[ 8], 13);
	HH(bb, cc, dd, ee, aa, block[ 1], 15);
	HH(aa, bb, cc, dd, ee, block[ 2], 14);
	HH(ee, aa, bb, cc, dd, block[ 7],  8);
	HH(dd, ee, aa, bb, cc, block[ 0], 13);
	HH(cc, dd, ee, aa, bb, block[ 6],  6);
	HH(bb, cc, dd, ee, aa, block[13],  5);
	HH(aa, bb, cc, dd, ee, block[11], 12);
	HH(ee, aa, bb, cc, dd, block[ 5],  7);
	HH(dd, ee, aa, bb, cc, block[12],  5);

	/* round 4 */
	II(cc, dd, ee, aa, bb, block[ 1], 11);
	II(bb, cc, dd, ee, aa, block[ 9], 12);
	II(aa, bb, cc, dd, ee, block[11], 14);
	II(ee, aa, bb, cc, dd, block[10], 15);
	II(dd, ee, aa, bb, cc, block[ 0], 14);
	II(cc, dd, ee, aa, bb, block[ 8], 15);
	II(bb, cc, dd, ee, aa, block[12],  9);
	II(aa, bb, cc, dd, ee, block[ 4],  8);
	II(ee, aa, bb, cc, dd, block[13],  9);
	II(dd, ee, aa, bb, cc, block[ 3], 14);
	II(cc, dd, ee, aa, bb, block[ 7],  5);
	II(bb, cc, dd, ee, aa, block[15],  6);
	II(aa, bb, cc, dd, ee, block[14],  8);
	II(ee, aa, bb, cc, dd, block[ 5],  6);
	II(dd, ee, aa, bb, cc, block[ 6],  5);
	II(cc, dd, ee, aa, bb, block[ 2], 12);

	/* round 5 */
	JJ(bb, cc, dd, ee, aa, block[ 4],  9);
	JJ(aa, bb, cc, dd, ee, block[ 0], 15);
	JJ(ee, aa, bb, cc, dd, block[ 5],  5);
	JJ(dd, ee, aa, bb, cc, block[ 9], 11);
	JJ(cc, dd, ee, aa, bb, block[ 7],  6);
	JJ(bb, cc, dd, ee, aa, block[12],  8);
	JJ(aa, bb, cc, dd, ee, block[ 2], 13);
	JJ(ee, aa, bb, cc, dd, block[10], 12);
	JJ(dd, ee, aa, bb, cc, block[14],  5);
	JJ(cc, dd, ee, aa, bb, block[ 1], 12);
	JJ(bb, cc, dd, ee, aa, block[ 3], 13);
	JJ(aa, bb, cc, dd, ee, block[ 8], 14);
	JJ(ee, aa, bb, cc, dd, block[11], 11);
	JJ(dd, ee, aa, bb, cc, block[ 6],  8);
	JJ(cc, dd, ee, aa, bb, block[15],  5);
	JJ(bb, cc, dd, ee, aa, block[13],  6);

	/* parallel round 1 */
	JJJ(aaa, bbb, ccc, ddd, eee, block[ 5],  8);
	JJJ(eee, aaa, bbb, ccc, ddd, block[14],  9);
	JJJ(ddd, eee, aaa, bbb, ccc, block[ 7],  9);
	JJJ(ccc, ddd, eee, aaa, bbb, block[ 0], 11);
	JJJ(bbb, ccc, ddd, eee, aaa, block[ 9], 13);
	JJJ(aaa, bbb, ccc, ddd, eee, block[ 2], 15);
	JJJ(eee, aaa, bbb, ccc, ddd, block[11], 15);
	JJJ(ddd, eee, aaa, bbb, ccc, block[ 4],  5);
	JJJ(ccc, ddd, eee, aaa, bbb, block[13],  7);
	JJJ(bbb, ccc, ddd, eee, aaa, block[ 6],  7);
	JJJ(aaa, bbb, ccc, ddd, eee, block[15],  8);
	JJJ(eee, aaa, bbb, ccc, ddd, block[ 8], 11);
	JJJ(ddd, eee, aaa, bbb, ccc, block[ 1], 14);
	JJJ(ccc, ddd, eee, aaa, bbb, block[10], 14);
	JJJ(bbb, ccc, ddd, eee, aaa, block[ 3], 12);
	JJJ(aaa, bbb, ccc, ddd, eee, block[12],  6);

	/* parallel round 2 */
	III(eee, aaa, bbb, ccc, ddd, block[ 6],  9);
	III(ddd, eee, aaa, bbb, ccc, block[11], 13);
	III(ccc, ddd, eee, aaa, bbb, block[ 3], 15);
	III(bbb, ccc, ddd, eee, aaa, block[ 7],  7);
	III(aaa, bbb, ccc, ddd, eee, block[ 0], 12);
	III(eee, aaa, bbb, ccc, ddd, block[13],  8);
	III(ddd, eee, aaa, bbb, ccc, block[ 5],  9);
	III(ccc, ddd, eee, aaa, bbb, block[10], 11);
	III(bbb, ccc, ddd, eee, aaa, block[14],  7);
	III(aaa, bbb, ccc, ddd, eee, block[15],  7);
	III(eee, aaa, bbb, ccc, ddd, block[ 8], 12);
	III(ddd, eee, aaa, bbb, ccc, block[12],  7);
	III(ccc, ddd, eee, aaa, bbb, block[ 4],  6);
	III(bbb, ccc, ddd, eee, aaa, block[ 9], 15);
	III(aaa, bbb, ccc, ddd, eee, block[ 1], 13);
	III(eee, aaa, bbb, ccc, ddd, block[ 2], 11);

	/* parallel round 3 */
	HHH(ddd, eee, aaa, bbb, ccc, block[15],  9);
	HHH(ccc, ddd, eee, aaa, bbb, block[ 5],  7);
	HHH(bbb, ccc, ddd, eee, aaa, block[ 1], 15);
	HHH(aaa, bbb, ccc, ddd, eee, block[ 3], 11);
	HHH(eee, aaa, bbb, ccc, ddd, block[ 7],  8);
	HHH(ddd, eee, aaa, bbb, ccc, block[14],  6);
	HHH(ccc, ddd, eee, aaa, bbb, block[ 6],  6);
	HHH(bbb, ccc, ddd, eee, aaa, block[ 9], 14);
	HHH(aaa, bbb, ccc, ddd, eee, block[11], 12);
	HHH(eee, aaa, bbb, ccc, ddd, block[ 8], 13);
	HHH(ddd, eee, aaa, bbb, ccc, block[12],  5);
	HHH(ccc, ddd, eee, aaa, bbb, block[ 2], 14);
	HHH(bbb, ccc, ddd, eee, aaa, block[10], 13);
	HHH(aaa, bbb, ccc, ddd, eee, block[ 0], 13);
	HHH(eee, aaa, bbb, ccc, ddd, block[ 4],  7);
	HHH(ddd, eee, aaa, bbb, ccc, block[13],  5);

	/* parallel round 4 */
	GGG(ccc, ddd, eee, aaa, bbb, block[ 8], 15);
	GGG(bbb, ccc, ddd, eee, aaa, block[ 6],  5);
	GGG(aaa, bbb, ccc, ddd, eee, block[ 4],  8);
	GGG(eee, aaa, bbb, ccc, ddd, block[ 1], 11);
	GGG(ddd, eee, aaa, bbb, ccc, block[ 3], 14);
	GGG(ccc, ddd, eee, aaa, bbb, block[11], 14);
	GGG(bbb, ccc, ddd, eee, aaa, block[15],  6);
	GGG(aaa, bbb, ccc, ddd, eee, block[ 0], 14);
	GGG(eee, aaa, bbb, ccc, ddd, block[ 5],  6);
	GGG(ddd, eee, aaa, bbb, ccc, block[12],  9);
	GGG(ccc, ddd, eee, aaa, bbb, block[ 2], 12);
	GGG(bbb, ccc, ddd, eee, aaa, block[13],  9);
	GGG(aaa, bbb, ccc, ddd, eee, block[ 9], 12);
	GGG(eee, aaa, bbb, ccc, ddd, block[ 7],  5);
	GGG(ddd, eee, aaa, bbb, ccc, block[10], 15);
	GGG(ccc, ddd, eee, aaa, bbb, block[14],  8);

	/* parallel round 5 */
	FFF(bbb, ccc, ddd, eee, aaa, block[12] ,  8);
	FFF(aaa, bbb, ccc, ddd, eee, block[15] ,  5);
	FFF(eee, aaa, bbb, ccc, ddd, block[10] , 12);
	FFF(ddd, eee, aaa, bbb, ccc, block[ 4] ,  9);
	FFF(ccc, ddd, eee, aaa, bbb, block[ 1] , 12);
	FFF(bbb, ccc, ddd, eee, aaa, block[ 5] ,  5);
	FFF(aaa, bbb, ccc, ddd, eee, block[ 8] , 14);
	FFF(eee, aaa, bbb, ccc, ddd, block[ 7] ,  6);
	FFF(ddd, eee, aaa, bbb, ccc, block[ 6] ,  8);
	FFF(ccc, ddd, eee, aaa, bbb, block[ 2] , 13);
	FFF(bbb, ccc, ddd, eee, aaa, block[13] ,  6);
	FFF(aaa, bbb, ccc, ddd, eee, block[14] ,  5);
	FFF(eee, aaa, bbb, ccc, ddd, block[ 0] , 15);
	FFF(ddd, eee, aaa, bbb, ccc, block[ 3] , 13);
	FFF(ccc, ddd, eee, aaa, bbb, block[ 9] , 11);
	FFF(bbb, ccc, ddd, eee, aaa, block[11] , 11);

	/* combine results */
	ddd += cc + state[1];		/* final result for state[0] */
	state[1] = state[2] + dd + eee;
	state[2] = state[3] + ee + aaa;
	state[3] = state[4] + aa + bbb;
	state[4] = state[0] + bb + ccc;
	state[0] = ddd;
}

/********************************************************************/

void RMD160Update(context, data, nbytes)
	RMD160_CTX *context;
	const u_char *data;
	size_t nbytes;
{
	u_int32_t	X[16];
	size_t	i, j;

	(void)memset(X, 0, 16 * sizeof(u_int32_t));

	/* process all complete blocks */
	for (i = 0; i < (nbytes >> 6); i++) {
		for (j=0; j < 16; j++)
			X[j] = BYTES_TO_DWORD(data + (64 * i) + (4 * j));
		RMD160Transform(context->state, X);
	}

	/* update length[] */
	if (context->length[0] + nbytes < context->length[0])
		context->length[1]++;		/* overflow to msb of length */
	context->length[0] += nbytes;

	/*
	 * Put bytes from data into context's buffer
	 */
	(void)memset(context->buffer, 0, 16 * sizeof(u_int32_t));
	/* extract bytes 6 to 10 inclusive */
	data += (context->length[0] & 0x3C0);
	for (i = 0; i < (context->length[0] & 63); i++) {
		/* byte i goes into word buffer[i div 4] at pos.  8*(i mod 4) */
		context->buffer[i>>2] ^= (u_int32_t) *data++ << (8 * (i & 3));
	}
}

/********************************************************************/

void RMD160Final(digest, context)
	u_char digest[20];
	RMD160_CTX *context;
{
	u_int32_t i;

	/* append the bit m_n == 1 */
	context->buffer[(context->length[0] >> 2) & 15] ^=
	    1U << (8 * (context->length[0] & 3) + 7);

	if ((context->length[0] & 63) > 55) {
		/* length goes to next block */
		RMD160Transform(context->state, context->buffer);
		(void)memset(context->buffer, 0, 16 * sizeof(u_int32_t));
	}

	/* append length in bits */
	context->buffer[14] = context->length[0] << 3;
	context->buffer[15] = (context->length[0] >> 29) |
	    (context->length[1] << 3);
	RMD160Transform(context->state, context->buffer);

	if (digest != NULL) {
		for (i = 0; i < 20; i += 4) {
			/* extracts the 8 least significant bits. */
			digest[i]     =  context->state[i>>2];
			digest[i + 1] = (context->state[i>>2] >>  8);
			digest[i + 2] = (context->state[i>>2] >> 16);
			digest[i + 3] = (context->state[i>>2] >> 24);
		}
	}
}

/************************ end of file rmd160.c **********************/
