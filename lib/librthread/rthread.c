/*	$OpenBSD: rthread.c,v 1.45 2011/11/09 10:28:01 guenther Exp $ */
/*
 * Copyright (c) 2004,2005 Ted Unangst <tedu@openbsd.org>
 * All Rights Reserved.
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
/*
 * The heart of rthreads.  Basic functions like creating and joining
 * threads.
 */

#include <sys/types.h>

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dlfcn.h>

#include <pthread.h>

#include "thread_private.h"	/* in libc/include */
#include "rthread.h"
#include "tcb.h"

static int concurrency_level;	/* not used */

int _threads_ready;
struct listhead _thread_list = LIST_HEAD_INITIALIZER(_thread_list);
_spinlock_lock_t _thread_lock = _SPINLOCK_UNLOCKED;
static struct pthread_queue _thread_gc_list
    = TAILQ_HEAD_INITIALIZER(_thread_gc_list);
static _spinlock_lock_t _thread_gc_lock = _SPINLOCK_UNLOCKED;
struct pthread _initial_thread;
struct thread_control_block _initial_thread_tcb;

int __tfork_thread(const struct __tfork *, void *, void (*)(void *), void *);

/*
 * internal support functions
 */
void
_spinlock(_spinlock_lock_t *lock)
{

	while (_atomic_lock(lock))
		pthread_yield();
}

void
_spinunlock(_spinlock_lock_t *lock)
{

	*lock = _SPINLOCK_UNLOCKED;
}

/*
 * This sets up the thread base for the initial thread so that it
 * references the errno location provided by libc.  For other threads
 * this is handled by the block in _rthread_start().
 */
void _rthread_initlib(void) __attribute__((constructor));
void _rthread_initlib(void)
{
	struct thread_control_block *tcb = &_initial_thread_tcb;

	/* use libc's errno for the main thread */
	TCB_INIT(tcb, &_initial_thread, ___errno());
	TCB_SET(tcb);
}

int *
__errno(void)
{
	return (TCB_ERRNOPTR());
}

static void
_rthread_start(void *v)
{
	pthread_t thread = v;
	void *retval;

	retval = thread->fn(thread->arg);
	pthread_exit(retval);
}

static int
_rthread_init(void)
{
	pthread_t thread = &_initial_thread;
	extern int __isthreaded;

	thread->tid = getthrid();
	thread->donesem.lock = _SPINLOCK_UNLOCKED;
	thread->flags |= THREAD_CANCEL_ENABLE|THREAD_CANCEL_DEFERRED;
	thread->flags_lock = _SPINLOCK_UNLOCKED;
	strlcpy(thread->name, "Main process", sizeof(thread->name));
	LIST_INSERT_HEAD(&_thread_list, thread, threads);
	_rthread_debug_init();
	_rthread_debug(1, "rthread init\n");
	_threads_ready = 1;
	__isthreaded = 1;

#if defined(__ELF__) && defined(PIC)
	/*
	 * To avoid recursion problems in ld.so, we need to trigger the
	 * functions once to fully bind them before registering them
	 * for use.
	 */
	_rthread_dl_lock(0);
	_rthread_dl_lock(1);
	_rthread_bind_lock(0);
	_rthread_bind_lock(1);
	dlctl(NULL, DL_SETTHREADLCK, _rthread_dl_lock);
	dlctl(NULL, DL_SETBINDLCK, _rthread_bind_lock);
#endif

	return (0);
}

static void
_rthread_free(pthread_t thread)
{
	/* catch wrongdoers for the moment */
	/* initial_thread.tid must remain valid */
	if (thread != &_initial_thread) {
		struct stack *stack = thread->stack;
		pid_t tid = thread->tid;
		void *arg = thread->arg;

		/* catch wrongdoers for the moment */
		memset(thread, 0xd0, sizeof(*thread));
		thread->stack = stack;
		thread->tid = tid;
		thread->arg = arg;
		_spinlock(&_thread_gc_lock);
		TAILQ_INSERT_TAIL(&_thread_gc_list, thread, waiting);
		_spinunlock(&_thread_gc_lock);
	}
}

static void
_rthread_setflag(pthread_t thread, int flag)
{
	_spinlock(&thread->flags_lock);
	thread->flags |= flag;
	_spinunlock(&thread->flags_lock);
}

static void
_rthread_clearflag(pthread_t thread, int flag)
{
	_spinlock(&thread->flags_lock);
	thread->flags &= ~flag;
	_spinunlock(&thread->flags_lock);
}

/*
 * real pthread functions
 */
pthread_t
pthread_self(void)
{
	if (!_threads_ready)
		if (_rthread_init())
			return (NULL);

	return (TCB_THREAD());
}

