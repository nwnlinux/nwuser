typedef struct dir_entry {
        struct dirent64 *entry;
        struct dir_entry *next;
} DIRENTRY;

typedef struct dir_stack {
        int             num_elements;
        int             cur_element;
        DIRENTRY        *elements;
        struct  dirent64        *buffer;
} DIRSTACK;

/* Not all of these are currently implimented */

#define NWU_LOG_OPENDIR 0x01            /* opendir/readdir/closedir */
#define NWU_LOG_OPEN    0x02            /* open/fopen */
#define NWU_LOG_MKDIR   0x04            /* mkdir */
#define NWU_LOG_RMDIR   0x08            /* rmdir */
#define NWU_LOG_UNLINK  0x10            /* unlink */
#define NWU_LOG_STAT    0x20            /* xstat/lxstat */
#define NWU_LOG_CHDIR   0x40            /* chdir */
#define NWU_DEBUG_OPENDIR 0x80          /* debug out of opendir() replacement */
#define NWU_DEBUG_PATHING 0x100		/* Debug annoying stuff. :-?  */
#define NWU_LOG_FOPEN		0x200            /* fopen */
#define NWU_DEBUG_STDIO		0x400          /* fopen */


int __nwu_push( struct dirent64 *key, DIRENTRY **stack );
void __nwu_dump( DIRSTACK *stack );
int __nwu_possible(char *name, char *new_path);
int __nwu_exists_local(char *name);
int __nwu_exists_master(char *name);
void __nwu_copy(char *name);

void __nwu_log(int level, char *fmt, ...);
void __nwu_fcache(const char *path, FILE *stream);

void __nwu_initialize(void) __attribute__((constructor)); 

/* "global" variable definitions.                                 */
/* prepended with a __nwu_  to hopefully not conflict w/ anything */

#ifndef _DEFINE_GLOBALS

extern int (*__nwu_open)(const char *path, int flags, ...);
extern int (*__nwu_open64)(const char *path, int flags, ...);

extern FILE *(*__nwu_fopen)(const char *path, const char *mode);
extern FILE *(*__nwu_fopen64)(const char *path, const char *mode);

extern size_t (*__nwu_fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);
extern int (*__nwu_fclose)(FILE *stream);

extern DIR *(*__nwu_opendir)(const char *name);
extern int (*__nwu_closedir)(DIR *dir);
extern int (*__nwu_mkdir)(const char *pathname, mode_t mode);
extern int (*__nwu_rmdir)(const char *pathname );
extern int (*__nwu_unlink)(const char *pathname ); 
extern int (*__nwu_remove)(const char *pathname ); 
extern int (*__nwu_chdir)(const char *pathname ); 

extern struct dirent *(*__nwu_readdir)(DIR *dir);
extern struct dirent64 *(*__nwu_readdir64)(DIR *dir);

extern int (*__nwu_xstat)(int ver, const char *file_name, struct stat *buf);
extern int (*__nwu_xstat64)(int ver, const char *file_name, struct stat64 *buf);

extern int (*__nwu_lxstat)(int ver, const char *file_name, struct stat *buf);
extern int (*__nwu_lxstat64)(int ver, const char *file_name, struct stat64 *buf);

extern int __nwu_mkdirp(const char *path, mode_t mode);

extern char     __nwu_homedir[PATH_MAX];
extern char	__nwu_workingdir[PATH_MAX];
extern char	*__nwu_basedir; 

#endif


