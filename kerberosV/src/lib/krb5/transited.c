/*
 * Copyright (c) 1997 - 2000 Kungliga Tekniska H�gskolan
 * (Royal Institute of Technology, Stockholm, Sweden). 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 *
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 *
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 */

#include "krb5_locl.h"

RCSID("$KTH: transited.c,v 1.7 2000/02/07 13:30:41 joda Exp $");

/* this is an attempt at one of the most horrible `compression'
   schemes that has ever been invented; it's so amazingly brain-dead
   that words can not describe it, and all this just to save a few
   silly bytes */

struct tr_realm {
    char *realm;
    unsigned leading_space:1;
    unsigned leading_slash:1;
    unsigned trailing_dot:1;
    struct tr_realm *next;
};

static void
free_realms(struct tr_realm *r)
{
    struct tr_realm *p;
    while(r){
	p = r;
	r = r->next;
	free(p->realm);
	free(p);
    }	
}

static int
make_path(struct tr_realm *r, const char *from, const char *to)
{
    const char *p;
    struct tr_realm *path = r->next;
    struct tr_realm *tmp;

    if(strlen(from) < strlen(to)){
	const char *tmp;
	tmp = from;
	from = to;
	to = tmp;
    }
	
    if(strcmp(from + strlen(from) - strlen(to), to) == 0){
	p = from;
	while(1){
	    p = strchr(p, '.');
	    if(p == NULL)
		return KRB5KDC_ERR_POLICY;
	    p++;
	    if(strcmp(p, to) == 0)
		break;
	    tmp = calloc(1, sizeof(*tmp));
	    tmp->next = path;
	    path = tmp;
	    path->realm = strdup(p);
	    if(path->realm == NULL){
		r->next = path; /* XXX */
		return ENOMEM;;
	    }
	}
    }else if(strncmp(from, to, strlen(to)) == 0){
	p = from + strlen(from);
	while(1){
	    while(p >= from && *p != '/') p--;
	    if(p == from)
		return KRB5KDC_ERR_POLICY;
	    if(strncmp(to, from, p - from) == 0)
		break;
	    tmp = calloc(1, sizeof(*tmp));
	    tmp->next = path;
	    path = tmp;
	    path->realm = malloc(p - from + 1);
	    if(path->realm == NULL){
		r->next = path; /* XXX */
		return ENOMEM;
	    }
	    memcpy(path->realm, from, p - from);
	    path->realm[p - from] = '\0';
	    p--;
	}
    }else
	return KRB5KDC_ERR_POLICY;
    r->next = path;
    
    return 0;
}

static int
make_paths(struct tr_realm *realms, const char *client_realm, 
	   const char *server_realm)
{
    struct tr_realm *r;
    int ret;
    const char *prev_realm = client_realm;
    const char *next_realm = NULL;
    for(r = realms; r; r = r->next){
	/* it *might* be that you can have more than one empty
	   component in a row, at least that's how I interpret the
	   "," exception in 1510 */
	if(r->realm[0] == '\0'){
	    while(r->next && r->next->realm[0] == '\0')
		r = r->next;
	    if(r->next)
		next_realm = r->next->realm;
	    else
		next_realm = server_realm;
	    ret = make_path(r, prev_realm, next_realm);
	    if(ret){
		free_realms(realms);
		return ret;
	    }
	}
	prev_realm = r->realm;
    }
    return 0;
}

static int
expand_realms(struct tr_realm *realms, const char *client_realm)
{
    struct tr_realm *r;
    const char *prev_realm = NULL;
    for(r = realms; r; r = r->next){
	if(r->trailing_dot){
	    char *tmp;
	    if(prev_realm == NULL)
		prev_realm = client_realm;
	    tmp = realloc(r->realm, strlen(r->realm) + strlen(prev_realm) + 1);
	    if(tmp == NULL){
		free_realms(realms);
		return ENOMEM;
	    }
	    r->realm = tmp;
	    strcat(r->realm, prev_realm);
	}else if(r->leading_slash && !r->leading_space && prev_realm){
	    /* yet another exception: if you use x500-names, the
               leading realm doesn't have to be "quoted" with a space */
	    char *tmp;
	    tmp = malloc(strlen(r->realm) + strlen(prev_realm) + 1);
	    if(tmp == NULL){
		free_realms(realms);
		return ENOMEM;
	    }
	    strcpy(tmp, prev_realm);
	    strcat(tmp, r->realm);
	    free(r->realm);
	    r->realm = tmp;
	}
	prev_realm = r->realm;
    }
    return 0;
}

static struct tr_realm *
make_realm(char *realm)
{
    struct tr_realm *r;
    char *p, *q;
    int quote = 0;
    r = calloc(1, sizeof(*r));
    if(r == NULL){
	free(realm);
	return NULL;
    }
    r->realm = realm;
    for(p = q = r->realm; *p; p++){
	if(p == r->realm && *p == ' '){
	    r->leading_space = 1;
	    continue;
	}
	if(q == r->realm && *p == '/')
	    r->leading_slash = 1;
	if(quote){
	    *q++ = *p;
	    quote = 0;
	    continue;
	}
	if(*p == '\\'){
	    quote = 1;
	    continue;
	}
	if(p[0] == '.' && p[1] == '\0')
	    r->trailing_dot = 1;
	*q++ = *p;
    }
    *q = '\0';
    return r;
}