static void
_rthread_reaper(void)
{
	pthread_t thread;

restart:_spinlock(&_thread_gc_lock);
	TAILQ_FOREACH(thread, &_thread_gc_list, waiting) {
		if (thread->tid != 0)
			continue;
		TAILQ_REMOVE(&_thread_gc_list, thread, waiting);
		_spinunlock(&_thread_gc_lock);
		_rthread_debug(3, "rthread reaping %p stack %p\n",
		    (void *)thread, (void *)thread->stack);
		_rthread_free_stack(thread->stack);
		_rtld_free_tls(thread->arg,
		    sizeof(struct thread_control_block), sizeof(void *));
		free(thread);
		goto restart;
	}
	_spinunlock(&_thread_gc_lock);
}

void
pthread_exit(void *retval)
{
	struct rthread_cleanup_fn *clfn;
	pthread_t thread = pthread_self();

	thread->retval = retval;

	for (clfn = thread->cleanup_fns; clfn; ) {
		struct rthread_cleanup_fn *oclfn = clfn;
		clfn = clfn->next;
		oclfn->fn(oclfn->arg);
		free(oclfn);
	}
	_rthread_tls_destructors(thread);
	_spinlock(&_thread_lock);
	LIST_REMOVE(thread, threads);
	_spinunlock(&_thread_lock);

#ifdef TCB_GET
	thread->arg = TCB_GET();
#else
	thread->arg = __get_tcb();
#endif
	if (thread->flags & THREAD_DETACHED)
		_rthread_free(thread);
	else {
		_rthread_setflag(thread, THREAD_DONE);
		_sem_post(&thread->donesem);
	}

	threxit(&thread->tid);
	for(;;);
}

int
pthread_join(pthread_t thread, void **retval)
{
	int e;

	if (thread == NULL)
		e = EINVAL;
	else if (thread->tid == getthrid())
		e = EDEADLK;
	else if (thread->flags & THREAD_DETACHED)
		e = EINVAL;
	else {
		_sem_wait(&thread->donesem, 0);
		if (retval)
			*retval = thread->retval;
		e = 0;
		/* We should be the last having a ref to this thread, but
		 * someone stupid or evil might haved detached it;
		 * in that case the thread will cleanup itself */
		if ((thread->flags & THREAD_DETACHED) == 0)
			_rthread_free(thread);
	}

	_rthread_reaper();
	return (e);
}

int
pthread_detach(pthread_t thread)
{
	int rc = 0;

	_spinlock(&thread->flags_lock);
	if (thread->flags & THREAD_DETACHED) {
		rc = EINVAL;
		_spinunlock(&thread->flags_lock);
	} else if (thread->flags & THREAD_DONE) {
		_spinunlock(&thread->flags_lock);
		_rthread_free(thread);
	} else {
		thread->flags |= THREAD_DETACHED;
		_spinunlock(&thread->flags_lock);
	}
	_rthread_reaper();
	return (rc);
}

int
pthread_create(pthread_t *threadp, const pthread_attr_t *attr,
    void *(*start_routine)(void *), void *arg)
{
	struct thread_control_block *tcb;
	pthread_t thread;
	struct __tfork param;
	int rc = 0;

	if (!_threads_ready)
		if ((rc = _rthread_init()))
		    return (rc);

	_rthread_reaper();

	thread = calloc(1, sizeof(*thread));
	if (!thread)
		return (errno);
	thread->donesem.lock = _SPINLOCK_UNLOCKED;
	thread->flags_lock = _SPINLOCK_UNLOCKED;
	thread->fn = start_routine;
	thread->arg = arg;
	thread->tid = -1;

	if (attr)
		thread->attr = *(*attr);
	else {
		thread->attr.stack_size = RTHREAD_STACK_SIZE_DEF;
		thread->attr.guard_size = sysconf(_SC_PAGESIZE);
		thread->attr.stack_size -= thread->attr.guard_size;
	}
	if (thread->attr.detach_state == PTHREAD_CREATE_DETACHED)
		thread->flags |= THREAD_DETACHED;
	thread->flags |= THREAD_CANCEL_ENABLE|THREAD_CANCEL_DEFERRED;

	thread->stack = _rthread_alloc_stack(thread);
	if (!thread->stack) {
		rc = errno;
		goto fail1;
	}

	tcb = _rtld_allocate_tls(NULL, sizeof(*tcb), sizeof(void *));
	if (tcb == NULL) {
		rc = errno;
		goto fail2;
	}
	TCB_INIT(tcb, thread, &thread->myerrno);

	param.tf_tcb = tcb;
	param.tf_tid = &thread->tid;
	param.tf_flags = 0;

	_spinlock(&_thread_lock);
	LIST_INSERT_HEAD(&_thread_list, thread, threads);
	_spinunlock(&_thread_lock);

	rc = __tfork_thread(&param, thread->stack->sp, _rthread_start, thread);
	if (rc != -1) {
		/* success */
		*threadp = thread;
		return (0);
	}
		
	rc = errno;

	_spinlock(&_thread_lock);
	LIST_REMOVE(thread, threads);
	_spinunlock(&_thread_lock);
	_rtld_free_tls(tcb, sizeof(*tcb), sizeof(void *));
fail2:
	_rthread_free_stack(thread->stack);
fail1:
	_rthread_free(thread);

	return (rc);
}

