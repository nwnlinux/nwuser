/* 
 * Hack to enable per user settings for NWN.
 *
 * Copyright 2004 - David Holland zzqzzq_zzq@hotmail.com
 * Copyright 2008 - David Holland david.w.holland@gmail.com
 *
 * Do what you want with the code, just maintain
 * the copyright, and give credit
 * 
 * file functions
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

#include "code.h"

int __xstat(int ver, const char *file_name, struct stat *buf) {

	char	path[PATH_MAX]; 
	char	short_path[PATH_MAX]; 

	if( __nwu_xstat == NULL ) { 
		__nwu_initialize(); 
	} 

	if( __nwu_possible((char *)file_name, short_path) ) { 
		if( __nwu_exists_local((char *)short_path) ) { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
			return( __nwu_xstat(ver, path, buf) ); 
		} else { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
			return( __nwu_xstat(ver, path, buf) ); 
		}
	} 
	return(__nwu_xstat(ver, file_name, buf)); 
}

int __xstat64(int ver, const char *file_name, struct stat64 *buf) {

	char	path[PATH_MAX]; 
	char	short_path[PATH_MAX];

	if( __nwu_xstat64 == NULL ) { 
		__nwu_initialize(); 
	} 

	if( __nwu_possible((char *)file_name, short_path) ) { 
		if( __nwu_exists_local((char *)short_path) ) { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
			return( __nwu_xstat64(ver, path, buf) ); 
		} else { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
			return( __nwu_xstat64(ver, path, buf) ); 
		}
	} 
	return(__nwu_xstat64(ver, file_name, buf)); 
}

int __lxstat(int ver, const char *file_name, struct stat *buf) {

	char	path[PATH_MAX]; 
	char	short_path[PATH_MAX];

	if( __nwu_lxstat == NULL ) { 
		__nwu_initialize(); 
	} 

	if( __nwu_possible((char *)file_name, short_path) ) { 
		if( __nwu_exists_local((char *)short_path) ) { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
			return( __nwu_lxstat(ver, path, buf) ); 
		} else { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
			return( __nwu_lxstat(ver, path, buf) ); 
		}
	} 
	return(__nwu_lxstat(ver, file_name, buf)); 
}

int __lxstat64(int ver, const char *file_name, struct stat64 *buf) {

	char	path[PATH_MAX]; 
	char	short_path[PATH_MAX]; 

	if( __nwu_lxstat64 == NULL ) { 
		__nwu_initialize(); 
	}

	if( __nwu_possible((char *)file_name, short_path) ) { 
		if( __nwu_exists_local((char *)short_path) ) { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
			return( __nwu_lxstat64(ver, path, buf) ); 
		} else { 
			snprintf(path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
			return( __nwu_lxstat64(ver, path, buf) ); 
		}
	} 
	return(__nwu_lxstat64(ver, file_name, buf)); 
}

int open64(const char *path, int flags, ... ) { 
	mode_t		mode; 
	va_list		arg_list; 
	int		write_mode = 0; 
	char		final_path[PATH_MAX]; 
	char		short_path[PATH_MAX];

	if( __nwu_open64 == NULL ) { 
		__nwu_initialize(); 
	}

	__nwu_log( NWU_LOG_OPEN, "OPEN64: %s - %02x\n", path, flags); 
	if( flags & O_CREAT ) {
		va_start(arg_list, flags); 
		mode = va_arg(arg_list, mode_t); 
		va_end(arg_list);
	} else { 
		mode = 0; 
	}

	if( ! __nwu_possible((char *) path, short_path) ) { 
		return( __nwu_open64( path, flags, mode ) ); 
	}

	if( (flags & O_WRONLY) || (flags & O_RDWR) ) { 
		write_mode = 1; 
	} 
	if( write_mode && __nwu_exists_master((char *)short_path ) && ! __nwu_exists_local((char *)short_path) ) { 
		__nwu_copy((char *)short_path); 
	} 
	if( write_mode || __nwu_exists_local((char *)short_path) ) { 
		snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
		return( __nwu_open64( final_path, flags, mode ) ) ; 
	} 
	snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
	return( __nwu_open64(final_path, flags, mode));
}

int open(const char *path, int flags, ... ) { 
	mode_t		mode; 
	va_list		arg_list; 
	int		write_mode = 0; 
	char		final_path[PATH_MAX]; 

	char		short_path[PATH_MAX]; 

	if( __nwu_open == NULL ) { 
		__nwu_initialize(); 
	}

	__nwu_log( NWU_LOG_OPEN, "OPEN: %s - %02x\n", path, flags); 
	if( flags & O_CREAT ) {
		va_start(arg_list, flags); 
		mode = va_arg(arg_list, mode_t); 
		va_end(arg_list);
	} else { 
		mode = 0; 
	}

	if( ! __nwu_possible((char *) path, short_path) ) { 
		return( __nwu_open( path, flags, mode ) ); 
	}

	if( (flags & O_WRONLY) || (flags & O_RDWR) ) { 
		write_mode = 1; 
	} 
	if( write_mode && __nwu_exists_master((char *)short_path ) && ! __nwu_exists_local((char *)short_path) ) { 
		__nwu_copy((char *)short_path); 
	} 
	if( write_mode || __nwu_exists_local((char *)short_path) ) { 
		snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
		return( __nwu_open( final_path, flags, mode ) ) ; 
	} 
	snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
	return( __nwu_open(final_path, flags, mode));
}

FILE *fopen(const char *path, const char *mode) { 
	int	write_mode = 0; 
	char	last; 
	char	final_path[PATH_MAX]; 
	char	short_path[PATH_MAX]; 
	FILE	*retval; 

	if( __nwu_fopen == NULL ) { 
		__nwu_initialize(); 
	} 

	__nwu_log( NWU_LOG_FOPEN, "FOPEN: %s - %s\n", path, mode );

	switch( mode[0] ) { 
		case 'w': 
			write_mode = 1; 
			break; 
		case 'a': 
			write_mode = 1; 
			break; 
	}
	last = mode[1]; 
        if ( last == 'b' ) last = mode[2];   /* Ignore any possible 'b' */
        if ( last == '+') write_mode = 1; 

	if( ! __nwu_possible((char *) path, short_path) ) { 
		retval = __nwu_fopen( path, mode );
		if( retval != NULL ) {
			__nwu_fcache( path, retval );
		}
		return(retval);
	}

	if( write_mode && __nwu_exists_master((char *)short_path ) && ! __nwu_exists_local((char *)short_path) ) { 
		__nwu_copy((char *)short_path); 
	} 
	if( write_mode || __nwu_exists_local((char *)short_path) ) { 
		snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
		retval = __nwu_fopen( final_path, mode );
		if( retval != NULL ) {
			__nwu_fcache( path, retval );
		}
		return(retval);
	} 
	snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
	retval = __nwu_fopen(final_path, mode);
	if( retval != NULL ) {
		__nwu_fcache( path, retval );
	}
	return( retval );
}

