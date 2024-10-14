/* 
 * Hack to enable per user settings for NWN.
 *
 * Copyright 2004 - David Holland zzqzzq_zzq@hotmail.com
 * Copyright 2008 - David Holland david.w.holland@gmail.com
 *
 * Do what you want with the code, just maintain
 * the copyright, and give credit
 *
 * Linked List utility functions
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
#include <libgen.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <stdarg.h>
#include <assert.h>

#include "code.h"

extern int __nwu_loglevel; 
extern int __nwuser_initialized; 

/* utility stack functions */

int __nwu_push( struct dirent64 *key, DIRENTRY **stack ) { 
	DIRENTRY 		*nextnode; 
	int			found = 0; 
	struct	dirent64	*cur; 
	DIRENTRY		*entry; 

/* Need to verify the name isn't already on the stack */
	entry = (*stack) ;
	while( entry != NULL ) { 
		cur = entry->entry;
#ifdef DEBUG
//		fprintf(stderr, "PUSH: %s %s\n", cur->d_name, key->d_name ); 
#endif 
		if( strcmp( cur->d_name, key->d_name ) == 0 ) { 
			found = 1; 
			break; 
		} 
		entry = entry -> next; 
	}
	if( !found ) { 
#ifdef DEBUG
////		fprintf(stderr, "PUSHED: %s\n", key->d_name); 
#endif

		/* Use 'cur' as a temporary variable. */
		cur = (struct dirent64 *)malloc(sizeof(struct dirent64)); 
		memcpy( cur, key, sizeof(struct dirent64) ); 
		nextnode = (DIRENTRY *)malloc(sizeof(DIRENTRY)); 
		nextnode -> entry = cur; 
		nextnode -> next = (*stack); 
		(*stack) = nextnode; 
		return(1); 
	} 
	return(0);
} 

void __nwu_dump( DIRSTACK *stack ) { 
	DIRENTRY	*entry; 

	entry = stack->elements; 
	while( entry != NULL ) { 
		__nwu_log(NWU_DEBUG_OPENDIR, "DUMP: %lld: %s\n", entry->entry->d_ino, entry->entry->d_name); 
		entry = entry->next; 
	} 
}

/* This routine allows us to short circuit some double checking. 
 * returns 1 if NWUser should "do" its things. 
 * - 9/7/04 - Its also buggy.  - Sigh.
 * - 10/5/04 - And its still buggy. - sigh.
 */ 

int __nwu_possible(char *name, char *new_path) { 
	char	*ptr; 
	char	dot[] = "."; 
	if( name != NULL && name[0] != '/' ) { 
		strcpy(new_path, name);
		__nwu_log(NWU_DEBUG_PATHING, "NWUPOSSIBLE: Relative Path: %s -> %s\n", name, new_path); 
		return(1); 
	} 
	if( name != NULL && __nwu_basedir != NULL ) { 
		ptr = strstr(name, __nwu_basedir); 
		if( ptr != NULL && ptr == name ) { 
			sprintf(new_path, "%s%s", dot, name + strlen(__nwu_basedir)); 
			if(strlen(new_path) == 0 ) { 
				strcpy(new_path, dot); 	/* shoudln't be possible */
			}
			__nwu_log(NWU_DEBUG_PATHING, "NWUPOSSIBLE: Absolute Path: %s -> %s\n", name, new_path); 
			return(1); 
		} 
	}
	strcpy(new_path, name); 
	__nwu_log(NWU_DEBUG_PATHING, "NWUPOSSIBLE: Unmatched Path: %s -> %s\n", name, new_path); 
	return(0); 
}

int __nwu_exists_local(char *name) { 
	char path[PATH_MAX]; 
	struct stat stat_buf; 

	/* force the leading / to make certain __xstat doesn't try to 
		look in the "master" area */

	assert( snprintf(path, PATH_MAX, "/%s/%s/%s", __nwu_homedir, __nwu_workingdir, name) >= 0 );
	if( __nwu_xstat( _STAT_VER, path, &stat_buf ) == 0 ) { 
		return(1); 
	} 
	return(0);
}

int __nwu_exists_master(char *name) { 
	struct stat stat_buf; 
	char path[PATH_MAX]; 

	assert( snprintf(path, PATH_MAX, "/%s/%s/%s", __nwu_basedir, __nwu_workingdir, name) >= 0 );
	if( __nwu_xstat( _STAT_VER, path, &stat_buf ) == 0 ) { 
		return(1); 
	} 
	return(0);
}

void __nwu_copy(char *path) { 
	int 	input, output;
	off_t 	offset = 0;
	char	*dirp; 
	char	*dname; 
	char	dpath[PATH_MAX]; 

	if( path == NULL || strlen(path) == 0 ) { 
		return; 
	} 

	dirp = strdup(path); 
	dname = dirname(dirp);
	assert( snprintf(dpath, PATH_MAX, "%s/%s", __nwu_homedir, dname) >= 0 );
	if( __nwu_mkdirp( dpath, 0777 ) != 0  && errno != EEXIST ) { 
		return; 
	} 
	free(dirp); 

	if( !__nwu_exists_local(path) && __nwu_exists_master(path) ) { 
		assert( snprintf(dpath, PATH_MAX, "%s/%s", __nwu_homedir, path) >= 0 );
		input = __nwu_open64( path, O_RDONLY ); 
		output = __nwu_open64( dpath, O_RDWR|O_CREAT, 0666 ); 
		if( input < 0 || output < 0 ) { 
			return; 
		} 
		while( sendfile( output, input, &offset, 65535 ) > 0 ) { 
			/* do nothing,  */
		} 
		close(input); 
		close(output); 
	} 
	return; 
}

void __nwu_log(int level, char *fmt, ...) {
        va_list arg_list;
	static FILE	*nwu_log = NULL; 
	static int	logging = 0; 
	static char	log_location[PATH_MAX] = ""; 

	if( __nwuser_initialized ) {			/* Avoid the fopen() if we're not initialized */

		if( ! logging ) { 			/* avoid the recursive fopen() error */

			logging = 1; 

			if( strlen(log_location) == 0 ) { 
				assert( snprintf(log_location, PATH_MAX, "%s/nwuser.log", __nwu_homedir) >= 0 );
			} 

			if( level & __nwu_loglevel ) {
		
				if( ! nwu_log ) { 	
					nwu_log = __nwu_fopen(log_location, "a");
				}
				va_start(arg_list, fmt);
				if( nwu_log != NULL && getenv("NWU_STDERR") == NULL ) {
					vfprintf(nwu_log, fmt, arg_list);
					fdatasync(fileno(nwu_log)); 		/* sync the log file */
				} else {
					vfprintf(stderr, fmt, arg_list);
				}
				va_end(arg_list);
			}
		}

	} else { 
		if( level & __nwu_loglevel ) {
			va_start(arg_list, fmt);
			vfprintf(stderr, fmt, arg_list);
			va_end(arg_list);
			fdatasync(fileno(stderr));              /* sync the log file */
		}
	}

	logging = 0; 

	return;
}

