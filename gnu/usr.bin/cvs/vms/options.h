/*
 * Copyright (c) 1992, Brian Berliner and Jeff Polk
 * Copyright (c) 1989-1992, Brian Berliner
 * 
 * You may distribute under the terms of the GNU General Public License as
 * specified in the README file that comes with the CVS 1.4 kit.
 * 
 * This file holds (most of) the configuration tweaks that can be made to
 * customize CVS for your site.  CVS comes configured for a typical SunOS 4.x
 * environment.  The comments for each configurable item are intended to be
 * self-explanatory.  All #defines are tested first to see if an over-riding
 * option was specified on the "make" command line.
 * 
 * If special libraries are needed, you will have to edit the Makefile.in file
 * or the configure script directly.  Sorry.
 */

/*
 * For portability and heterogeneity reasons, CVS is shipped by default using
 * my own text-file version of the ndbm database library in the src/myndbm.c
 * file.  If you want better performance and are not concerned about
 * heterogeneous hosts accessing your modules file, turn this option off.
 */
#ifndef MY_NDBM
#define	MY_NDBM
#endif

/*
 * The "patch" program to run when using the CVS server and accepting
 * patches across the network.  Specify a full pathname if your site
 * wants to use a particular patch.
 */
#ifndef PATCH_PROGRAM
#define PATCH_PROGRAM	"patch"
#endif

/* Directory used for storing temporary files, if not overridden by
   environment variables or the -T global option.  There should be little
   need to change this (-T is a better mechanism if you need to use a
   different directory for temporary files).  */
#ifndef TMPDIR_DFLT
#define	TMPDIR_DFLT	"sys$scratch"
#endif

/*
 * The default editor to use, if one does not specify the "-e" option to cvs,
 * or does not have an EDITOR environment variable.  I set this to just "vi",
 * and use the shell to find where "vi" actually is.  This allows sites with
 * /usr/bin/vi or /usr/ucb/vi to work equally well (assuming that your PATH
 * is reasonable).
 */
#ifndef EDITOR_DFLT
#define	EDITOR_DFLT	""
#endif

/*
 * The default umask to use when creating or otherwise setting file or
 * directory permissions in the repository.  Must be a value in the
 * range of 0 through 0777.  For example, a value of 002 allows group
 * rwx access and world rx access; a value of 007 allows group rwx
 * access but no world access.  This value is overridden by the value
 * of the CVSUMASK environment variable, which is interpreted as an
 * octal number.
 */
#ifndef UMASK_DFLT
#define	UMASK_DFLT	002
#endif

/*
 * The cvs admin command is restricted to the members of the group
 * CVS_ADMIN_GROUP.  If this group does not exist, all users are
 * allowed to run cvs admin.  To disable the cvs admin for all users,
 * create an empty group CVS_ADMIN_GROUP.  To disable access control for
 * cvs admin, comment out the define below.
 */
#ifndef CVS_ADMIN_GROUP
/* #define CVS_ADMIN_GROUP "cvsadmin" */
#endif

/*
 * The Repository file holds the path to the directory within the
 * source repository that contains the RCS ,v files for each CVS
 * working directory.  This path is either a full-path or a path
 * relative to CVSROOT.
 * 
 * The big advantage that I can see to having a relative path is that
 * one can change the physical location of the master source
 * repository, change the contents of CVS/Root files in your
 * checked-out code, and CVS will work without problems.
 *
 * Therefore, RELATIVE_REPOS is now the default.  In the future, this
 * is likely to disappear entirely as a compile-time (or other) option,
 * so if you have other software which relies on absolute pathnames,
 * update them.
 */
#define RELATIVE_REPOS 1

/*
 * When committing or importing files, you must enter a log message.
 * Normally, you can do this either via the -m flag on the command line or an
 * editor will be started for you.  If you like to use logging templates (the
 * rcsinfo file within the $CVSROOT/CVSROOT directory), you might want to
 * force people to use the editor even if they specify a message with -m.
 * Enabling FORCE_USE_EDITOR will cause the -m message to be appended to the
 * temp file when the editor is started.
 */
#ifndef FORCE_USE_EDITOR
/* #define 	FORCE_USE_EDITOR */
#endif

/*
 * When locking the repository, some sites like to remove locks and assume
 * the program that created them went away if the lock has existed for a long
 * time.  This used to be the default for previous versions of CVS.  CVS now
 * attempts to be much more robust, so lock files should not be left around
 * by mistake. The new behaviour will never remove old locks (they must now
 * be removed by hand).  Enabling CVS_FUDGELOCKS will cause CVS to remove
 * locks that are older than CVSLCKAGE seconds.
 * Use of this option is NOT recommended.
 */
#ifndef CVS_FUDGELOCKS
/* #define CVS_FUDGELOCKS */
#endif

/* There is some pretty unixy code in src/commit.c which tries to
   prevent people from commiting changes as "root" (which would prevent
   CVS from making a log entry with the actual user).  On VMS, I suppose
   one could say that SYSTEM is equivalent, but I would think that it
   actually is not necessary; at least at the VMS sites I've worked at
   people just used their own accounts (turning privileges on and off
   as desired).  */

#ifndef CVS_BADROOT
/* #define	CVS_BADROOT */
#endif

/*
 * Yes, we can do the authenticated client.
 */
#define AUTH_CLIENT_SUPPORT 1

/*
 * define this to enable the SETXID support.  Probably has no effect on VMS.
 */
#ifndef SETXID_SUPPORT
/* #define SETXID_SUPPORT */
#endif

/*
 * If you are working with a large remote repository and a 'cvs checkout' is
 * swamping your network and memory, define these to enable flow control.
 * You will end up with even less guarantees of a consistant checkout,
 * but that may be better than no checkout at all.  The master server process
 * will monitor how far it is getting behind, if it reaches the high water
 * mark, it will signal the child process to stop generating data when
 * convenient (ie: no locks are held, currently at the beginning of a 
 * new directory).  Once the buffer has drained sufficiently to reach the
 * low water mark, it will be signalled to start again.
 * -- EXPERIMENTAL! --  A better solution may be in the works.
 * You may override the default hi/low watermarks here too.
 */
#ifndef SERVER_FLOWCONTROL
/* #define SERVER_FLOWCONTROL */
/* #define SERVER_HI_WATER (2 * 1024 * 1024) */
/* #define SERVER_LO_WATER (1 * 1024 * 1024) */
#endif

/* End of CVS configuration section */

/*
 * Externs that are included in libc, but are used frequently enough to
 * warrant defining here.
 */
#ifndef STDC_HEADERS
extern void exit ();
#endif

#define NO_SOCKET_TO_FD 1
#include "vms.h"
