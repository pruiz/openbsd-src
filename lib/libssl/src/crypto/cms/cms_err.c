/* $OpenBSD: cms_err.c,v 1.3 2014/06/12 15:49:28 deraadt Exp $ */
/* ====================================================================
 * Copyright (c) 1999-2009 The OpenSSL Project.  All rights reserved.
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
#include <openssl/cms.h>

/* BEGIN ERROR CODES */
#ifndef OPENSSL_NO_ERR

#define ERR_FUNC(func) ERR_PACK(ERR_LIB_CMS,func,0)
#define ERR_REASON(reason) ERR_PACK(ERR_LIB_CMS,0,reason)

static ERR_STRING_DATA CMS_str_functs[]= {
	{ERR_FUNC(CMS_F_CHECK_CONTENT),	"CHECK_CONTENT"},
	{ERR_FUNC(CMS_F_CMS_ADD0_CERT),	"CMS_add0_cert"},
	{ERR_FUNC(CMS_F_CMS_ADD0_RECIPIENT_KEY),	"CMS_add0_recipient_key"},
	{ERR_FUNC(CMS_F_CMS_ADD0_RECIPIENT_PASSWORD),	"CMS_add0_recipient_password"},
	{ERR_FUNC(CMS_F_CMS_ADD1_RECEIPTREQUEST),	"CMS_add1_ReceiptRequest"},
	{ERR_FUNC(CMS_F_CMS_ADD1_RECIPIENT_CERT),	"CMS_add1_recipient_cert"},
	{ERR_FUNC(CMS_F_CMS_ADD1_SIGNER),	"CMS_add1_signer"},
	{ERR_FUNC(CMS_F_CMS_ADD1_SIGNINGTIME),	"CMS_ADD1_SIGNINGTIME"},
	{ERR_FUNC(CMS_F_CMS_COMPRESS),	"CMS_compress"},
	{ERR_FUNC(CMS_F_CMS_COMPRESSEDDATA_CREATE),	"cms_CompressedData_create"},
	{ERR_FUNC(CMS_F_CMS_COMPRESSEDDATA_INIT_BIO),	"cms_CompressedData_init_bio"},
	{ERR_FUNC(CMS_F_CMS_COPY_CONTENT),	"CMS_COPY_CONTENT"},
	{ERR_FUNC(CMS_F_CMS_COPY_MESSAGEDIGEST),	"CMS_COPY_MESSAGEDIGEST"},
	{ERR_FUNC(CMS_F_CMS_DATA),	"CMS_data"},
	{ERR_FUNC(CMS_F_CMS_DATAFINAL),	"CMS_dataFinal"},
	{ERR_FUNC(CMS_F_CMS_DATAINIT),	"CMS_dataInit"},
	{ERR_FUNC(CMS_F_CMS_DECRYPT),	"CMS_decrypt"},
	{ERR_FUNC(CMS_F_CMS_DECRYPT_SET1_KEY),	"CMS_decrypt_set1_key"},
	{ERR_FUNC(CMS_F_CMS_DECRYPT_SET1_PASSWORD),	"CMS_decrypt_set1_password"},
	{ERR_FUNC(CMS_F_CMS_DECRYPT_SET1_PKEY),	"CMS_decrypt_set1_pkey"},
	{ERR_FUNC(CMS_F_CMS_DIGESTALGORITHM_FIND_CTX),	"cms_DigestAlgorithm_find_ctx"},
	{ERR_FUNC(CMS_F_CMS_DIGESTALGORITHM_INIT_BIO),	"cms_DigestAlgorithm_init_bio"},
	{ERR_FUNC(CMS_F_CMS_DIGESTEDDATA_DO_FINAL),	"cms_DigestedData_do_final"},
	{ERR_FUNC(CMS_F_CMS_DIGEST_VERIFY),	"CMS_digest_verify"},
	{ERR_FUNC(CMS_F_CMS_ENCODE_RECEIPT),	"cms_encode_Receipt"},
	{ERR_FUNC(CMS_F_CMS_ENCRYPT),	"CMS_encrypt"},
	{ERR_FUNC(CMS_F_CMS_ENCRYPTEDCONTENT_INIT_BIO),	"cms_EncryptedContent_init_bio"},
	{ERR_FUNC(CMS_F_CMS_ENCRYPTEDDATA_DECRYPT),	"CMS_EncryptedData_decrypt"},
	{ERR_FUNC(CMS_F_CMS_ENCRYPTEDDATA_ENCRYPT),	"CMS_EncryptedData_encrypt"},
	{ERR_FUNC(CMS_F_CMS_ENCRYPTEDDATA_SET1_KEY),	"CMS_EncryptedData_set1_key"},
	{ERR_FUNC(CMS_F_CMS_ENVELOPEDDATA_CREATE),	"CMS_EnvelopedData_create"},
	{ERR_FUNC(CMS_F_CMS_ENVELOPEDDATA_INIT_BIO),	"cms_EnvelopedData_init_bio"},
	{ERR_FUNC(CMS_F_CMS_ENVELOPED_DATA_INIT),	"CMS_ENVELOPED_DATA_INIT"},
	{ERR_FUNC(CMS_F_CMS_FINAL),	"CMS_final"},
	{ERR_FUNC(CMS_F_CMS_GET0_CERTIFICATE_CHOICES),	"CMS_GET0_CERTIFICATE_CHOICES"},
	{ERR_FUNC(CMS_F_CMS_GET0_CONTENT),	"CMS_get0_content"},
	{ERR_FUNC(CMS_F_CMS_GET0_ECONTENT_TYPE),	"CMS_GET0_ECONTENT_TYPE"},
	{ERR_FUNC(CMS_F_CMS_GET0_ENVELOPED),	"cms_get0_enveloped"},
	{ERR_FUNC(CMS_F_CMS_GET0_REVOCATION_CHOICES),	"CMS_GET0_REVOCATION_CHOICES"},
	{ERR_FUNC(CMS_F_CMS_GET0_SIGNED),	"CMS_GET0_SIGNED"},
	{ERR_FUNC(CMS_F_CMS_MSGSIGDIGEST_ADD1),	"cms_msgSigDigest_add1"},
	{ERR_FUNC(CMS_F_CMS_RECEIPTREQUEST_CREATE0),	"CMS_ReceiptRequest_create0"},
	{ERR_FUNC(CMS_F_CMS_RECEIPT_VERIFY),	"cms_Receipt_verify"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_DECRYPT),	"CMS_RecipientInfo_decrypt"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KEKRI_DECRYPT),	"CMS_RECIPIENTINFO_KEKRI_DECRYPT"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KEKRI_ENCRYPT),	"CMS_RECIPIENTINFO_KEKRI_ENCRYPT"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KEKRI_GET0_ID),	"CMS_RecipientInfo_kekri_get0_id"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KEKRI_ID_CMP),	"CMS_RecipientInfo_kekri_id_cmp"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KTRI_CERT_CMP),	"CMS_RecipientInfo_ktri_cert_cmp"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KTRI_DECRYPT),	"CMS_RECIPIENTINFO_KTRI_DECRYPT"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KTRI_ENCRYPT),	"CMS_RECIPIENTINFO_KTRI_ENCRYPT"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KTRI_GET0_ALGS),	"CMS_RecipientInfo_ktri_get0_algs"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_KTRI_GET0_SIGNER_ID),	"CMS_RecipientInfo_ktri_get0_signer_id"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_PWRI_CRYPT),	"cms_RecipientInfo_pwri_crypt"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_SET0_KEY),	"CMS_RecipientInfo_set0_key"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_SET0_PASSWORD),	"CMS_RecipientInfo_set0_password"},
	{ERR_FUNC(CMS_F_CMS_RECIPIENTINFO_SET0_PKEY),	"CMS_RecipientInfo_set0_pkey"},
	{ERR_FUNC(CMS_F_CMS_SET1_SIGNERIDENTIFIER),	"cms_set1_SignerIdentifier"},
	{ERR_FUNC(CMS_F_CMS_SET_DETACHED),	"CMS_set_detached"},
	{ERR_FUNC(CMS_F_CMS_SIGN),	"CMS_sign"},
	{ERR_FUNC(CMS_F_CMS_SIGNED_DATA_INIT),	"CMS_SIGNED_DATA_INIT"},
	{ERR_FUNC(CMS_F_CMS_SIGNERINFO_CONTENT_SIGN),	"CMS_SIGNERINFO_CONTENT_SIGN"},
	{ERR_FUNC(CMS_F_CMS_SIGNERINFO_SIGN),	"CMS_SignerInfo_sign"},
	{ERR_FUNC(CMS_F_CMS_SIGNERINFO_VERIFY),	"CMS_SignerInfo_verify"},
	{ERR_FUNC(CMS_F_CMS_SIGNERINFO_VERIFY_CERT),	"CMS_SIGNERINFO_VERIFY_CERT"},
	{ERR_FUNC(CMS_F_CMS_SIGNERINFO_VERIFY_CONTENT),	"CMS_SignerInfo_verify_content"},
	{ERR_FUNC(CMS_F_CMS_SIGN_RECEIPT),	"CMS_sign_receipt"},
	{ERR_FUNC(CMS_F_CMS_STREAM),	"CMS_stream"},
	{ERR_FUNC(CMS_F_CMS_UNCOMPRESS),	"CMS_uncompress"},
	{ERR_FUNC(CMS_F_CMS_VERIFY),	"CMS_verify"},
	{0, NULL}
};

