/*	$OpenBSD: cron.c,v 1.52 2015/02/09 22:35:08 deraadt Exp $	*/

/* Copyright 1988,1990,1993,1994 by Paul Vixie
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1997,2000 by Internet Software Consortium, Inc.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define	MAIN_PROGRAM

#include "cron.h"

enum timejump { negative, small, medium, large };

static	void	usage(void),
		run_reboot_jobs(cron_db *),
		find_jobs(time_t, cron_db *, int, int),
		set_time(int),
		cron_sleep(time_t),
		sigchld_handler(int),
		sighup_handler(int),
		sigchld_reaper(void),
		quit(int),
		parse_args(int c, char *v[]);

static	volatile sig_atomic_t	got_sighup, got_sigchld;
static	time_t			timeRunning, virtualTime, clockTime;
static	int			cronSock;
static	long			GMToff;
static	cron_db			database;
static	at_db			at_database;
static	double			batch_maxload = BATCH_MAXLOAD;

static void
usage(void)
{

	fprintf(stderr, "usage: %s [-n] [-l load_avg]\n", ProgramName);
	exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
	struct sigaction sact;
	int fd;

	ProgramName = argv[0];

	setlocale(LC_ALL, "");

	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IOLBF, 0);

	NoFork = 0;
	parse_args(argc, argv);

	bzero((char *)&sact, sizeof sact);
	sigemptyset(&sact.sa_mask);
	sact.sa_flags = 0;
#ifdef SA_RESTART
	sact.sa_flags |= SA_RESTART;
#endif
	sact.sa_handler = sigchld_handler;
	(void) sigaction(SIGCHLD, &sact, NULL);
	sact.sa_handler = sighup_handler;
	(void) sigaction(SIGHUP, &sact, NULL);
	sact.sa_handler = quit;
	(void) sigaction(SIGINT, &sact, NULL);
	(void) sigaction(SIGTERM, &sact, NULL);
	sact.sa_handler = SIG_IGN;
	(void) sigaction(SIGPIPE, &sact, NULL);

	acquire_daemonlock(0);
	set_cron_uid();
	set_cron_cwd();

	if (putenv("PATH="_PATH_DEFPATH) < 0) {
		log_it("CRON", getpid(), "DEATH", "can't malloc");
		exit(EXIT_FAILURE);
	}

	if (NoFork == 0) {
		switch (fork()) {
		case -1:
			log_it("CRON",getpid(),"DEATH","can't fork");
			exit(EXIT_FAILURE);
			break;
		case 0:
			/* child process */
			(void) setsid();
			if ((fd = open(_PATH_DEVNULL, O_RDWR, 0)) >= 0) {
				(void) dup2(fd, STDIN_FILENO);
				(void) dup2(fd, STDOUT_FILENO);
				(void) dup2(fd, STDERR_FILENO);
				if (fd != STDERR_FILENO)
					(void) close(fd);
			}
			log_it("CRON",getpid(),"STARTUP",CRON_VERSION);
			break;
		default:
			/* parent process should just die */
			_exit(EXIT_SUCCESS);
		}
	}

	acquire_daemonlock(0);
	cronSock = open_socket();
	database.head = NULL;
	database.tail = NULL;
	database.mtime = 0;
	load_database(&database);
	at_database.head = NULL;
	at_database.tail = NULL;
	at_database.mtime = 0;
	scan_atjobs(&at_database, NULL);
	set_time(TRUE);
	run_reboot_jobs(&database);
	timeRunning = virtualTime = clockTime;

	/*
	 * Too many clocks, not enough time (Al. Einstein)
	 * These clocks are in minutes since the epoch, adjusted for timezone.
	 * virtualTime: is the time it *would* be if we woke up
	 * promptly and nobody ever changed the clock. It is
	 * monotonically increasing... unless a timejump happens.
	 * At the top of the loop, all jobs for 'virtualTime' have run.
	 * timeRunning: is the time we last awakened.
	 * clockTime: is the time when set_time was last called.
	 */
	while (TRUE) {
		int timeDiff;
		enum timejump wakeupKind;

		/* ... wait for the time (in minutes) to change ... */
		do {
			cron_sleep(timeRunning + 1);
			set_time(FALSE);
		} while (clockTime == timeRunning);
		timeRunning = clockTime;

		/*
		 * Calculate how the current time differs from our virtual
		 * clock.  Classify the change into one of 4 cases.
		 */
		timeDiff = timeRunning - virtualTime;

		/* shortcut for the most common case */
		if (timeDiff == 1) {
			virtualTime = timeRunning;
			find_jobs(virtualTime, &database, TRUE, TRUE);
		} else {
			if (timeDiff > (3*MINUTE_COUNT) ||
			    timeDiff < -(3*MINUTE_COUNT))
				wakeupKind = large;
			else if (timeDiff > 5)
				wakeupKind = medium;
			else if (timeDiff > 0)
				wakeupKind = small;
			else
				wakeupKind = negative;

			switch (wakeupKind) {
			case small:
				/*
				 * case 1: timeDiff is a small positive number
				 * (wokeup late) run jobs for each virtual
				 * minute until caught up.
				 */
				do {
					if (job_runqueue())
						sleep(10);
					virtualTime++;
					find_jobs(virtualTime, &database,
					    TRUE, TRUE);
				} while (virtualTime < timeRunning);
				break;

			case medium:
				/*
				 * case 2: timeDiff is a medium-sized positive
				 * number, for example because we went to DST
				 * run wildcard jobs once, then run any
				 * fixed-time jobs that would otherwise be
				 * skipped if we use up our minute (possible,
				 * if there are a lot of jobs to run) go
				 * around the loop again so that wildcard jobs
				 * have a chance to run, and we do our
				 * housekeeping.
				 */
				/* run wildcard jobs for current minute */
				find_jobs(timeRunning, &database, TRUE, FALSE);

				/* run fixed-time jobs for each minute missed */
				do {
					if (job_runqueue())
						sleep(10);
					virtualTime++;
					find_jobs(virtualTime, &database,
					    FALSE, TRUE);
					set_time(FALSE);
				} while (virtualTime< timeRunning &&
				    clockTime == timeRunning);
				break;

			case negative:
				/*
				 * case 3: timeDiff is a small or medium-sized
				 * negative num, eg. because of DST ending.
				 * Just run the wildcard jobs. The fixed-time
				 * jobs probably have already run, and should
				 * not be repeated.  Virtual time does not
				 * change until we are caught up.
				 */
				find_jobs(timeRunning, &database, TRUE, FALSE);
				break;
			default:
				/*
				 * other: time has changed a *lot*,
				 * jump virtual time, and run everything
				 */
				virtualTime = timeRunning;
				find_jobs(timeRunning, &database, TRUE, TRUE);
			}
		}

		/* Jobs to be run (if any) are loaded; clear the queue. */
		job_runqueue();

		/* Run any jobs in the at queue. */
		atrun(&at_database, batch_maxload,
		    timeRunning * SECONDS_PER_MINUTE - GMToff);

		/* Check to see if we received a signal while running jobs. */
		if (got_sighup) {
			got_sighup = 0;
			log_close();
		}
		if (got_sigchld) {
			got_sigchld = 0;
			sigchld_reaper();
		}
		load_database(&database);
		scan_atjobs(&at_database, NULL);
	}
}