int
pthread_kill(pthread_t thread, int sig)
{
	return (kill(thread->tid, sig));
}

int
pthread_equal(pthread_t t1, pthread_t t2)
{
	return (t1 == t2);
}

int
pthread_cancel(pthread_t thread)
{

	_rthread_setflag(thread, THREAD_CANCELLED);
	return (0);
}

void
pthread_testcancel(void)
{
	if ((pthread_self()->flags & (THREAD_CANCELLED|THREAD_CANCEL_ENABLE)) ==
	    (THREAD_CANCELLED|THREAD_CANCEL_ENABLE))
		pthread_exit(PTHREAD_CANCELED);

}

int
pthread_setcancelstate(int state, int *oldstatep)
{
	pthread_t self = pthread_self();
	int oldstate;

	oldstate = self->flags & THREAD_CANCEL_ENABLE ?
	    PTHREAD_CANCEL_ENABLE : PTHREAD_CANCEL_DISABLE;
	if (state == PTHREAD_CANCEL_ENABLE) {
		_rthread_setflag(self, THREAD_CANCEL_ENABLE);
		pthread_testcancel();
	} else if (state == PTHREAD_CANCEL_DISABLE) {
		_rthread_clearflag(self, THREAD_CANCEL_ENABLE);
	} else {
		return (EINVAL);
	}
	if (oldstatep)
		*oldstatep = oldstate;

	return (0);
}

int
pthread_setcanceltype(int type, int *oldtypep)
{
	pthread_t self = pthread_self();
	int oldtype;

	oldtype = self->flags & THREAD_CANCEL_DEFERRED ?
	    PTHREAD_CANCEL_DEFERRED : PTHREAD_CANCEL_ASYNCHRONOUS;
	if (type == PTHREAD_CANCEL_DEFERRED) {
		_rthread_setflag(self, THREAD_CANCEL_DEFERRED);
		pthread_testcancel();
	} else if (type == PTHREAD_CANCEL_ASYNCHRONOUS) {
		_rthread_clearflag(self, THREAD_CANCEL_DEFERRED);
	} else {
		return (EINVAL);
	}
	if (oldtypep)
		*oldtypep = oldtype;

	return (0);
}

void
pthread_cleanup_push(void (*fn)(void *), void *arg)
{
	struct rthread_cleanup_fn *clfn;
	pthread_t self = pthread_self();

	clfn = calloc(1, sizeof(*clfn));
	if (!clfn)
		return;
	clfn->fn = fn;
	clfn->arg = arg;
	clfn->next = self->cleanup_fns;
	self->cleanup_fns = clfn;
}

void
pthread_cleanup_pop(int execute)
{
	struct rthread_cleanup_fn *clfn;
	pthread_t self = pthread_self();

	clfn = self->cleanup_fns;
	if (clfn) {
		self->cleanup_fns = clfn->next;
		if (execute)
			clfn->fn(clfn->arg);
		free(clfn);
	}
}

int
pthread_getconcurrency(void)
{
	return (concurrency_level);
}

int
pthread_setconcurrency(int new_level)
{
	if (new_level < 0)
		return (EINVAL);
	concurrency_level = new_level;
	return (0);
}

/*
 * compat debug stuff
 */
void
_thread_dump_info(void)
{
	pthread_t thread;

	_spinlock(&_thread_lock);
	LIST_FOREACH(thread, &_thread_list, threads)
		printf("thread %d flags %d name %s\n",
		    thread->tid, thread->flags, thread->name);
	_spinunlock(&_thread_lock);
}

#if defined(__ELF__) && defined(PIC)
void
_rthread_dl_lock(int what)
{
	static _spinlock_lock_t lock = _SPINLOCK_UNLOCKED;

	if (what == 0)
		_spinlock(&lock);
	else
		_spinunlock(&lock);
}

void
_rthread_bind_lock(int what)
{
	static _spinlock_lock_t lock = _SPINLOCK_UNLOCKED;

	if (what == 0)
		_spinlock(&lock);
	else
		_spinunlock(&lock);
}
#endif