static ERR_STRING_DATA CMS_str_reasons[]= {
	{ERR_REASON(CMS_R_ADD_SIGNER_ERROR)      , "add signer error"},
	{ERR_REASON(CMS_R_CERTIFICATE_ALREADY_PRESENT), "certificate already present"},
	{ERR_REASON(CMS_R_CERTIFICATE_HAS_NO_KEYID), "certificate has no keyid"},
	{ERR_REASON(CMS_R_CERTIFICATE_VERIFY_ERROR), "certificate verify error"},
	{ERR_REASON(CMS_R_CIPHER_INITIALISATION_ERROR), "cipher initialisation error"},
	{ERR_REASON(CMS_R_CIPHER_PARAMETER_INITIALISATION_ERROR), "cipher parameter initialisation error"},
	{ERR_REASON(CMS_R_CMS_DATAFINAL_ERROR)   , "cms datafinal error"},
	{ERR_REASON(CMS_R_CMS_LIB)               , "cms lib"},
	{ERR_REASON(CMS_R_CONTENTIDENTIFIER_MISMATCH), "contentidentifier mismatch"},
	{ERR_REASON(CMS_R_CONTENT_NOT_FOUND)     , "content not found"},
	{ERR_REASON(CMS_R_CONTENT_TYPE_MISMATCH) , "content type mismatch"},
	{ERR_REASON(CMS_R_CONTENT_TYPE_NOT_COMPRESSED_DATA), "content type not compressed data"},
	{ERR_REASON(CMS_R_CONTENT_TYPE_NOT_ENVELOPED_DATA), "content type not enveloped data"},
	{ERR_REASON(CMS_R_CONTENT_TYPE_NOT_SIGNED_DATA), "content type not signed data"},
	{ERR_REASON(CMS_R_CONTENT_VERIFY_ERROR)  , "content verify error"},
	{ERR_REASON(CMS_R_CTRL_ERROR)            , "ctrl error"},
	{ERR_REASON(CMS_R_CTRL_FAILURE)          , "ctrl failure"},
	{ERR_REASON(CMS_R_DECRYPT_ERROR)         , "decrypt error"},
	{ERR_REASON(CMS_R_DIGEST_ERROR)          , "digest error"},
	{ERR_REASON(CMS_R_ERROR_GETTING_PUBLIC_KEY), "error getting public key"},
	{ERR_REASON(CMS_R_ERROR_READING_MESSAGEDIGEST_ATTRIBUTE), "error reading messagedigest attribute"},
	{ERR_REASON(CMS_R_ERROR_SETTING_KEY)     , "error setting key"},
	{ERR_REASON(CMS_R_ERROR_SETTING_RECIPIENTINFO), "error setting recipientinfo"},
	{ERR_REASON(CMS_R_INVALID_ENCRYPTED_KEY_LENGTH), "invalid encrypted key length"},
	{ERR_REASON(CMS_R_INVALID_KEY_ENCRYPTION_PARAMETER), "invalid key encryption parameter"},
	{ERR_REASON(CMS_R_INVALID_KEY_LENGTH)    , "invalid key length"},
	{ERR_REASON(CMS_R_MD_BIO_INIT_ERROR)     , "md bio init error"},
	{ERR_REASON(CMS_R_MESSAGEDIGEST_ATTRIBUTE_WRONG_LENGTH), "messagedigest attribute wrong length"},
	{ERR_REASON(CMS_R_MESSAGEDIGEST_WRONG_LENGTH), "messagedigest wrong length"},
	{ERR_REASON(CMS_R_MSGSIGDIGEST_ERROR)    , "msgsigdigest error"},
	{ERR_REASON(CMS_R_MSGSIGDIGEST_VERIFICATION_FAILURE), "msgsigdigest verification failure"},
	{ERR_REASON(CMS_R_MSGSIGDIGEST_WRONG_LENGTH), "msgsigdigest wrong length"},
	{ERR_REASON(CMS_R_NEED_ONE_SIGNER)       , "need one signer"},
	{ERR_REASON(CMS_R_NOT_A_SIGNED_RECEIPT)  , "not a signed receipt"},
	{ERR_REASON(CMS_R_NOT_ENCRYPTED_DATA)    , "not encrypted data"},
	{ERR_REASON(CMS_R_NOT_KEK)               , "not kek"},
	{ERR_REASON(CMS_R_NOT_KEY_TRANSPORT)     , "not key transport"},
	{ERR_REASON(CMS_R_NOT_PWRI)              , "not pwri"},
	{ERR_REASON(CMS_R_NOT_SUPPORTED_FOR_THIS_KEY_TYPE), "not supported for this key type"},
	{ERR_REASON(CMS_R_NO_CIPHER)             , "no cipher"},
	{ERR_REASON(CMS_R_NO_CONTENT)            , "no content"},
	{ERR_REASON(CMS_R_NO_CONTENT_TYPE)       , "no content type"},
	{ERR_REASON(CMS_R_NO_DEFAULT_DIGEST)     , "no default digest"},
	{ERR_REASON(CMS_R_NO_DIGEST_SET)         , "no digest set"},
	{ERR_REASON(CMS_R_NO_KEY)                , "no key"},
	{ERR_REASON(CMS_R_NO_KEY_OR_CERT)        , "no key or cert"},
	{ERR_REASON(CMS_R_NO_MATCHING_DIGEST)    , "no matching digest"},
	{ERR_REASON(CMS_R_NO_MATCHING_RECIPIENT) , "no matching recipient"},
	{ERR_REASON(CMS_R_NO_MATCHING_SIGNATURE) , "no matching signature"},
	{ERR_REASON(CMS_R_NO_MSGSIGDIGEST)       , "no msgsigdigest"},
	{ERR_REASON(CMS_R_NO_PASSWORD)           , "no password"},
	{ERR_REASON(CMS_R_NO_PRIVATE_KEY)        , "no private key"},
	{ERR_REASON(CMS_R_NO_PUBLIC_KEY)         , "no public key"},
	{ERR_REASON(CMS_R_NO_RECEIPT_REQUEST)    , "no receipt request"},
	{ERR_REASON(CMS_R_NO_SIGNERS)            , "no signers"},
	{ERR_REASON(CMS_R_PRIVATE_KEY_DOES_NOT_MATCH_CERTIFICATE), "private key does not match certificate"},
	{ERR_REASON(CMS_R_RECEIPT_DECODE_ERROR)  , "receipt decode error"},
	{ERR_REASON(CMS_R_RECIPIENT_ERROR)       , "recipient error"},
	{ERR_REASON(CMS_R_SIGNER_CERTIFICATE_NOT_FOUND), "signer certificate not found"},
	{ERR_REASON(CMS_R_SIGNFINAL_ERROR)       , "signfinal error"},
	{ERR_REASON(CMS_R_SMIME_TEXT_ERROR)      , "smime text error"},
	{ERR_REASON(CMS_R_STORE_INIT_ERROR)      , "store init error"},
	{ERR_REASON(CMS_R_TYPE_NOT_COMPRESSED_DATA), "type not compressed data"},
	{ERR_REASON(CMS_R_TYPE_NOT_DATA)         , "type not data"},
	{ERR_REASON(CMS_R_TYPE_NOT_DIGESTED_DATA), "type not digested data"},
	{ERR_REASON(CMS_R_TYPE_NOT_ENCRYPTED_DATA), "type not encrypted data"},
	{ERR_REASON(CMS_R_TYPE_NOT_ENVELOPED_DATA), "type not enveloped data"},
	{ERR_REASON(CMS_R_UNABLE_TO_FINALIZE_CONTEXT), "unable to finalize context"},
	{ERR_REASON(CMS_R_UNKNOWN_CIPHER)        , "unknown cipher"},
	{ERR_REASON(CMS_R_UNKNOWN_DIGEST_ALGORIHM), "unknown digest algorihm"},
	{ERR_REASON(CMS_R_UNKNOWN_ID)            , "unknown id"},
	{ERR_REASON(CMS_R_UNSUPPORTED_COMPRESSION_ALGORITHM), "unsupported compression algorithm"},
	{ERR_REASON(CMS_R_UNSUPPORTED_CONTENT_TYPE), "unsupported content type"},
	{ERR_REASON(CMS_R_UNSUPPORTED_KEK_ALGORITHM), "unsupported kek algorithm"},
	{ERR_REASON(CMS_R_UNSUPPORTED_KEY_ENCRYPTION_ALGORITHM), "unsupported key encryption algorithm"},
	{ERR_REASON(CMS_R_UNSUPPORTED_RECIPIENT_TYPE), "unsupported recipient type"},
	{ERR_REASON(CMS_R_UNSUPPORTED_RECPIENTINFO_TYPE), "unsupported recpientinfo type"},
	{ERR_REASON(CMS_R_UNSUPPORTED_TYPE)      , "unsupported type"},
	{ERR_REASON(CMS_R_UNWRAP_ERROR)          , "unwrap error"},
	{ERR_REASON(CMS_R_UNWRAP_FAILURE)        , "unwrap failure"},
	{ERR_REASON(CMS_R_VERIFICATION_FAILURE)  , "verification failure"},
	{ERR_REASON(CMS_R_WRAP_ERROR)            , "wrap error"},
	{0, NULL}
};

#endif

void
ERR_load_CMS_strings(void)
{
#ifndef OPENSSL_NO_ERR
	if (ERR_func_error_string(CMS_str_functs[0].error) == NULL) {
		ERR_load_strings(0, CMS_str_functs);
		ERR_load_strings(0, CMS_str_reasons);
	}
#endif
}
