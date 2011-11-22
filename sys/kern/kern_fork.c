/*	$OpenBSD: kern_fork.c,v 1.132 2011/11/22 23:20:19 joshe Exp $	*/
/*	$NetBSD: kern_fork.c,v 1.29 1996/02/09 18:59:34 christos Exp $	*/

/*
 * Copyright (c) 1982, 1986, 1989, 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
 *	@(#)kern_fork.c	8.6 (Berkeley) 4/8/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/mount.h>
#include <sys/proc.h>
#include <sys/exec.h>
#include <sys/resourcevar.h>
#include <sys/signalvar.h>
#include <sys/vnode.h>
#include <sys/file.h>
#include <sys/acct.h>
#include <sys/ktrace.h>
#include <sys/sched.h>
#include <dev/rndvar.h>
#include <sys/pool.h>
#include <sys/mman.h>
#include <sys/ptrace.h>

#include <sys/syscallargs.h>

#include "systrace.h"
#include <dev/systrace.h>

#include <uvm/uvm_extern.h>
#include <uvm/uvm_map.h>

#ifdef __HAVE_MD_TCB
# include <machine/tcb.h>
#endif

int	nprocs = 1;		/* process 0 */
int	randompid;		/* when set to 1, pid's go random */
pid_t	lastpid;
struct	forkstat forkstat;

void fork_return(void *);
void tfork_child_return(void *);
int pidtaken(pid_t);

void process_new(struct proc *, struct process *);

void
fork_return(void *arg)
{
	struct proc *p = (struct proc *)arg;

	if (p->p_flag & P_TRACED)
		psignal(p, SIGTRAP);

	child_return(p);
}

/*ARGSUSED*/
int
sys_fork(struct proc *p, void *v, register_t *retval)
{
	int flags;

	flags = FORK_FORK;
	if (p->p_ptmask & PTRACE_FORK)
		flags |= FORK_PTRACE;
	return (fork1(p, SIGCHLD, flags, NULL, 0,
	    fork_return, NULL, retval, NULL));
}

/*ARGSUSED*/
int
sys_vfork(struct proc *p, void *v, register_t *retval)
{
	return (fork1(p, SIGCHLD, FORK_VFORK|FORK_PPWAIT, NULL, 0, NULL,
	    NULL, retval, NULL));
}

int
sys_rfork(struct proc *p, void *v, register_t *retval)
{
	struct sys_rfork_args /* {
		syscallarg(int) flags;
	} */ *uap = v;

	int rforkflags;
	int flags;

	flags = FORK_RFORK;
	rforkflags = SCARG(uap, flags);

	if ((rforkflags & RFPROC) == 0)
		return (EINVAL);

	switch(rforkflags & (RFFDG|RFCFDG)) {
	case (RFFDG|RFCFDG):
		return EINVAL;
	case RFCFDG:
		flags |= FORK_CLEANFILES;
		break;
	case RFFDG:
		break;
	default:
		flags |= FORK_SHAREFILES;
		break;
	}

	if (rforkflags & RFNOWAIT)
		flags |= FORK_NOZOMBIE;

	if (rforkflags & RFMEM)
		flags |= FORK_SHAREVM;

	if (rforkflags & RFTHREAD)
		flags |= FORK_THREAD | FORK_SIGHAND | FORK_NOZOMBIE;

	return (fork1(p, SIGCHLD, flags, NULL, 0, NULL, NULL, retval, NULL));
}

int
sys___tfork(struct proc *p, void *v, register_t *retval)
{
	struct sys___tfork_args /* {
		syscallarg(struct __tfork) *param;
	} */ *uap = v;
	struct __tfork param;
	int flags;
	int error;

	if ((error = copyin(SCARG(uap, param), &param, sizeof(param))))
		return (error);

	/* XXX will supersede rfork at some point... */
	if (param.tf_flags != 0)
		return (EINVAL);

	flags = FORK_TFORK | FORK_THREAD | FORK_SIGHAND | FORK_SHAREVM
	    | FORK_NOZOMBIE | FORK_SHAREFILES;

	return (fork1(p, 0, flags, NULL, param.tf_tid, tfork_child_return,
	    param.tf_tcb, retval, NULL));
}

void
tfork_child_return(void *arg)
{
	struct proc *p = curproc;

	TCB_SET(p, arg);
	child_return(p);
}

/*
 * Allocate and initialize a new process.
 */
