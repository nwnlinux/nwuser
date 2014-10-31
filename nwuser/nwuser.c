/* 
 * 
 * Hack to enable per user settings for NWN.
 *
 * Copyright 2004 - David Holland zzqzzq_zzq@hotmail.com
 * Copyright 2008 - David Holland david.w.holland@gmail.com
 *
 * Do what you want with the code, just maintain
 * the copyright, and give credit
 * 
 * Initialization functions. 
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

#ifdef CRASH

#include <signal.h>
#include <execinfo.h>

#endif

#define _DEFINE_GLOBALS
#include "code.h"
#include "nwuser.h"

int (*__nwu_open)(const char *path, int flags, ...) = NULL;
int (*__nwu_open64)(const char *path, int flags, ...) = NULL;

FILE *(*__nwu_fopen)(const char *path, const char *mode) = NULL;
FILE *(*__nwu_fopen64)(const char *path, const char *mode) = NULL;
size_t (*__nwu_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream) = NULL;
int (*__nwu_fclose)(FILE *stream) = NULL;

DIR *(*__nwu_opendir)(const char *name) = NULL;
int (*__nwu_closedir)(DIR *dir) = NULL;
int (*__nwu_mkdir)(const char *pathname, mode_t mode) = NULL;
int (*__nwu_rmdir)(const char *pathname ) = NULL;
int (*__nwu_unlink)(const char *pathname ) = NULL;
int (*__nwu_remove)(const char *pathname ) = NULL;
int (*__nwu_chdir)(const char *pathname ) = NULL;

struct dirent *(*__nwu_readdir)(DIR *dir) = NULL;
struct dirent64 *(*__nwu_readdir64)(DIR *dir) = NULL;

int (*__nwu_xstat)(int ver, const char *file_name, struct stat *buf) = NULL;
int (*__nwu_xstat64)(int ver, const char *file_name, struct stat64 *buf) = NULL;

int (*__nwu_lxstat)(int ver, const char *file_name, struct stat *buf) = NULL;
int (*__nwu_lxstat64)(int ver, const char *file_name, struct stat64 *buf) = NULL;

char	*__nwu_basedir = NULL; 
char 	__nwu_homedir[PATH_MAX] = ""; 
char	__nwu_workingdir[PATH_MAX] = ""; 
int	__nwu_loglevel = 0; 

int	__nwuser_initialized = 0; 

#ifdef CRASH 

void nwu_sighandler(int sig) { 
	int 	fd = -1 ; 
	char	msg[1024]; 
	void	*array[1024]; 	/* large stack dump */
	size_t	size; 
	pid_t	my_pid; 
	pid_t	pid; 

	int (*__nwu_copen)(const char *path, int flags, ...) = NULL;

/* Write rudamentary crash log */ 
	__nwu_copen = dlsym(RTLD_NEXT, "open"); 

	if( __nwu_copen != NULL ) { 
		fd = __nwu_copen("/tmp/nwu_crash.log", O_CREAT | O_WRONLY, 0644 ); 
	} 

	if( fd < 0 || __nwu_copen == NULL ) { 
		fd = fileno(stderr); 
	} 
	sprintf(msg, "Aieeeeeeee, NWUser Crashed: %d\n", sig); 
	write(fd, msg, strlen(msg)); 

	size = backtrace(array, 1024); 
	backtrace_symbols_fd(array, size, fd); 

	close(fd); 

/* Try for a more complex crash log */

	if( __nwu_copen != NULL ) { 

		fd = __nwu_copen("/tmp/nwu_crash.cmd", O_CREAT | O_WRONLY, 0644 ); 
		if( fd < 0 ) { 
			exit(-1); 
		}
		sprintf(msg, "where\n"); 
		write(fd, msg, strlen(msg)); 

		sprintf(msg, "quit\n"); 
		write(fd, msg, strlen(msg)); 

		close(fd); 

		putenv("LD_PRELOAD"); 	/* Unset the preload variable */
		my_pid = getpid(); 

		switch( pid = fork() ) { 
		case -1: 
			/* fork failed */
			exit(-1); 
		case 0: 
			/* child */	
			
			sprintf(msg, "gdb /proc/%d/exe %d < /tmp/nwu_crash.cmd > /tmp/nwu_crash2.log 2>&1", my_pid, my_pid); 
			system(msg); 
			_exit(0); 
		default: 
			/* parent */ 
			sleep(10); 
		}
	}

	exit(-1); 
}

