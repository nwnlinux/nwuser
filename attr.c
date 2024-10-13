/* 
 * Hack to enable per user settings for NWN. 
 * 
 * Copyright 2004 - David Holland zzqzzq_zzq@hotmail.com
 * Copyright 2008 - David Holland david.w.holland@gmail.com
 *
 * Do what you want with the code, just maintain
 * the copyright, and give credit
 * 
 * No, none of this code is particularly efficient, but it 
 * arguably doesn't have to be, as at worst, it's specifically 
 * targetted towards NWN, and hopefully you don't have more than 
 * a few hundred save games.. 
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <limits.h>
#include <dlfcn.h>
#include <pwd.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>

#ifdef XATTR
#include <attr/xattr.h>
#endif

#include "code.h"

#ifdef XATTR
ssize_t getxattr (const char *path, const char *name, void *value, size_t size) { 
	errno = ENOTSUP; 
	return(-1); 
}

ssize_t lgetxattr (const char *path, const char *name, void *value, size_t size) { 
	errno = ENOTSUP; 
	return(-1); 
}

ssize_t fgetxattr (int filedes, const char *name, void *value, size_t size) { 
	errno = ENOTSUP; 
	return(-1); 
}

int setxattr (const char *path, const char *name, const void *value, size_t size, int flags) { 
	errno = ENOTSUP; 
	return(-1); 
}

int lsetxattr (const char *path, const char *name, const void *value, size_t size, int flags) { 
	errno = ENOTSUP; 
	return(-1); 
}

int fsetxattr (int filedes, const char *name, const void *value, size_t size, int flags) { 
	errno = ENOTSUP; 
	return(-1); 
}

ssize_t listxattr (const char *path, char *list, size_t size) { 
	errno = ENOTSUP; 
	return(-1); 
}

ssize_t llistxattr (const char *path, char *list, size_t size) { 
	errno = ENOTSUP; 
	return(-1); 
}

ssize_t flistxattr (int filedes, char *list, size_t size) { 
	errno = ENOTSUP; 
	return(-1); 
}

int removexattr (const char *path, const char *name) {
	errno = ENOTSUP; 
	return(-1); 
} 

int lremovexattr (const char *path, const char *name) {
	errno = ENOTSUP; 
	return(-1); 
} 

int fremovexattr (int filedes, const char *name) {
	errno = ENOTSUP; 
	return(-1); 
} 

#endif
