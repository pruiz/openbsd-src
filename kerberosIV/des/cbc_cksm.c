/* lib/des/cbc_cksm.c */
/* Copyright (C) 1995 Eric Young (eay@mincom.oz.au)
 * All rights reserved.
 * 
 * This file is part of an SSL implementation written
 * by Eric Young (eay@mincom.oz.au).
 * The implementation was written so as to conform with Netscapes SSL
 * specification.  This library and applications are
 * FREE FOR COMMERCIAL AND NON-COMMERCIAL USE
 * as long as the following conditions are aheared to.
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.  If this code is used in a product,
 * Eric Young should be given attribution as the author of the parts used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by Eric Young (eay@mincom.oz.au)
 * 
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include "des_locl.h"

unsigned long des_cbc_cksum(input, output, length, schedule, ivec)
des_cblock (*input);
des_cblock (*output);
long length;
des_key_schedule schedule;
des_cblock (*ivec);
	{
	register unsigned long tout0,tout1,tin0,tin1;
	register long l=length;
	unsigned long tin[2];
	unsigned char *in,*out,*iv;

	in=(unsigned char *)input;
	out=(unsigned char *)output;
	iv=(unsigned char *)ivec;

	c2l(iv,tout0);
	c2l(iv,tout1);
	for (; l>0; l-=8)
		{
		if (l >= 8)
			{
			c2l(in,tin0);
			c2l(in,tin1);
			}
		else
			c2ln(in,tin0,tin1,l);
			
		tin0^=tout0; tin[0]=tin0;
		tin1^=tout1; tin[1]=tin1;
		des_encrypt((unsigned long *)tin,schedule,DES_ENCRYPT);
		/* fix 15/10/91 eay - thanks to keithr@sco.COM */
		tout0=tin[0];
		tout1=tin[1];
		}
	if (out != NULL)
		{
		l2c(tout0,out);
		l2c(tout1,out);
		}
	tout0=tin0=tin1=tin[0]=tin[1]=0;
	return(tout1);
	}
