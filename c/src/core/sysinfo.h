#ifndef SYSINFO_H
#define SYSINFO_H

#include <errno.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>

typedef struct {
   unsigned int ncpu;
   char  vers[11];
   char  arch[11];
   uid_t euid;
   gid_t egid;
} SysInfo;

int  sysinfo_vers(char * buf, size_t bufSize);
int  sysinfo_arch(char * buf, size_t bufSize);
int  sysinfo_ncpu();

int  sysinfo_make(SysInfo * sysinfo);
void sysinfo_dump(SysInfo * sysinfo);

#endif