static void
run_reboot_jobs(cron_db *db)
{
	user *u;
	entry *e;

	for (u = db->head; u != NULL; u = u->next) {
		for (e = u->crontab; e != NULL; e = e->next) {
			if (e->flags & WHEN_REBOOT)
				job_add(e, u);
		}
	}
	(void) job_runqueue();
}

static void
find_jobs(time_t vtime, cron_db *db, int doWild, int doNonWild)
{
	time_t virtualSecond  = vtime * SECONDS_PER_MINUTE;
	struct tm *tm = gmtime(&virtualSecond);
	int minute, hour, dom, month, dow;
	user *u;
	entry *e;

	/* make 0-based values out of these so we can use them as indices
	 */
	minute = tm->tm_min -FIRST_MINUTE;
	hour = tm->tm_hour -FIRST_HOUR;
	dom = tm->tm_mday -FIRST_DOM;
	month = tm->tm_mon +1 /* 0..11 -> 1..12 */ -FIRST_MONTH;
	dow = tm->tm_wday -FIRST_DOW;

	/* the dom/dow situation is odd.  '* * 1,15 * Sun' will run on the
	 * first and fifteenth AND every Sunday;  '* * * * Sun' will run *only*
	 * on Sundays;  '* * 1,15 * *' will run *only* the 1st and 15th.  this
	 * is why we keep 'e->dow_star' and 'e->dom_star'.  yes, it's bizarre.
	 * like many bizarre things, it's the standard.
	 */
	for (u = db->head; u != NULL; u = u->next) {
		for (e = u->crontab; e != NULL; e = e->next) {
			if (bit_test(e->minute, minute) &&
			    bit_test(e->hour, hour) &&
			    bit_test(e->month, month) &&
			    ( ((e->flags & DOM_STAR) || (e->flags & DOW_STAR))
			      ? (bit_test(e->dow,dow) && bit_test(e->dom,dom))
			      : (bit_test(e->dow,dow) || bit_test(e->dom,dom))
			    )
			   ) {
				if ((doNonWild &&
				    !(e->flags & (MIN_STAR|HR_STAR))) ||
				    (doWild && (e->flags & (MIN_STAR|HR_STAR))))
					job_add(e, u);
			}
		}
	}
}

