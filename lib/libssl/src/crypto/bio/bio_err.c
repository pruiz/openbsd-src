/* crypto/bio/bio_err.c */
/* ====================================================================
 * Copyright (c) 1999-2011 The OpenSSL Project.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. All advertising materials mentioning features or use of this
 *    software must display the following acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit. (http://www.OpenSSL.org/)"
 *
 * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. For written permission, please contact
 *    openssl-core@OpenSSL.org.
 *
 * 5. Products derived from this software may not be called "OpenSSL"
 *    nor may "OpenSSL" appear in their names without prior written
 *    permission of the OpenSSL Project.
 *
 * 6. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by the OpenSSL Project
 *    for use in the OpenSSL Toolkit (http://www.OpenSSL.org/)"
 *
 * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 * ====================================================================
 *
 * This product includes cryptographic software written by Eric Young
 * (eay@cryptsoft.com).  This product includes software written by Tim
 * Hudson (tjh@cryptsoft.com).
 *
 */

/* NOTE: this file was auto generated by the mkerr.pl script: any changes
 * made to it will be overwritten when the script next updates this file,
 * only reason strings will be preserved.
 */

#include <stdio.h>
#include <openssl/err.h>
#include <openssl/bio.h>

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

#define ERR_FUNC(func) ERR_PACK(ERR_LIB_BIO,func,0)
#define ERR_REASON(reason) ERR_PACK(ERR_LIB_BIO,0,reason)

static ERR_STRING_DATA BIO_str_functs[] = {
	{ERR_FUNC(BIO_F_ACPT_STATE),	"ACPT_STATE"},
	{ERR_FUNC(BIO_F_BIO_ACCEPT),	"BIO_accept"},
	{ERR_FUNC(BIO_F_BIO_BER_GET_HEADER),	"BIO_BER_GET_HEADER"},
	{ERR_FUNC(BIO_F_BIO_CALLBACK_CTRL),	"BIO_callback_ctrl"},
	{ERR_FUNC(BIO_F_BIO_CTRL),	"BIO_ctrl"},
	{ERR_FUNC(BIO_F_BIO_GETHOSTBYNAME),	"BIO_gethostbyname"},
	{ERR_FUNC(BIO_F_BIO_GETS),	"BIO_gets"},
	{ERR_FUNC(BIO_F_BIO_GET_ACCEPT_SOCKET),	"BIO_get_accept_socket"},
	{ERR_FUNC(BIO_F_BIO_GET_HOST_IP),	"BIO_get_host_ip"},
	{ERR_FUNC(BIO_F_BIO_GET_PORT),	"BIO_get_port"},
	{ERR_FUNC(BIO_F_BIO_MAKE_PAIR),	"BIO_MAKE_PAIR"},
	{ERR_FUNC(BIO_F_BIO_NEW),	"BIO_new"},
	{ERR_FUNC(BIO_F_BIO_NEW_FILE),	"BIO_new_file"},
	{ERR_FUNC(BIO_F_BIO_NEW_MEM_BUF),	"BIO_new_mem_buf"},
	{ERR_FUNC(BIO_F_BIO_NREAD),	"BIO_nread"},
	{ERR_FUNC(BIO_F_BIO_NREAD0),	"BIO_nread0"},
	{ERR_FUNC(BIO_F_BIO_NWRITE),	"BIO_nwrite"},
	{ERR_FUNC(BIO_F_BIO_NWRITE0),	"BIO_nwrite0"},
	{ERR_FUNC(BIO_F_BIO_PUTS),	"BIO_puts"},
	{ERR_FUNC(BIO_F_BIO_READ),	"BIO_read"},
	{ERR_FUNC(BIO_F_BIO_SOCK_INIT),	"BIO_sock_init"},
	{ERR_FUNC(BIO_F_BIO_WRITE),	"BIO_write"},
	{ERR_FUNC(BIO_F_BUFFER_CTRL),	"BUFFER_CTRL"},
	{ERR_FUNC(BIO_F_CONN_CTRL),	"CONN_CTRL"},
	{ERR_FUNC(BIO_F_CONN_STATE),	"CONN_STATE"},
	{ERR_FUNC(BIO_F_DGRAM_SCTP_READ),	"DGRAM_SCTP_READ"},
	{ERR_FUNC(BIO_F_FILE_CTRL),	"FILE_CTRL"},
	{ERR_FUNC(BIO_F_FILE_READ),	"FILE_READ"},
	{ERR_FUNC(BIO_F_LINEBUFFER_CTRL),	"LINEBUFFER_CTRL"},
	{ERR_FUNC(BIO_F_MEM_READ),	"MEM_READ"},
	{ERR_FUNC(BIO_F_MEM_WRITE),	"MEM_WRITE"},
	{ERR_FUNC(BIO_F_SSL_NEW),	"SSL_new"},
	{ERR_FUNC(BIO_F_WSASTARTUP),	"WSASTARTUP"},
	{0, NULL}
};