static struct tr_realm*
append_realm(struct tr_realm *head, struct tr_realm *r)
{
    struct tr_realm *p;
    if(head == NULL){
	r->next = NULL;
	return r;
    }
    p = head;
    while(p->next) p = p->next;
    p->next = r;
    return head;
}

static int
decode_realms(const char *tr, int length, struct tr_realm **realms)
{
    struct tr_realm *r = NULL;

    char *tmp;
    int quote = 0;
    const char *start = tr;
    int i;

    for(i = 0; i < length; i++){
	if(quote){
	    quote = 0;
	    continue;
	}
	if(tr[i] == '\\'){
	    quote = 1;
	    continue;
	}
	if(tr[i] == ','){
	    tmp = malloc(tr + i - start + 1);
	    memcpy(tmp, start, tr + i - start);
	    tmp[tr + i - start] = '\0';
	    r = make_realm(tmp);
	    if(r == NULL){
		free_realms(*realms);
		return ENOMEM;
	    }
	    *realms = append_realm(*realms, r);
	    start = tr + i + 1;
	}
    }
    tmp = malloc(tr + i - start + 1);
    memcpy(tmp, start, tr + i - start);
    tmp[tr + i - start] = '\0';
    r = make_realm(tmp);
    if(r == NULL){
	free_realms(*realms);
	return ENOMEM;
    }
    *realms = append_realm(*realms, r);
    
    return 0;
}


krb5_error_code
krb5_domain_x500_decode(krb5_data tr, char ***realms, int *num_realms, 
			const char *client_realm, const char *server_realm)
{
    struct tr_realm *r = NULL;
    struct tr_realm *p, **q;
    int ret;
    
    /* split string in components */
    ret = decode_realms(tr.data, tr.length, &r);
    if(ret)
	return ret;
    
    /* apply prefix rule */
    ret = expand_realms(r, client_realm);
    if(ret)
	return ret;
    
    ret = make_paths(r, client_realm, server_realm);
    if(ret)
	return ret;
    
    /* remove empty components */
    q = &r;
    for(p = r; p; ){
	if(p->realm[0] == '\0'){
	    free(p->realm);
	    *q = p->next;
	    free(p);
	    p = *q;
	}else{
	    q = &p->next;
	    p = p->next;
	}
    }
    {
	char **R;
	*realms = NULL;
	*num_realms = 0;
	while(r){
	    R = realloc(*realms, (*num_realms + 1) * sizeof(**realms));
	    if(R == NULL) {
		free(*realms);
		return ENOMEM;
	    }
	    R[*num_realms] = r->realm;
	    (*num_realms)++;
	    *realms = R;
	    p = r->next;
	    free(r);
	    r = p;
	}
    }
    return 0;
}

krb5_error_code
krb5_domain_x500_encode(char **realms, int num_realms, krb5_data *encoding)
{
    char *s = NULL;
    int len = 0;
    int i;
    for(i = 0; i < num_realms; i++){
	len += strlen(realms[i]);
	if(realms[i][0] == '/')
	    len++;
    }
    len += num_realms - 1;
    s = malloc(len + 1);
    *s = '\0';
    for(i = 0; i < num_realms; i++){
	if(i && i < num_realms - 1)
	    strcat(s, ",");
	if(realms[i][0] == '/')
	    strcat(s, " ");
	strcat(s, realms[i]);
    }
    encoding->data = s;
    encoding->length = strlen(s);
    return 0;
}

krb5_error_code
krb5_check_transited_realms(krb5_context context,
			    const char *const *realms, 
			    int num_realms, 
			    int *bad_realm)
{
    int i;
    int ret = 0;
    char **bad_realms = krb5_config_get_strings(context, NULL, 
						"libdefaults", 
						"transited_realms_reject", 
						NULL);
    if(bad_realms == NULL)
	return 0;

    for(i = 0; i < num_realms; i++) {
	char **p;
	for(p = bad_realms; *p; p++)
	    if(strcmp(*p, realms[i]) == 0) {
		ret = KRB5KRB_AP_ERR_ILL_CR_TKT;
		if(bad_realm)
		    *bad_realm = i;
		break;
	    }
    }
    krb5_config_free_strings(bad_realms);
    return ret;
}

#if 0
int
main(int argc, char **argv)
{
    krb5_data x;
    char **r;
    int num, i;
    x.data = argv[1];
    x.length = strlen(x.data);
    if(domain_expand(x, &r, &num, argv[2], argv[3]))
	exit(1);
    for(i = 0; i < num; i++)
	printf("%s\n", r[i]);
    return 0;
}
#endif

