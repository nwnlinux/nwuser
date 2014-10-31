/* 
 * Hack to enable per user settings for NWN.
 *
 * Copyright 2004 - David Holland zzqzzq_zzq@hotmail.com
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
#include <sys/mman.h>
#include <limits.h>

#ifndef PAGESIZE
#define PAGESIZE 4096
#endif

#include "code.h"

char *__nwu_fcache_dat[8192]; 

static int __nwu_fcache_init = 0; 

void __nwu_fcache( const char *path, FILE *stream ) { 
	int i; 

	if( ! __nwu_fcache_init ) { 
		for( i = 0; i < 1024; i++ ) { 
			__nwu_fcache_dat[i] = NULL;
		}
		__nwu_fcache_init = 1; 
	}

	__nwu_fcache_dat[fileno(stream)] = malloc(strlen(path) + 1); 
	if( __nwu_fcache_dat[fileno(stream)] != NULL ) { 
		strcpy( __nwu_fcache_dat[fileno(stream)], path ); 
	}
	return; 
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) { 
	size_t 	retval; 

	if( __nwu_fread == NULL ) { 
		__nwu_initialize(); 
	} 

	if( __nwu_fcache_dat[ fileno(stream) ] == NULL ) { 
		__nwu_log( NWU_DEBUG_STDIO, "FREAD( 0x%08x, %d, %d = (%d), fileno(%d) )\n", ptr, size, nmemb, size * nmemb, fileno(stream) ); 
	} else { 
		__nwu_log( NWU_DEBUG_STDIO, "FREAD( 0x%08x, %d, %d = (%d), %s )\n", ptr, size, nmemb, size * nmemb, __nwu_fcache_dat[ fileno(stream) ] ); 
	}

	retval = __nwu_fread( ptr, size, nmemb, stream); 

	return( retval ); 
}



int fclose(FILE *stream) { 

	if( __nwu_fclose == NULL ) { 
		__nwu_initialize(); 
	}


	if( __nwu_fcache_dat[ fileno(stream) ] != NULL ) { 
		__nwu_log( NWU_DEBUG_STDIO, "FCLOSE( %s ) \n", __nwu_fcache_dat[ fileno(stream) ]);
		free( __nwu_fcache_dat[ fileno(stream) ] ); 
		__nwu_fcache_dat[ fileno(stream) ] = NULL; 
	} else { 
		__nwu_log(NWU_DEBUG_STDIO, "FCLOSE( NULL )\n"); 
	} 

	return( __nwu_fclose( stream )) ; 
}
