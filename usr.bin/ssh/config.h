/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */
/*

acconfig.h - template used by autoheader to create config.h.in
config.h.in - used by autoconf to create config.h
config.h - created by autoconf; contains defines generated by autoconf

Copyright (c) 1995 Tatu Ylonen <ylo@cs.hut.fi>

*/

#define RCSID(msg) \
static /**/const char *const rcsid[] = { (char *)rcsid, "\100(#)" msg }


/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef gid_t */

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#define HAVE_SYS_WAIT_H 1

/* Define as __inline if that's what the C compiler calls it.  */
/* #undef inline */

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef mode_t */

/* Define if your C compiler doesn't accept -c and -o together.  */
/* #undef NO_MINUS_C_MINUS_O */

/* Define if your Fortran 77 compiler doesn't accept -c and -o together. */
/* #undef F77_NO_MINUS_C_MINUS_O */

/* Define to `long' if <sys/types.h> doesn't define.  */
/* #undef off_t */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define to the type of arg1 for select(). */
/* #undef SELECT_TYPE_ARG1 */

/* Define to the type of args 2, 3 and 4 for select(). */
/* #undef SELECT_TYPE_ARG234 */

/* Define to the type of arg5 for select(). */
/* #undef SELECT_TYPE_ARG5 */

/* Define to `unsigned' if <sys/types.h> doesn't define.  */
/* #undef size_t */

