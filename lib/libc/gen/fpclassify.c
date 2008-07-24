/*	$OpenBSD: fpclassify.c,v 1.1 2008/07/24 09:31:07 martynas Exp $	*/
/*
 * Copyright (c) 2008 Martynas Venckus <martynas@openbsd.org>
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

#include <sys/types.h>
#include <machine/ieee.h>
#include <math.h>

int
__fpclassify(double d)
{
	struct ieee_double *p = (struct ieee_double *)&d;

	if (p->dbl_exp == 0) {
		if (p->dbl_frach == 0 && p->dbl_fracl == 0)
			return FP_ZERO;
		else
			return FP_SUBNORMAL;
	}

	if (p->dbl_exp == DBL_EXP_INFNAN) {
		if (p->dbl_frach == 0 && p->dbl_fracl == 0)
			return FP_INFINITE;
		else
			return FP_NAN;
	}

	return FP_NORMAL;
}

int
__fpclassifyf(float f)
{
	struct ieee_single *p = (struct ieee_single *)&f;

	if (p->sng_exp == 0) {
		if (p->sng_frac == 0)
			return FP_ZERO;
		else
			return FP_SUBNORMAL;
	}

	if (p->sng_exp == SNG_EXP_INFNAN) {
		if (p->sng_frac == 0)
			return FP_INFINITE;
		else
			return FP_NAN;
	}

	return FP_NORMAL;
}

#if 0	/* XXX */
int
__fpclassifyl(long double e)
{
	struct ieee_ext *p = (struct ieee_ext *)&e;

	if (p->ext_exp == 0) {
		if (p->ext_frach == 0 && p->ext_fracl == 0)
			return FP_ZERO;
		else
			return FP_SUBNORMAL;
	}

	p->ext_frach &= ~0x80000000;	/* clear sign bit */

	if (p->ext_exp == EXT_EXP_INFNAN) {
		if (p->ext_frach == 0 && p->ext_fracl == 0)
			return FP_INFINITE;
		else
			return FP_NAN;
	}

	return FP_NORMAL;
}
#endif	/* XXX */
