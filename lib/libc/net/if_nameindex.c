/*
 * %%% copyright-cmetz-98-bsd
 * Copyright (c) 1998-1999, Craig Metz, All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Craig Metz and
 *      by other contributors.
 * 4. Neither the name of the author nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Craig Metz and
 *      by other contributors.
 * 4. Neither the name of the author nor the names of contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <errno.h>

struct if_nameindex *if_nameindex(void)
{
  int i, j, fd;
  struct if_nameindex *nameindex = NULL;
  struct ifconf ifconf;
  void *p;
  int len;
  char lastname[IFNAMSIZ + 1];

  if ((fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    return NULL;

  ifconf.ifc_len = 0;
  ifconf.ifc_buf = 0;
  if (ioctl(fd, SIOCGIFCONF, (void *)&ifconf))
    goto ret;
  if (ifconf.ifc_len < IFNAMSIZ)
    goto ret;
  if (!(ifconf.ifc_buf = malloc(ifconf.ifc_len)))
    goto ret;
  if (ioctl(fd, SIOCGIFCONF, (void *)&ifconf))
    goto ret;

  i = sizeof(struct if_nameindex);
  j = 0;
  p = ifconf.ifc_buf;
  len = ifconf.ifc_len;
  lastname[0] = 0;
  lastname[IFNAMSIZ] = 0;

  while(len > 0) {
    if (len < (IFNAMSIZ + sizeof(struct sockaddr)))
      goto ret;
    if (strncmp(lastname, p, IFNAMSIZ)) {
      memcpy(lastname, p, IFNAMSIZ);
      i += sizeof(struct if_nameindex);
      j += strlen(lastname) + 1; 
    };
    len -= IFNAMSIZ;
    p += IFNAMSIZ;

    if (len < SA_LEN((struct sockaddr *)p))
      goto ret;
    len -= SA_LEN((struct sockaddr *)p);
    p += SA_LEN((struct sockaddr *)p);
  };

  if (!(nameindex = malloc(i + j))) {
    errno = ENOMEM;
    goto ret;
  };
  memset(nameindex, 0, i + j);

  {
  struct if_nameindex *n;
  char *c;

  n = nameindex;
  p = ifconf.ifc_buf;
  c = (void *)nameindex + i;
  i = 0;
  len = ifconf.ifc_len;
  lastname[0] = 0;

  while(len > 0) {
    if (len < (IFNAMSIZ + sizeof(struct sockaddr)))
      goto ret;
    if (strncmp(lastname, p, IFNAMSIZ)) {
      if (i) {
	if (!n->if_index)
	  n->if_index = i;
	n++;
      };
      i++;
      memcpy(lastname, p, IFNAMSIZ);
      strcpy(n->if_name = c, lastname);
      c += strlen(c) + 1;
    };
    len -= IFNAMSIZ;
    p += IFNAMSIZ;

    if (len < SA_LEN((struct sockaddr *)p))
      goto ret;
    if (((struct sockaddr *)p)->sa_family == AF_LINK)
      n->if_index = ((struct sockaddr_dl *)p)->sdl_index;
    len -= SA_LEN((struct sockaddr *)p);
    p += SA_LEN((struct sockaddr *)p);
  };

  if (!n->if_index)
    n->if_index = i;
  };

ret:
  close(fd);
  free(ifconf.ifc_buf);
  return nameindex;
};
