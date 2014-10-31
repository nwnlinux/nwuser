/* 
 * Hack to enable per user settings for NWN.
 *
 * Copyright 2004 - David Holland zzqzzq_zzq@hotmail.com
 * Copyright 2008 - David Holland david.w.holland@gmail.com
 *
 * Do what you want with the code, just maintain
 * the copyright, and give credit
 * 
 * Directory functions.
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

#include "code.h"

/* ipendir() implimentation. */ 

DIR *opendir(const char *name) { 
	DIR			*dir_master; 
	DIR			*dir_local; 
	struct	dirent64	*dir_entry; 

	DIRSTACK		*dir_stack; 

	char			local_name[PATH_MAX]; 
	char			short_name[PATH_MAX]; 

	if( __nwu_opendir == NULL ) { 
		__nwu_initialize(); 
	} 

	__nwu_possible((char *)name, short_name); 

	__nwu_log(NWU_LOG_OPENDIR, "OPENDIR: %s\n", name); 

	dir_stack = malloc(sizeof(DIRSTACK)); 

	dir_stack->num_elements = 0; 
	dir_stack->cur_element = 0; 
	dir_stack->elements = NULL; 
	dir_stack->buffer = NULL; 

	snprintf(local_name, PATH_MAX, "%s/%s/%s", __nwu_homedir, __nwu_workingdir, short_name); 
	__nwu_log( NWU_DEBUG_OPENDIR, "OPENDIR: Checking home: %s\n", local_name); 

	if( (dir_local = __nwu_opendir(local_name) ) != NULL ) { 
		while( (dir_entry = __nwu_readdir64( dir_local )) != NULL ) { 
			__nwu_log( NWU_DEBUG_OPENDIR, "LOCAL: %s\n", dir_entry->d_name); 
			dir_stack->num_elements = dir_stack->num_elements + __nwu_push( dir_entry, &dir_stack->elements ); 
		}
		__nwu_closedir(dir_local); 
	} 
	__nwu_dump( dir_stack ); 

	snprintf(local_name, PATH_MAX, "%s/%s/%s", __nwu_basedir, __nwu_workingdir, short_name); 
	__nwu_log( NWU_DEBUG_OPENDIR, "OPENDIR: Checking base: %s\n", local_name); 

	if( (dir_master = __nwu_opendir(local_name) ) != NULL ) { 
		while( (dir_entry = __nwu_readdir64( dir_master )) != NULL ) { 
			__nwu_log(NWU_DEBUG_OPENDIR, "MASTER: %s\n", dir_entry->d_name); 
			dir_stack->num_elements = dir_stack->num_elements + __nwu_push( dir_entry, &dir_stack->elements ); 
		}
		__nwu_closedir(dir_master); 
	}

	if( dir_stack->num_elements == 0 ) { 
		free(dir_stack); 
		errno = ENOENT; 
		return(NULL); 
	} 
	__nwu_dump(dir_stack); 
	return((DIR *)dir_stack); 
}

struct dirent *readdir(DIR *dir) { 
	int 		i; 
	DIRSTACK	*dir_stack; 
	DIRENTRY	*dir_entry; 
	struct	dirent	*buffer; 

	dir_stack = (DIRSTACK *)dir; 
	if( dir_stack->cur_element >= dir_stack->num_elements ) { 
		return(NULL); 
	}

	if( dir_stack->buffer == NULL ) { 
		/* 32bit filesize read, only need 32bit type dirent structure */
		buffer = (struct dirent *)malloc(sizeof(struct dirent));   
	} else { 
		buffer = (struct dirent *)dir_stack->buffer; 
	}

	dir_entry = dir_stack->elements; 
	i = 0; 
	while( i < dir_stack->cur_element && dir_entry != NULL ) { 
		i++; 
		dir_entry = dir_entry->next; 
	} 
	dir_stack->cur_element ++; 

	buffer->d_ino = dir_entry->entry->d_ino; 
	buffer->d_off = dir_entry->entry->d_off; 
	buffer->d_reclen = dir_entry->entry->d_reclen; 
	buffer->d_type = dir_entry->entry->d_type; 
	memcpy(buffer->d_name, dir_entry->entry->d_name, NAME_MAX + 1); 
	dir_stack->buffer = (struct dirent64 *)buffer; 

