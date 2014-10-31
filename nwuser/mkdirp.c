/* 
 * mkdirp() implimentation.
 *
 * This code is a modified version of the code found here: 
 * http://www.ovmj.org/GNUnet/doxygen/html/storage_8c-source.html#l00329
 * 
 * Originally authored by:
 * (C) 2001, 2002 Christian Grothoff (and other contributing authors)
 * 
 * Modifications by: 
 * (C) 2004 David Holland (zzqzzq_zzq@hotmail.com)
 * (C) 2008 David Holland (david.w.holland@gmail.com)
 * Modifications are to remove ifdef's and remove portability code. 
 * 
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include "code.h"


char * __nwu_expandFileName(const char * fil) {

	char buffer[PATH_MAX];
	char *fm;
	char *fn;
	const char *fil_ptr;

	if (fil == NULL) return NULL;

	if (fil[0] == '/' ) {
		/* absolute path, just copy */
		return(strdup(fil));
	} else if (fil[0] == '~') {
		fm = getenv("HOME");
		if (fm == NULL) {
			/* keep it symbolic to show error to user! */
			fm = "$HOME";
		}

		/* do not copy '~' */
		fil_ptr = fil + 1;

		/* skip over dir seperator to be consistent */    
		if (fil_ptr[0] == '/' ) {
			fil_ptr++;
		}
	} else {
		fil_ptr = fil;
		if (getcwd(buffer, PATH_MAX) != NULL) {
			fm = buffer;
		} else {
			fm = "$PWD";
		}
	}

	fn = malloc(strlen(fm) + 1 + strlen(fil_ptr) + 1);

	sprintf(fn, "%s/%s", fm, fil_ptr);
	return fn;
}

int __nwu_isDirectory(const char * fil) {
	struct stat filestat;
	int ret;

	ret = stat(fil, &filestat);
	if (ret != 0) {
		return 0;
	}
	if (S_ISDIR(filestat.st_mode)) {
		return 1;
	}
	return 0;
}


int __nwu_mkdirp(const char *path, mode_t mode) {
	char * rdir;
	int len;
	int pos;
	int ret = 0;

	__nwu_log(NWU_LOG_MKDIR, "MKDIRP: %s (%04o)\n", path, mode); 
	rdir = __nwu_expandFileName(path); /* expand directory */
	len = strlen(rdir);

	pos = 1; /* skip heading '/' */

	while (pos <= len) {
		if ( (rdir[pos] == '/' ) || (pos == len) ) {
			rdir[pos] = '\0';
			if (! __nwu_isDirectory(path)) {
				if (0 != mkdir(rdir, mode) ) {
					if (errno != EEXIST) {
						ret = -1;
					}
				}
			}
			rdir[pos] = '/' ;       
		}      
		pos++;
	}   
	free(rdir);
	return ret;
}