void
process_new(struct proc *p, struct process *parent)
{
	struct process *pr;

	pr = pool_get(&process_pool, PR_WAITOK);
	pr->ps_mainproc = p;

	TAILQ_INIT(&pr->ps_threads);
	TAILQ_INSERT_TAIL(&pr->ps_threads, p, p_thr_link);
	pr->ps_pptr = parent;
	LIST_INIT(&pr->ps_children);
	pr->ps_refcnt = 1;

	/*
	 * Make a process structure for the new process.
	 * Start by zeroing the section of proc that is zero-initialized,
	 * then copy the section that is copied directly from the parent.
	 */
	bzero(&pr->ps_startzero,
	    (unsigned) ((caddr_t)&pr->ps_endzero - (caddr_t)&pr->ps_startzero));
	bcopy(&parent->ps_startcopy, &pr->ps_startcopy,
	    (unsigned) ((caddr_t)&pr->ps_endcopy - (caddr_t)&pr->ps_startcopy));

	/* post-copy fixups */
	pr->ps_cred = pool_get(&pcred_pool, PR_WAITOK);
	bcopy(parent->ps_cred, pr->ps_cred, sizeof(*pr->ps_cred));
	crhold(parent->ps_cred->pc_ucred);
	pr->ps_limit->p_refcnt++;

	pr->ps_flags = parent->ps_flags & (PS_SUGID | PS_SUGIDEXEC);
	if (parent->ps_session->s_ttyvp != NULL &&
	    parent->ps_flags & PS_CONTROLT)
		atomic_setbits_int(&pr->ps_flags, PS_CONTROLT);

	p->p_p = pr;
}

/* print the 'table full' message once per 10 seconds */
struct timeval fork_tfmrate = { 10, 0 };

int
fork1(struct proc *curp, int exitsig, int flags, void *stack, pid_t *tidptr,
    void (*func)(void *), void *arg, register_t *retval,
    struct proc **rnewprocp)
{
	struct process *curpr = curp->p_p;
	struct process *pr;
	struct proc *p;
	uid_t uid;
	struct vmspace *vm;
	int count;
	vaddr_t uaddr;
	int s;
	struct  ptrace_state *newptstat = NULL;
#if NSYSTRACE > 0
	void *newstrp = NULL;
#endif

	/* sanity check some flag combinations */
	if (flags & FORK_THREAD) {
		if (!rthreads_enabled)
			return (ENOTSUP);
		if ((flags & (FORK_SIGHAND | FORK_NOZOMBIE)) !=
		    (FORK_SIGHAND | FORK_NOZOMBIE))
			return (EINVAL);
	}
	if (flags & FORK_SIGHAND && (flags & FORK_SHAREVM) == 0)
		return (EINVAL);

	/*
	 * Although process entries are dynamically created, we still keep
	 * a global limit on the maximum number we will create. We reserve
	 * the last 5 processes to root. The variable nprocs is the current
	 * number of processes, maxproc is the limit.
	 */
	uid = curp->p_cred->p_ruid;
	if ((nprocs >= maxproc - 5 && uid != 0) || nprocs >= maxproc) {
		static struct timeval lasttfm;

		if (ratecheck(&lasttfm, &fork_tfmrate))
			tablefull("proc");
		return (EAGAIN);
	}
	nprocs++;

	/*
	 * Increment the count of procs running with this uid. Don't allow
	 * a nonprivileged user to exceed their current limit.
	 */
	count = chgproccnt(uid, 1);
	if (uid != 0 && count > curp->p_rlimit[RLIMIT_NPROC].rlim_cur) {
		(void)chgproccnt(uid, -1);
		nprocs--;
		return (EAGAIN);
	}

	uaddr = uvm_km_kmemalloc_pla(kernel_map, uvm.kernel_object, USPACE,
	    USPACE_ALIGN, UVM_KMF_ZERO,
	    no_constraint.ucr_low, no_constraint.ucr_high,
	    0, 0, USPACE/PAGE_SIZE);
	if (uaddr == 0) {
		chgproccnt(uid, -1);
		nprocs--;
		return (ENOMEM);
	}

	/*
	 * From now on, we're committed to the fork and cannot fail.
	 */

	/* Allocate new proc. */
	p = pool_get(&proc_pool, PR_WAITOK);

	p->p_stat = SIDL;			/* protect against others */
	p->p_exitsig = exitsig;
	p->p_flag = 0;

	if (flags & FORK_THREAD) {
		atomic_setbits_int(&p->p_flag, P_THREAD);
		p->p_p = pr = curpr;
		TAILQ_INSERT_TAIL(&pr->ps_threads, p, p_thr_link);
		pr->ps_refcnt++;
	} else {
		process_new(p, curpr);
		pr = p->p_p;
	}