FILE *fopen64(const char *path, const char *mode) { 
	int	write_mode = 0; 
	char	last; 
	char	final_path[PATH_MAX]; 
	char	short_path[PATH_MAX];
	FILE	*retval; 

	if( __nwu_fopen64 == NULL ) { 
		__nwu_initialize(); 
	}

	__nwu_log( NWU_LOG_FOPEN, "FOPEN64: %s - %s\n", path, mode);

	switch( mode[0] ) { 
		case 'w': 
			write_mode = 1; 
			break; 
		case 'a': 
			write_mode = 1; 
			break; 
	}
	last = mode[1]; 
        if ( last == 'b' ) last = mode[2];   /* Ignore any possible 'b' */
        if ( last == '+') write_mode = 1; 

	if( ! __nwu_possible((char *) path, short_path) ) { 
		retval = __nwu_fopen64( path, mode );
		if( retval != NULL ) {
			__nwu_fcache(path, retval);
		}
		return( retval );
	}

	if( write_mode && __nwu_exists_master((char *)short_path ) && ! __nwu_exists_local((char *)short_path) ) { 
		__nwu_copy((char *)short_path); 
	} 
	if( write_mode || __nwu_exists_local((char *)short_path) ) { 
		snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path); 
		retval = __nwu_fopen64( final_path, mode ); 
		if( retval != NULL ) { 
			__nwu_fcache(path, retval); 
		} 
		return( retval ); 
	} 
	snprintf(final_path, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_path); 
	retval = __nwu_fopen64(final_path, mode); 
	if( retval != NULL ) { 
		__nwu_fcache(path, retval); 
	} 
	return( retval ); 
}

int unlink(const char *pathname) { 
	char    newpath[PATH_MAX];
	int	retval; 
	char	short_path[PATH_MAX];

	if( __nwu_unlink == NULL ) { 
		__nwu_initialize(); 
	}

	if( __nwu_possible((char *)pathname, short_path) ) {
		snprintf(newpath, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path);
		__nwu_log(NWU_LOG_UNLINK, "UNLINK1: %s\n", newpath); 
		retval = __nwu_unlink( newpath ); 
		if( retval < 0 && errno == ENOENT ) { 	/* Fix errno */
			errno = EPERM; 
		} 
		return(retval); 
	}
	__nwu_log(NWU_LOG_UNLINK, "UNLINK2: %s\n", pathname); 
	return(__nwu_unlink( pathname ));
}

int remove(const char *pathname) { 
	char    newpath[PATH_MAX];
	int	retval; 
	char	short_path[PATH_MAX]; 

	if( __nwu_remove == NULL ) { 
		__nwu_initialize(); 
	} 

	if( __nwu_possible((char *)pathname, short_path) ) {
		snprintf(newpath, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_path);
		__nwu_log(NWU_LOG_UNLINK, "REMOVE1: %s\n", newpath); 
		retval = __nwu_remove( newpath ); 
		if( retval < 0 && errno == ENOENT ) { 	/* Fix errno */
			errno = EPERM; 
		} 
		return(retval); 
	}
	__nwu_log(NWU_LOG_UNLINK, "REMOVE2: %s\n", pathname); 
	return(__nwu_remove( pathname ));
}