static ERR_STRING_DATA BIO_str_reasons[] = {
	{ERR_REASON(BIO_R_ACCEPT_ERROR)          , "accept error"},
	{ERR_REASON(BIO_R_BAD_FOPEN_MODE)        , "bad fopen mode"},
	{ERR_REASON(BIO_R_BAD_HOSTNAME_LOOKUP)   , "bad hostname lookup"},
	{ERR_REASON(BIO_R_BROKEN_PIPE)           , "broken pipe"},
	{ERR_REASON(BIO_R_CONNECT_ERROR)         , "connect error"},
	{ERR_REASON(BIO_R_EOF_ON_MEMORY_BIO)     , "EOF on memory BIO"},
	{ERR_REASON(BIO_R_ERROR_SETTING_NBIO)    , "error setting nbio"},
	{ERR_REASON(BIO_R_ERROR_SETTING_NBIO_ON_ACCEPTED_SOCKET), "error setting nbio on accepted socket"},
	{ERR_REASON(BIO_R_ERROR_SETTING_NBIO_ON_ACCEPT_SOCKET), "error setting nbio on accept socket"},
	{ERR_REASON(BIO_R_GETHOSTBYNAME_ADDR_IS_NOT_AF_INET), "gethostbyname addr is not af inet"},
	{ERR_REASON(BIO_R_INVALID_ARGUMENT)      , "invalid argument"},
	{ERR_REASON(BIO_R_INVALID_IP_ADDRESS)    , "invalid ip address"},
	{ERR_REASON(BIO_R_IN_USE)                , "in use"},
	{ERR_REASON(BIO_R_KEEPALIVE)             , "keepalive"},
	{ERR_REASON(BIO_R_NBIO_CONNECT_ERROR)    , "nbio connect error"},
	{ERR_REASON(BIO_R_NO_ACCEPT_PORT_SPECIFIED), "no accept port specified"},
	{ERR_REASON(BIO_R_NO_HOSTNAME_SPECIFIED) , "no hostname specified"},
	{ERR_REASON(BIO_R_NO_PORT_DEFINED)       , "no port defined"},
	{ERR_REASON(BIO_R_NO_PORT_SPECIFIED)     , "no port specified"},
	{ERR_REASON(BIO_R_NO_SUCH_FILE)          , "no such file"},
	{ERR_REASON(BIO_R_NULL_PARAMETER)        , "null parameter"},
	{ERR_REASON(BIO_R_TAG_MISMATCH)          , "tag mismatch"},
	{ERR_REASON(BIO_R_UNABLE_TO_BIND_SOCKET) , "unable to bind socket"},
	{ERR_REASON(BIO_R_UNABLE_TO_CREATE_SOCKET), "unable to create socket"},
	{ERR_REASON(BIO_R_UNABLE_TO_LISTEN_SOCKET), "unable to listen socket"},
	{ERR_REASON(BIO_R_UNINITIALIZED)         , "uninitialized"},
	{ERR_REASON(BIO_R_UNSUPPORTED_METHOD)    , "unsupported method"},
	{ERR_REASON(BIO_R_WRITE_TO_READ_ONLY_BIO), "write to read only BIO"},
	{ERR_REASON(BIO_R_WSASTARTUP)            , "WSAStartup"},
	{0, NULL}
};

#endif

void
ERR_load_BIO_strings(void)
{
#ifndef OPENSSL_NO_ERR
	if (ERR_func_error_string(BIO_str_functs[0].error) == NULL) {
		ERR_load_strings(0, BIO_str_functs);
		ERR_load_strings(0, BIO_str_reasons);
	}
#endif
}
