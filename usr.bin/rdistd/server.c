/*	$OpenBSD: server.c,v 1.31 2014/07/05 07:39:18 guenther Exp $	*/

#include <dirent.h>

static int setownership(char *, int, uid_t, gid_t, int);
setownership(char *file, int fd, uid_t uid, gid_t gid, int link)
		status = lchown(file, uid, gid);
		status = fchown(fd, uid, gid);

		status = chown(file, uid, gid);
		if (uid == (uid_t)-1)
		status = fchmodat(AT_FDCWD, file, mode, AT_SYMLINK_NOFOLLOW);
	uid_t uid;
	gid_t gid;
	gid_t primegid = (gid_t)-2;
			uid = (uid_t) atoi(owner + 1);
			gid = (gid_t)atoi(group + 1);
	gid = (gid_t)-1;
		gid = (gid_t)-1;
	static struct dirent *dp;
		if (dp->d_name[0] == '.' && (dp->d_name[1] == '\0' ||
		    (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))
	struct dirent *dp;
		if (dp->d_name[0] == '.' && (dp->d_name[1] == '\0' ||
		    (dp->d_name[1] == '.' && dp->d_name[2] == '\0')))