/*
 * Set StartTime and clockTime to the current time.
 * These are used for computing what time it really is right now.
 * Note that clockTime is a unix wallclock time converted to minutes.
 */
static void
set_time(int initialize)
{
	struct tm tm;
	static int isdst;

	StartTime = time(NULL);

	/* We adjust the time to GMT so we can catch DST changes. */
	tm = *localtime(&StartTime);
	if (initialize || tm.tm_isdst != isdst) {
		isdst = tm.tm_isdst;
		GMToff = get_gmtoff(&StartTime, &tm);
	}
	clockTime = (StartTime + GMToff) / (time_t)SECONDS_PER_MINUTE;
}

/*
 * Try to just hit the next minute.
 */
static void
cron_sleep(time_t target)
{
	int fd, nfds;
	unsigned char poke;
	struct timeval t1, t2, tv;
	struct sockaddr_un s_un;
	socklen_t sunlen;
	static struct pollfd pfd[1];

	gettimeofday(&t1, NULL);
	t1.tv_sec += GMToff;
	tv.tv_sec = (target * SECONDS_PER_MINUTE - t1.tv_sec) + 1;
	tv.tv_usec = 0;

	pfd[0].fd = cronSock;
	pfd[0].events = POLLIN;

	while (timerisset(&tv) && tv.tv_sec < 65) {
		poke = RELOAD_CRON | RELOAD_AT;

		/* Sleep until we time out, get a poke, or get a signal. */
		nfds = poll(pfd, 1, tv.tv_sec * 1000 + tv.tv_usec / 1000);
		if (nfds == 0)
			break;		/* timer expired */
		if (nfds == -1 && errno != EINTR)
			break;		/* an error occurred */
		if (nfds > 0) {
			sunlen = sizeof(s_un);
			fd = accept(cronSock, (struct sockaddr *)&s_un, &sunlen);
			if (fd >= 0 && fcntl(fd, F_SETFL, O_NONBLOCK) == 0) {
				(void) read(fd, &poke, 1);
				close(fd);
				if (poke & RELOAD_CRON) {
					database.mtime = 0;
					load_database(&database);
				}
				if (poke & RELOAD_AT) {
					/*
					 * We run any pending at jobs right
					 * away so that "at now" really runs
					 * jobs immediately.
					 */
					gettimeofday(&t2, NULL);
					at_database.mtime = 0;
					if (scan_atjobs(&at_database, &t2))
						atrun(&at_database,
						    batch_maxload, t2.tv_sec);
				}
			}
		} else {
			/* Interrupted by a signal. */
			if (got_sighup) {
				got_sighup = 0;
				log_close();
			}
			if (got_sigchld) {
				got_sigchld = 0;
				sigchld_reaper();
			}
		}

		/* Adjust tv and continue where we left off.  */
		gettimeofday(&t2, NULL);
		t2.tv_sec += GMToff;
		timersub(&t2, &t1, &t1);
		timersub(&tv, &t1, &tv);
		memcpy(&t1, &t2, sizeof(t1));
		if (tv.tv_sec < 0)
			tv.tv_sec = 0;
		if (tv.tv_usec < 0)
			tv.tv_usec = 0;
	}
}

static void
sighup_handler(int x)
{
	got_sighup = 1;
}

static void
sigchld_handler(int x)
{
	got_sigchld = 1;
}

static void
quit(int x)
{
	(void) unlink(_PATH_CRON_PID);
	_exit(0);
}

static void
sigchld_reaper(void)
{
	int waiter;
	pid_t pid;

	do {
		pid = waitpid(-1, &waiter, WNOHANG);
		switch (pid) {
		case -1:
			if (errno == EINTR)
				continue;
			break;
		case 0:
			break;
		default:
			break;
		}
	} while (pid > 0);
}

static void
parse_args(int argc, char *argv[])
{
	int argch;
	char *ep;

	while (-1 != (argch = getopt(argc, argv, "l:n"))) {
		switch (argch) {
		case 'l':
			errno = 0;
			batch_maxload = strtod(optarg, &ep);
			if (*ep != '\0' || ep == optarg || errno == ERANGE ||
			    batch_maxload < 0) {
				fprintf(stderr, "Illegal load average: %s\n",
				    optarg);
				usage();
			}
			break;
		case 'n':
			NoFork = 1;
			break;
		default:
			usage();
		}
	}
}
