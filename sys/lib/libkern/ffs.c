/*	$OpenBSD: ffs.c,v 1.4 2000/07/02 01:03:29 mickey Exp $	*/

/*
 * Public domain.
 * Written by Dale Rahn.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: ffs.c,v 1.4 2000/07/02 01:03:29 mickey Exp $";
#endif /* LIBC_SCCS and not lint */

#if !defined(_KERNEL) && !defined(_STANDALONE)
#include <string.h>
#else
#include <lib/libkern/libkern.h>
#endif

/*
 * ffs -- vax ffs instruction
 */
int
ffs(mask)
	register int mask;
{
	register int bit;
	register unsigned int r = mask;
	static const signed char t[16] = {
		-28, 1, 2, 1,
		  3, 1, 2, 1,
		  4, 1, 2, 1,
		  3, 1, 2, 1 };

	bit = 0;
	if (0 == (r & 0xffff)) {
		bit += 16;
		r >>= 16;
	}
	if (0 == (r & 0xff)) {
		bit += 8;
		r >>= 8;
	}
	if (0 == (r & 0xf)) {
		bit += 4;
		r >>= 4;
	}

	return (bit + t[ r & 0xf ]);
}