/* Define if the `S_IS*' macros in <sys/stat.h> do not work properly.  */
/* #undef STAT_MACROS_BROKEN */

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define to `int' if <sys/types.h> doesn't define.  */
/* #undef uid_t */

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Define if you have SYSV-style /dev/ptmx and /dev/pts/. */
/* #undef HAVE_DEV_PTMX */

/* Define if you have /dev/pts and /dev/ptc devices (as in AIX). */
/* #undef HAVE_DEV_PTS_AND_PTC */

/* Define if you have shadow passwords in /etc/security/passwd (AIX style). */
/* #undef HAVE_ETC_SECURITY_PASSWD */

/* Define if you have shadow passwords in /etc/security/passwd.adjunct
   (SunOS style). */
/* #undef HAVE_ETC_SECURITY_PASSWD_ADJUNCT */

  
/* Define if you have OSF1 C2 security installed on the system */
/* #undef HAVE_OSF1_C2_SECURITY */

/* Define if you have shadow passwords in /etc/shadow (Solaris style). */
/* #undef HAVE_ETC_SHADOW */

/* Define if you have system login defaults in /etc/default/login. */
/* #undef HAVE_ETC_DEFAULT_LOGIN */

/* Define if utmp structure has host field. */
#define HAVE_HOST_IN_UTMP 1

/* Define if utmp structure has addr field. */
/* #undef HAVE_ADDR_IN_UTMP */

/* Define if utmp structure has id field. */
/* #undef HAVE_ID_IN_UTMP */

/* Define if utmp structure has name field. */
#define HAVE_NAME_IN_UTMP 1

/* Define if utmp structure has pid field. */
/* #undef HAVE_PID_IN_UTMP */

/* Define if utmpx structure has ut_session. */
/* #undef HAVE_UT_SESSION */

/* Define if utmpx structure has ut_syslen. */
/* #undef HAVE_UT_SYSLEN */

/* Define if /var/adm/lastlog or whatever it is called is a directory
   (e.g. SGI IRIX). */
/* #undef LASTLOG_IS_DIR */

/* Define to use RSAREF. */
/* #undef RSAREF */

/* Define this to be the path of the xauth program. */
#define XAUTH_PATH "/usr/X11R6/bin/xauth"

/* This is defined if we found a lastlog file.  The presence of lastlog.h
   alone is not a sufficient indicator (at least newer BSD systems have
   lastlog but no lastlog.h. */
#define HAVE_LASTLOG 1

/* Define this if libutil.a contains BSD 4.4 compatible login(), logout(),
   and logwtmp() calls. */
#define HAVE_LIBUTIL_LOGIN 1

/* Location of system mail spool directory. */
#define MAIL_SPOOL_DIRECTORY "/var/mail"

/* Define this if O_NONBLOCK does not work on your system (e.g., Ultrix). */
/* #undef O_NONBLOCK_BROKEN */

/* Define this to include libwrap (tcp_wrappers) support. */
/* #undef LIBWRAP */

/* This is defined to pw_encrypt on Linux when using John Faugh's shadow 
   password implementation. */
/* #undef crypt */

/* This is defined on 386BSD to preted we are on FreeBSD. */
/* #undef __FreeBSD__ */

/* If defines, this overrides "tty" as the terminal group. */
/* #undef TTY_GROUP */

/* Define this if you want to support Security Dynammics SecurID
   cards. */
/* #undef HAVE_SECURID */

/* Define this if you are using HPSUX.  HPUX uses non-standard shared
   memory communication for X, which seems to be enabled by the display name
   matching that of the local host.  This circumvents it by using the IP
   address instead of the host name in DISPLAY. */
/* #undef HPSUX_NONSTANDARD_X11_KLUDGE */

/* Define this if inet_network should be used instead of inet_addr.  This is
   the case on DGUX 5.4. */
/* #undef BROKEN_INET_ADDR */

/* Define this if your system does not like sizeof(struct sockaddr_un) as the
   size argument in bind and connect calls for unix domain sockets. */
/* #undef USE_STRLEN_FOR_AF_UNIX */

/* Define this to use pipes instead of socketpairs for communicating with the
   client program.  Socketpairs do not seem to work on all systems. */
#define USE_PIPES 1

/* Define this if speed_t is defined in stdtypes.h or otherwise gets included
   into ttymodes.c from system headers. */
/* #undef SPEED_T_IN_STDTYPES_H */

/* Define this if compiling with SOCKS (the firewall traversal library).
   Also, you must define connect, getsockname, bind, accept, listen, and
   select to their R-versions. */
/* #undef SOCKS */
/* #undef connect */
/* #undef getsockname */
/* #undef bind */
/* #undef accept */
/* #undef listen */
/* #undef select */

/* Define these if on SCO Unix. */
/* #undef HAVE_SCO_ETC_SHADOW */
/* #undef SCO */

/* Define this if you want to compile in Kerberos V4 support.
   This can be done at configure time with the --with-krb4 argument. */
/* #undef KRB4 */

/* Define this if you want to compile in AFS support.
   This can be done at configure time with the --with-afs argument. */
/* #undef AFS */

/* The number of bytes in a int.  */
#define SIZEOF_INT 4

/* The number of bytes in a long.  */
#define SIZEOF_LONG 4

/* The number of bytes in a short.  */
#define SIZEOF_SHORT 2

/* Define if you have the _getpty function.  */
/* #undef HAVE__GETPTY */

/* Define if you have the clock function.  */
#define HAVE_CLOCK 1

/* Define if you have the fchmod function.  */
#define HAVE_FCHMOD 1

/* Define if you have the getdtablesize function.  */
#define HAVE_GETDTABLESIZE 1

/* Define if you have the getrusage function.  */
#define HAVE_GETRUSAGE 1

/* Define if you have the gettimeofday function.  */
#define HAVE_GETTIMEOFDAY 1

/* Define if you have the initgroups function.  */
#define HAVE_INITGROUPS 1

/* Define if you have the innetgr function.  */
#define HAVE_INNETGR 1

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the openpty function.  */
/* #undef HAVE_OPENPTY */

/* Define if you have the popen function.  */
#define HAVE_POPEN 1

/* Define if you have the putenv function.  */
#define HAVE_PUTENV 1

/* Define if you have the random function.  */
#define HAVE_RANDOM 1

/* Define if you have the remove function.  */
#define HAVE_REMOVE 1

/* Define if you have the seteuid function.  */
#define HAVE_SETEUID 1

/* Define if you have the setlogin function.  */
#define HAVE_SETLOGIN 1

/* Define if you have the setluid function.  */
/* #undef HAVE_SETLUID */

/* Define if you have the setrlimit function.  */
#define HAVE_SETRLIMIT 1

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the times function.  */
#define HAVE_TIMES 1

/* Define if you have the ulimit function.  */
/* #undef HAVE_ULIMIT */

/* Define if you have the umask function.  */
#define HAVE_UMASK 1

/* Define if you have the vhangup function.  */
/* #undef HAVE_VHANGUP */

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <dirent.h> header file.  */
#define HAVE_DIRENT_H 1

/* Define if you have the <krb.h> header file.  */
/* #undef HAVE_KRB_H */

/* Define if you have the <lastlog.h> header file.  */
/* #undef HAVE_LASTLOG_H */

/* Define if you have the <ndir.h> header file.  */
/* #undef HAVE_NDIR_H */

/* Define if you have the <netinet/in_systm.h> header file.  */
#define HAVE_NETINET_IN_SYSTM_H 1

/* Define if you have the <paths.h> header file.  */
#define HAVE_PATHS_H 1

/* Define if you have the <rusage.h> header file.  */
/* #undef HAVE_RUSAGE_H */

/* Define if you have the <sgtty.h> header file.  */
#define HAVE_SGTTY_H 1

/* Define if you have the <sys/dir.h> header file.  */
/* #undef HAVE_SYS_DIR_H */

/* Define if you have the <sys/filio.h> header file.  */
#define HAVE_SYS_FILIO_H 1

/* Define if you have the <sys/ioctl.h> header file.  */
#define HAVE_SYS_IOCTL_H 1

/* Define if you have the <sys/ndir.h> header file.  */
/* #undef HAVE_SYS_NDIR_H */

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <termios.h> header file.  */
#define HAVE_TERMIOS_H 1

/* Define if you have the <ulimit.h> header file.  */
/* #undef HAVE_ULIMIT_H */

/* Define if you have the <unistd.h> header file.  */
#define HAVE_UNISTD_H 1

/* Define if you have the <usersec.h> header file.  */
/* #undef HAVE_USERSEC_H */

/* Define if you have the <utime.h> header file.  */
#define HAVE_UTIME_H 1

/* Define if you have the <utmp.h> header file.  */
#define HAVE_UTMP_H 1

/* Define if you have the <utmpx.h> header file.  */
/* #undef HAVE_UTMPX_H */

/* Define if you have the crypt library (-lcrypt).  */
/* #undef HAVE_LIBCRYPT */

/* Define if you have the des library (-ldes).  */
#define HAVE_LIBDES 1

/* Define if you have the gen library (-lgen).  */
/* #undef HAVE_LIBGEN */

/* Define if you have the krb library (-lkrb).  */
#define HAVE_LIBKRB 1

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the s library (-ls).  */
/* #undef HAVE_LIBS */

/* Define if you have the security library (-lsecurity).  */
/* #undef HAVE_LIBSECURITY */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

/* Define if you have the sun library (-lsun).  */
/* #undef HAVE_LIBSUN */

/* Define if you have the z library (-lz).  */
#define HAVE_LIBZ 1
