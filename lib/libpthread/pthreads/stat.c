/* ==== stat.c ============================================================
 * Copyright (c) 1995 by Chris Provenzano, proven@mit.edu
 * All rights reserved.
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
 *  This product includes software developed by Chris Provenzano.
 * 4. The name of Chris Provenzano may not be used to endorse or promote 
 *	  products derived from this software without specific prior written
 *	  permission.
 *
 * THIS SOFTWARE IS PROVIDED BY CHRIS PROVENZANO ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL CHRIS PROVENZANO BE LIABLE FOR ANY 
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE.
 *
 * Description : All the syscalls dealing with fds.
 *
 *  1.00 93/05/27 proven
 *      -Started coding this file.
 */

#ifndef lint
static const char rcsid[] = "$Id: stat.c,v 1.2 1998/07/22 10:46:55 peter Exp $";
#endif

#include <pthread.h>
#include <errno.h>

struct stat;
struct statfs;

/* ==========================================================================
 * fstat()
 *
 * Might want to indirect this.
 */
int fstat(int fd, struct stat *buf)
{
	int ret;

	if ((ret = fd_lock(fd, FD_READ, NULL)) == OK) {
		if ((ret = machdep_sys_fstat(fd_table[fd]->fd.i, buf)) < OK) {
			SET_ERRNO(-ret);
			ret = NOTOK;
		}
		fd_unlock(fd, FD_READ);
	}
	return(ret);
}

/* ==========================================================================
 * stat()
 */
int stat(const char * path, struct stat * buf)
{
	int ret;

	if ((ret = machdep_sys_stat(path, buf)) < OK) {
		SET_ERRNO(-ret);
		ret = NOTOK;
	}
	return(ret);

}

/* ==========================================================================
 * lstat()
 */
int lstat(const char * path, struct stat * buf)
{
	int ret;

	if ((ret = machdep_sys_lstat(path, buf)) < OK) {
		SET_ERRNO(-ret);
		ret = NOTOK;
	}
	return(ret);

}

/* ==========================================================================
 * fstatfs()
 *
 * Might want to indirect this.
 */
int fstatfs(int fd, struct statfs *buf)
{
	int ret;

	if ((ret = fd_lock(fd, FD_READ, NULL)) == OK) {
		if ((ret = machdep_sys_fstatfs(fd_table[fd]->fd.i, buf)) < OK) {
			SET_ERRNO(-ret);
			ret = NOTOK;
		}
		fd_unlock(fd, FD_READ);
	}
	return(ret);
}