	/*
	 * Make a proc table entry for the new process.
	 * Start by zeroing the section of proc that is zero-initialized,
	 * then copy the section that is copied directly from the parent.
	 */
	bzero(&p->p_startzero,
	    (unsigned) ((caddr_t)&p->p_endzero - (caddr_t)&p->p_startzero));
	bcopy(&curp->p_startcopy, &p->p_startcopy,
	    (unsigned) ((caddr_t)&p->p_endcopy - (caddr_t)&p->p_startcopy));

	/*
	 * Initialize the timeouts.
	 */
	timeout_set(&p->p_sleep_to, endtsleep, p);
	timeout_set(&p->p_realit_to, realitexpire, p);

	/*
	 * Duplicate sub-structures as needed.
	 * Increase reference counts on shared objects.
	 * The p_stats and p_sigacts substructs are set in vm_fork.
	 */
	if (curp->p_flag & P_PROFIL)
		startprofclock(p);
	if (flags & FORK_PTRACE)
		atomic_setbits_int(&p->p_flag, curp->p_flag & P_TRACED);

	/* bump references to the text vnode (for procfs) */
	p->p_textvp = curp->p_textvp;
	if (p->p_textvp)
		vref(p->p_textvp);

	if (flags & FORK_CLEANFILES)
		p->p_fd = fdinit(curp);
	else if (flags & FORK_SHAREFILES)
		p->p_fd = fdshare(curp);
	else
		p->p_fd = fdcopy(curp);

	if (flags & FORK_PPWAIT) {
		atomic_setbits_int(&pr->ps_flags, PS_PPWAIT);
		atomic_setbits_int(&curpr->ps_flags, PS_ISPWAIT);
	}
	if (flags & FORK_NOZOMBIE)
		atomic_setbits_int(&p->p_flag, P_NOZOMBIE);

#ifdef KTRACE
	/*
	 * Copy traceflag and tracefile if enabled.
	 * If not inherited, these were zeroed above.
	 */
	if (curp->p_traceflag & KTRFAC_INHERIT) {
		p->p_traceflag = curp->p_traceflag;
		if ((p->p_tracep = curp->p_tracep) != NULL)
			vref(p->p_tracep);
	}
#endif

	/*
	 * set priority of child to be that of parent
	 * XXX should move p_estcpu into the region of struct proc which gets
	 * copied.
	 */
	scheduler_fork_hook(curp, p);

	/*
	 * Create signal actions for the child process.
	 */
	if (flags & FORK_SIGHAND)
		p->p_sigacts = sigactsshare(curp);
	else
		p->p_sigacts = sigactsinit(curp);
	if (flags & FORK_THREAD)
		sigstkinit(&p->p_sigstk);

	/*
	 * If emulation has process fork hook, call it now.
	 */
	if (p->p_emul->e_proc_fork)
		(*p->p_emul->e_proc_fork)(p, curp);

	p->p_addr = (struct user *)uaddr;

	/*
	 * Finish creating the child process.  It will return through a
	 * different path later.
	 */
	uvm_fork(curp, p, ((flags & FORK_SHAREVM) ? TRUE : FALSE), stack,
	    0, func ? func : child_return, arg ? arg : p);

	timeout_set(&p->p_stats->p_virt_to, virttimer_trampoline, p);
	timeout_set(&p->p_stats->p_prof_to, proftimer_trampoline, p);

	vm = p->p_vmspace;

	if (flags & FORK_FORK) {
		forkstat.cntfork++;
		forkstat.sizfork += vm->vm_dsize + vm->vm_ssize;
	} else if (flags & FORK_VFORK) {
		forkstat.cntvfork++;
		forkstat.sizvfork += vm->vm_dsize + vm->vm_ssize;
#if 0
	} else if (flags & FORK_TFORK) {
		forkstat.cnttfork++;
#endif
	} else if (flags & FORK_RFORK) {
		forkstat.cntrfork++;
		forkstat.sizrfork += vm->vm_dsize + vm->vm_ssize;
	} else {
		forkstat.cntkthread++;
		forkstat.sizkthread += vm->vm_dsize + vm->vm_ssize;
	}

	if (p->p_flag & P_TRACED && flags & FORK_FORK)
		newptstat = malloc(sizeof(*newptstat), M_SUBPROC, M_WAITOK);
#if NSYSTRACE > 0
	if (ISSET(curp->p_flag, P_SYSTRACE))
		newstrp = systrace_getproc();
#endif

	/* Find an unused pid satisfying 1 <= lastpid <= PID_MAX */
	do {
		lastpid = 1 + (randompid ? arc4random() : lastpid) % PID_MAX;
	} while (pidtaken(lastpid));
	p->p_pid = lastpid;