	return( (struct dirent *)dir_stack->buffer ); 
}

struct dirent64 *readdir64(DIR *dir) { 
	int 		i; 
	DIRSTACK	*dir_stack; 
	DIRENTRY	*dir_entry; 

	dir_stack = (DIRSTACK *)dir; 
	if( dir_stack->cur_element >= dir_stack->num_elements ) { 
		return(NULL); 
	}

	if( dir_stack->buffer == NULL ) { 
		/* 32bit filesize read, only need 32bit type dirent structure */
		dir_stack->buffer = (struct dirent64 *)malloc(sizeof(struct dirent64));   
	}

	dir_entry = dir_stack->elements; 
	i = 0; 
	while( i < dir_stack->cur_element && dir_entry != NULL ) { 
		i++; 
		dir_entry = dir_entry->next; 
	} 
	dir_stack->cur_element ++; 

	dir_stack->buffer->d_ino = dir_entry->entry->d_ino; 
	dir_stack->buffer->d_off = dir_entry->entry->d_off; 
	dir_stack->buffer->d_reclen = dir_entry->entry->d_reclen; 
	dir_stack->buffer->d_type = dir_entry->entry->d_type; 
	memcpy(dir_stack->buffer->d_name, dir_entry->entry->d_name, NAME_MAX + 1); 

	return( (struct dirent64 *)dir_stack->buffer ); 
}

int closedir(DIR *dir) { 
	DIRSTACK	*dir_stack; 
	DIRENTRY	*dir_entry; 
	DIRENTRY	*next; 

	__nwu_log(NWU_LOG_OPENDIR, "CLOSEDIR()\n"); 

	dir_stack = (DIRSTACK *)dir; 
	dir_entry = dir_stack->elements; 
	while( dir_entry != NULL ) { 
		next = dir_entry->next; 
		free(dir_entry->entry); 
		free(dir_entry); 
		dir_entry = next; 
	} 
	if( dir_stack->buffer != NULL ) { 
		free( dir_stack->buffer ); 
	} 
	free(dir_stack); 
	errno = 0; 
	return(0); 
}

int mkdir(const char *pathname, mode_t mode) { 
	char	newpath[PATH_MAX]; 
	int	retval; 
	int	tmp_errno; 
	char	short_path[PATH_MAX];

	if( __nwu_mkdir == NULL ) { 
		__nwu_initialize(); 
	}

	__nwu_log(NWU_LOG_MKDIR, "MKDIR: %s - %04o\n", pathname, mode); 
	if( __nwu_possible((char *)pathname, short_path) ) { 
		snprintf(newpath, PATH_MAX, "%s/%s", __nwu_homedir, short_path); 
		retval = __nwu_mkdir( newpath, mode ); 
		tmp_errno = errno; 
		if ( __nwu_exists_master( (char *)short_path ) ) { 
			errno = EEXIST; 
			return(-1); 
		} 
		errno = tmp_errno;
		return(retval); 
	} 
	return(__nwu_mkdir( pathname, mode )); 
}

int rmdir(const char *pathname) { 
	char	newpath[PATH_MAX]; 
	char	short_path[PATH_MAX];

	if( __nwu_rmdir == NULL ) { 
		__nwu_initialize(); 
	}

	__nwu_log(NWU_LOG_RMDIR, "RMDIR: %s\n", pathname); 
	if( __nwu_possible((char *)pathname, short_path) ) { 
		snprintf(newpath, PATH_MAX, "%s/%s", __nwu_homedir, short_path); 
		return( __nwu_rmdir( newpath ) ); 
	} 
	return(__nwu_rmdir( pathname )); 
}

/* probably not a particularly good implimentation */

int chdir(const char *path) { 
	char	newpath[PATH_MAX];
	char	short_path[PATH_MAX];

	if( path == NULL || strlen(path) <= 0 ) { 
		errno = EFAULT; 
		return(-1); 
	}
	__nwu_log(NWU_LOG_CHDIR, "CHDIR: %s\n", path); 
	if( __nwu_possible((char *)path, short_path) ) { 
		snprintf(__nwu_workingdir, PATH_MAX, "%s", short_path); 
		snprintf(newpath, PATH_MAX, "%s/%s", __nwu_homedir, short_path);
		return(0); 
	} 
	strcpy(__nwu_workingdir, ""); 
	return(0); 
}
