/*	$OpenBSD: timingsafe.c,v 1.1 2014/06/13 01:55:02 matthew Exp $	*/
/*
 * Copyright (c) 2014 Google Inc.
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT_EQ(a, b) assert((a) == (b))

enum {
	N = 8
};

static unsigned char bufone[N], buftwo[N];

void
check()
{
	int cmp = memcmp(bufone, buftwo, N);

	/*
	 * timingsafe_memcmp is specified to return -1, 0, or 1,
	 * but memcmp only specifies <0, 0, or >0.
	 */
	if (cmp < 0) cmp = -1;
	if (cmp > 0) cmp = 1;

	/* Check for reflexivity. */
	ASSERT_EQ(0, timingsafe_bcmp(bufone, bufone, N));
	ASSERT_EQ(0, timingsafe_bcmp(buftwo, buftwo, N));
#if notyet
	ASSERT_EQ(0, timingsafe_memcmp(bufone, bufone, N));
	ASSERT_EQ(0, timingsafe_memcmp(buftwo, buftwo, N));
#endif

	/* Check that timingsafe_bcmp returns 0 iff memcmp returns 0. */
	ASSERT_EQ(cmp == 0, timingsafe_bcmp(bufone, buftwo, N) == 0);

#if notyet
	/* Check that timingsafe_memcmp returns cmp... */
	ASSERT_EQ(cmp, timingsafe_memcmp(bufone, buftwo, N));

	/* ... or -cmp if the argument order is swapped. */
	ASSERT_EQ(-cmp, timingsafe_memcmp(buftwo, bufone, N));
#endif
}

int
main()
{
	int i, j;

	for (i = 0; i < 10000; i++) {
		arc4random_buf(bufone, N);
		arc4random_buf(buftwo, N);

		check();
		for (j = 0; j < N; j++) {
			buftwo[j] = bufone[j];
			check();
		}
	}

	return (0);
}
