/* $OpenBSD: ssl2.h,v 1.11 2014/08/11 04:45:19 miod Exp $ */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 * 
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 * 
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
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
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from 
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
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

#ifndef HEADER_SSL2_H 
#define HEADER_SSL2_H 

#ifdef  __cplusplus
extern "C" {
#endif

/* Protocol Version Codes */
#define SSL2_VERSION		0x0002
#define SSL2_VERSION_MAJOR	0x00
#define SSL2_VERSION_MINOR	0x02
/* #define SSL2_CLIENT_VERSION	0x0002 */
/* #define SSL2_SERVER_VERSION	0x0002 */

/* Protocol Message Codes */
#define SSL2_MT_ERROR			0
#define SSL2_MT_CLIENT_HELLO		1
#define SSL2_MT_CLIENT_MASTER_KEY	2
#define SSL2_MT_CLIENT_FINISHED		3
#define SSL2_MT_SERVER_HELLO		4
#define SSL2_MT_SERVER_VERIFY		5
#define SSL2_MT_SERVER_FINISHED		6
#define SSL2_MT_REQUEST_CERTIFICATE	7
#define SSL2_MT_CLIENT_CERTIFICATE	8

/* Error Message Codes */
#define SSL2_PE_UNDEFINED_ERROR		0x0000
#define SSL2_PE_NO_CIPHER		0x0001
#define SSL2_PE_NO_CERTIFICATE		0x0002
#define SSL2_PE_BAD_CERTIFICATE		0x0004
#define SSL2_PE_UNSUPPORTED_CERTIFICATE_TYPE 0x0006

/* Cipher Kind Values */
#define SSL2_CK_NULL_WITH_MD5			0x02000000 /* v3 */
#define SSL2_CK_RC4_128_WITH_MD5		0x02010080
#define SSL2_CK_RC4_128_EXPORT40_WITH_MD5	0x02020080
#define SSL2_CK_RC2_128_CBC_WITH_MD5		0x02030080
#define SSL2_CK_RC2_128_CBC_EXPORT40_WITH_MD5	0x02040080
#define SSL2_CK_IDEA_128_CBC_WITH_MD5		0x02050080
#define SSL2_CK_DES_64_CBC_WITH_MD5		0x02060040
#define SSL2_CK_DES_64_CBC_WITH_SHA		0x02060140 /* v3 */
#define SSL2_CK_DES_192_EDE3_CBC_WITH_MD5	0x020700c0
#define SSL2_CK_DES_192_EDE3_CBC_WITH_SHA	0x020701c0 /* v3 */
#define SSL2_CK_RC4_64_WITH_MD5			0x02080080 /* MS hack */

#define SSL2_CK_DES_64_CFB64_WITH_MD5_1		0x02ff0800 /* SSLeay */
#define SSL2_CK_NULL				0x02ff0810 /* SSLeay */

#define SSL2_TXT_DES_64_CFB64_WITH_MD5_1	"DES-CFB-M1"
#define SSL2_TXT_NULL_WITH_MD5			"NULL-MD5"
#define SSL2_TXT_RC4_128_WITH_MD5		"RC4-MD5"
#define SSL2_TXT_RC4_128_EXPORT40_WITH_MD5	"EXP-RC4-MD5"
#define SSL2_TXT_RC2_128_CBC_WITH_MD5		"RC2-CBC-MD5"
#define SSL2_TXT_RC2_128_CBC_EXPORT40_WITH_MD5	"EXP-RC2-CBC-MD5"
#define SSL2_TXT_IDEA_128_CBC_WITH_MD5		"IDEA-CBC-MD5"
#define SSL2_TXT_DES_64_CBC_WITH_MD5		"DES-CBC-MD5"
#define SSL2_TXT_DES_64_CBC_WITH_SHA		"DES-CBC-SHA"
#define SSL2_TXT_DES_192_EDE3_CBC_WITH_MD5	"DES-CBC3-MD5"
#define SSL2_TXT_DES_192_EDE3_CBC_WITH_SHA	"DES-CBC3-SHA"
#define SSL2_TXT_RC4_64_WITH_MD5		"RC4-64-MD5"

#define SSL2_TXT_NULL				"NULL"

/* Flags for the SSL_CIPHER.algorithm2 field */
#define SSL2_CF_5_BYTE_ENC			0x01
#define SSL2_CF_8_BYTE_ENC			0x02

/* Certificate Type Codes */
#define SSL2_CT_X509_CERTIFICATE		0x01

/* Authentication Type Code */
#define SSL2_AT_MD5_WITH_RSA_ENCRYPTION		0x01

#define SSL2_MAX_SSL_SESSION_ID_LENGTH		32

/* Upper/Lower Bounds */
#define SSL2_MAX_MASTER_KEY_LENGTH_IN_BITS	256
#define SSL2_MAX_RECORD_LENGTH_2_BYTE_HEADER	32767u	/* 2^15-1 */
#define SSL2_MAX_RECORD_LENGTH_3_BYTE_HEADER	16383	/* 2^14-1 */

#define SSL2_CHALLENGE_LENGTH	16
/*#define SSL2_CHALLENGE_LENGTH	32 */
#define SSL2_MIN_CHALLENGE_LENGTH	16
#define SSL2_MAX_CHALLENGE_LENGTH	32
#define SSL2_CONNECTION_ID_LENGTH	16
#define SSL2_MAX_CONNECTION_ID_LENGTH	16
#define SSL2_SSL_SESSION_ID_LENGTH	16
#define SSL2_MAX_CERT_CHALLENGE_LENGTH	32
#define SSL2_MIN_CERT_CHALLENGE_LENGTH	16
#define SSL2_MAX_KEY_MATERIAL_LENGTH	24

#ifdef  __cplusplus
}
#endif
#endif