#endif 

/* Initailize this preload library */ 

void __nwu_initialize(void) __attribute__((constructor)); 

void __nwu_initialize(void) { 
	struct	passwd	*pwent; 
	char		*level; 
	int		sleeptime; 

	if( ! __nwuser_initialized ) { 

/* sleep so we can attach a debugger */

	if ((level = getenv("NWU_SLEEPTIME"))) {
		sleeptime = atoi(level);
		sleep(sleeptime); 
	} 

#ifdef CRASH
	signal(SIGBUS, nwu_sighandler); 
	signal(SIGSEGV, nwu_sighandler); 
#endif

	if ((level = getenv("NWU_VERBOSE"))) __nwu_loglevel = atoi(level);

	__nwu_open = dlsym(RTLD_NEXT, "open"); 
	__nwu_open64 = dlsym(RTLD_NEXT, "open64"); 
	__nwu_fopen = dlsym(RTLD_NEXT, "fopen"); 
	__nwu_fopen64 = dlsym(RTLD_NEXT, "fopen64"); 
	__nwu_fread = dlsym(RTLD_NEXT, "fread");
	__nwu_fclose = dlsym(RTLD_NEXT, "fclose");

 		/* Not really necessary, for NWN, but it makes LS happy,  */
	__nwu_xstat = dlsym(RTLD_NEXT, "__xstat");
	__nwu_lxstat = dlsym(RTLD_NEXT, "__lxstat");
	__nwu_xstat64 = dlsym(RTLD_NEXT, "__xstat64");
	__nwu_lxstat64 = dlsym(RTLD_NEXT, "__lxstat64");

	__nwu_opendir = dlsym(RTLD_NEXT, "opendir"); 
	__nwu_closedir = dlsym(RTLD_NEXT, "closedir"); 
	__nwu_mkdir = dlsym(RTLD_NEXT, "mkdir"); 
	__nwu_rmdir = dlsym(RTLD_NEXT, "rmdir"); 
	__nwu_unlink = dlsym(RTLD_NEXT, "unlink"); 
	__nwu_remove = dlsym(RTLD_NEXT, "remove"); 
	__nwu_chdir = dlsym(RTLD_NEXT, "chdir"); 

	__nwu_readdir = dlsym(RTLD_NEXT, "readdir"); 
	__nwu_readdir64 = dlsym(RTLD_NEXT, "readdir64"); 

	pwent = getpwuid(getuid()); 
	if( pwent != NULL ) { 
		strncpy( __nwu_homedir, pwent->pw_dir, PATH_MAX - 100 ); /* leave some extra space */
		strcat ( __nwu_homedir, "/.nwn" ); 
	}

	__nwu_basedir = (char *)malloc(PATH_MAX); 
	if( ! __nwu_basedir ) { 
		fprintf(stderr, "ERROR: Unable to allocate PATH_MAX memory: %d\n", errno); 
		exit(-1); 
	}

	if( !getcwd( __nwu_basedir, PATH_MAX) ) {
		fprintf(stderr, "ERROR: getcwd() failed.  Toto, I don't think we're in Kansas anymore.\n"); 
		exit(-1); 
	}
	fprintf(stderr, "NOTICE: NWUser: Version %s Successfully loaded. (BaseDir = %s)\n", _NWUSER_VERSION, __nwu_basedir); 

	__nwuser_initialized = 1; 

	__nwu_loglevel = 1; 
	__nwu_log(1, "NOTICE: NWUser: Version %s Successfully loaded. (BaseDir = %s)\n", _NWUSER_VERSION, __nwu_basedir);

	if ((level = getenv("NWU_VERBOSE"))) {
		__nwu_loglevel = atoi(level);
	} else { 
		__nwu_loglevel = 0; 
	}

	} 
}