	LIST_INSERT_HEAD(&allproc, p, p_list);
	LIST_INSERT_HEAD(PIDHASH(p->p_pid), p, p_hash);
	if ((flags & FORK_THREAD) == 0) {
		LIST_INSERT_AFTER(curpr, pr, ps_pglist);
		LIST_INSERT_HEAD(&curpr->ps_children, pr, ps_sibling);
	}

	if (p->p_flag & P_TRACED) {
		p->p_oppid = curp->p_pid;
		if ((flags & FORK_THREAD) == 0 &&
		    pr->ps_pptr != curpr->ps_pptr)
			proc_reparent(pr, curpr->ps_pptr);

		/*
		 * Set ptrace status.
		 */
		if (flags & FORK_FORK) {
			p->p_ptstat = newptstat;
			newptstat = NULL;
			curp->p_ptstat->pe_report_event = PTRACE_FORK;
			p->p_ptstat->pe_report_event = PTRACE_FORK;
			curp->p_ptstat->pe_other_pid = p->p_pid;
			p->p_ptstat->pe_other_pid = curp->p_pid;
		}
	}

#if NSYSTRACE > 0
	if (newstrp)
		systrace_fork(curp, p, newstrp);
#endif

	if (tidptr != NULL) {
		pid_t	pid = p->p_pid + THREAD_PID_OFFSET;

		if (copyout(&pid, tidptr, sizeof(pid)))
			psignal(curp, SIGSEGV);
	}

	/*
	 * Make child runnable, set start time, and add to run queue.
	 */
	SCHED_LOCK(s);
 	getmicrotime(&p->p_stats->p_start);
	p->p_acflag = AFORK;
	p->p_stat = SRUN;
	p->p_cpu = sched_choosecpu_fork(curp, flags);
	setrunqueue(p);
	SCHED_UNLOCK(s);

	if (newptstat)
		free(newptstat, M_SUBPROC);

	/*
	 * Notify any interested parties about the new process.
	 */
	if ((flags & FORK_THREAD) == 0)
		KNOTE(&curpr->ps_klist, NOTE_FORK | p->p_pid);

	/*
	 * Update stats now that we know the fork was successful.
	 */
	uvmexp.forks++;
	if (flags & FORK_PPWAIT)
		uvmexp.forks_ppwait++;
	if (flags & FORK_SHAREVM)
		uvmexp.forks_sharevm++;

	/*
	 * Pass a pointer to the new process to the caller.
	 */
	if (rnewprocp != NULL)
		*rnewprocp = p;

	/*
	 * Preserve synchronization semantics of vfork.  If waiting for
	 * child to exec or exit, set PS_PPWAIT on child and PS_ISPWAIT
	 * on ourselves, and sleep on our process for the latter flag
	 * to go away.
	 * XXX Need to stop other rthreads in the parent
	 */
	if (flags & FORK_PPWAIT)
		while (curpr->ps_flags & PS_ISPWAIT)
			tsleep(curpr, PWAIT, "ppwait", 0);

	/*
	 * If we're tracing the child, alert the parent too.
	 */
	if ((flags & FORK_PTRACE) && (curp->p_flag & P_TRACED))
		psignal(curp, SIGTRAP);

	/*
	 * Return child pid to parent process,
	 * marking us as parent via retval[1].
	 */
	if (retval != NULL) {
		retval[0] = p->p_pid +
		    (flags & FORK_THREAD ? THREAD_PID_OFFSET : 0);
		retval[1] = 0;
	}
	return (0);
}

/*
 * Checks for current use of a pid, either as a pid or pgid.
 */
int
pidtaken(pid_t pid)
{
	struct proc *p;

	if (pfind(pid) != NULL)
		return (1);
	if (pgfind(pid) != NULL)
		return (1);
	LIST_FOREACH(p, &zombproc, p_list) {
		if (p->p_pid == pid || (p->p_p->ps_pgrp && p->p_p->ps_pgrp->pg_id == pid))
			return (1);
	}
	return (0);
}

#if defined(MULTIPROCESSOR)
/*
 * XXX This is a slight hack to get newly-formed processes to
 * XXX acquire the kernel lock as soon as they run.
 */
void
proc_trampoline_mp(void)
{
	struct proc *p;

	p = curproc;

	SCHED_ASSERT_LOCKED();
	__mp_unlock(&sched_lock);
	spl0();
	SCHED_ASSERT_UNLOCKED();
	KASSERT(__mp_lock_held(&kernel_lock) == 0);

	KERNEL_LOCK();
}
#endif
