/*	$OpenBSD: rthread_fork.c,v 1.12 2015/05/10 18:33:15 guenther Exp $ */

/*
 * Copyright (c) 2008 Kurt Miller <kurt@openbsd.org>
 * Copyright (c) 2008 Philip Guenther <guenther@openbsd.org>
 * Copyright (c) 2003 Daniel Eischen <deischen@freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
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
 * $FreeBSD: /repoman/r/ncvs/src/lib/libc_r/uthread/uthread_atfork.c,v 1.1 2004/12/10 03:36:45 grog Exp $
 */

#if defined(__ELF__)
#include <sys/types.h>
#include <sys/exec_elf.h>
#pragma weak _DYNAMIC
#endif

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "thread_private.h"	/* in libc/include */

#include "rthread.h"

pid_t   _thread_sys_fork(void);
pid_t   _thread_sys_vfork(void);
pid_t	_dofork(int);

pid_t
_dofork(int is_vfork)
{
	pthread_t me;
	pid_t (*sys_fork)(void);
	pid_t newid;
#if defined(__ELF__)
	sigset_t nmask, omask;
#endif

	sys_fork = is_vfork ? &_thread_sys_vfork : &_thread_sys_fork;

	if (!_threads_ready)
		return sys_fork();

	me = pthread_self();

	/*
	 * Protect important libc/ld.so critical areas across the fork call.
	 * dlclose() will grab the atexit lock via __cxa_finalize() so lock
	 * the dl_lock first. malloc()/free() can grab the arc4 lock so lock
	 * malloc_lock first. Finally lock the bind_lock last so that any lazy
	 * binding in the other locking functions can succeed.
	 */

#if defined(__ELF__)
	if (_DYNAMIC)
		_rthread_dl_lock(0);
#endif

	_thread_atexit_lock();
	_thread_malloc_lock();
	_thread_arc4_lock();

#if defined(__ELF__)
	if (_DYNAMIC) {
		sigfillset(&nmask);
		_thread_sys_sigprocmask(SIG_BLOCK, &nmask, &omask);
		_rthread_bind_lock(0);
	}
#endif

	newid = sys_fork();

#if defined(__ELF__)
	if (_DYNAMIC) {
		_rthread_bind_lock(1);
		_thread_sys_sigprocmask(SIG_SETMASK, &omask, NULL);
	}
#endif

	_thread_arc4_unlock();
	_thread_malloc_unlock();
	_thread_atexit_unlock();

	if (newid == 0) {
#if defined(__ELF__)
		/* reinitialize the lock in the child */
		if (_DYNAMIC)
			_rthread_dl_lock(2);
#endif
		/* update this thread's structure */
		me->tid = getthrid();
		me->donesem.lock = _SPINLOCK_UNLOCKED_ASSIGN;
		me->flags &= ~THREAD_DETACHED;
		me->flags_lock = _SPINLOCK_UNLOCKED_ASSIGN;

		/* this thread is the initial thread for the new process */
		me->flags |= THREAD_ORIGINAL;

		/* reinit the thread list */
		LIST_INIT(&_thread_list);
		LIST_INSERT_HEAD(&_thread_list, me, threads);
		_thread_lock = _SPINLOCK_UNLOCKED_ASSIGN;

		/* single threaded now */
		__isthreaded = 0;
	}
#if defined(__ELF__)
	else if (_DYNAMIC)
		_rthread_dl_lock(1);
#endif
	return newid;
}

pid_t
_thread_fork(void)
{
	return _dofork(0);
}

pid_t
vfork(void)
{
	return _dofork(1);
}
