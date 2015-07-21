/*	$OpenBSD: pvreg.h,v 1.1 2015/07/21 03:38:22 reyk Exp $	*/

/*
 * Copyright (c) 2015 Reyk Floeter <reyk@openbsd.org>
 * Copyright (c) 2015 Stefan Fritsch <sf@sfritsch.de>
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

#ifndef _DEV_PV_PVBUS_H_
#define _DEV_PV_PVBUS_H_

#define	CPUID_HV_SIGNATURE_START	0x40000000
#define	CPUID_HV_SIGNATURE_END		0x40010000
#define	CPUID_HV_SIGNATURE_STEP		0x100
#define CPUID_HV_SIGNATURE_STRLEN	12

/* KVM */
#define	CPUID_OFFSET_KVM_FEATURES		0x1

#define	KVM_FEATURE_CLOCKSOURCE			0	/* deprecated */
#define	KVM_FEATURE_NOP_IO_DELAY		1
#define	KVM_FEATURE_MMU_OP			2	/* deprecated */
#define	KVM_FEATURE_CLOCKSOURCE2		3
#define	KVM_FEATURE_ASYNC_PF			4
#define	KVM_FEATURE_STEAL_TIME			5
#define	KVM_FEATURE_PV_EOI			6
#define	KVM_FEATURE_PV_UNHALT			7
#define	KVM_FEATURE_CLOCKSOURCE_STABLE_BIT	24

#define	MSR_KVM_EOI_EN				0x4b564d04

#define KVM_PV_EOI_BIT				0

#endif /* _DEV_PV_PVBUS_H_ */
