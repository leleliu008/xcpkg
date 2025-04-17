#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/utsname.h>

#include "sysinfo.h"


int sysinfo_arch(char * buf, size_t bufCapacity) {
    struct utsname uts;

    if (uname(&uts) < 0) {
        return -1;
    }

    size_t capcity = strlen(uts.machine) + 1;

    strncpy(buf, uts.machine, (bufCapacity > capcity) ? capcity : bufCapacity);

    return 0;
}

int sysinfo_vers(char * buf, size_t bufCapacity) {
    const char * const fp = "/System/Library/CoreServices/SystemVersion.plist";

    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return -1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror(fp);
        close(fd);
        return -1;
    }

    if (st.st_size == 0) {
        fprintf(stderr, "empty file: %s\n", fp);
        close(fd);
        return -1;
    }

    char * data = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        perror(fp);
        close(fd);
        return -1;
    }

    char * p = strstr(data, "<key>ProductVersion</key>");

    if (p == NULL) {
        fprintf(stderr, "no <key>ProductVersion</key> in file : %s\n", fp);

        if (munmap(data, st.st_size) == -1) {
            perror("Failed to unmap file");
        }

        close(fd);

        return -1;
    }

    p = strstr(p, "<string>");

    if (p == NULL) {
        fprintf(stderr, "no <string> after <key>ProductVersion</key> in file : %s\n", fp);

        if (munmap(data, st.st_size) == -1) {
            perror("Failed to unmap file");
        }

        close(fd);

        return -1;
    }

    p += 8;

    char * q = strstr(p, "</string>");

    if (q == NULL) {
        fprintf(stderr, "no </string> after <key>ProductVersion</key> in file : %s\n", fp);

        if (munmap(data, st.st_size) == -1) {
            perror("Failed to unmap file");
        }

        close(fd);

        return -1;
    }

    size_t len = q - p;

    size_t n = (len > bufCapacity - 1) ? bufCapacity - 1 : len;

    strncpy(buf, p, n);

    buf[n] = '\0';

    if (munmap(data, st.st_size) == -1) {
        perror("Failed to unmap file");
    }

    close(fd);

    return 0;
}

int sysinfo_ncpu() {
    long nprocs;

#if defined (_SC_NPROCESSORS_ONLN)
    nprocs = sysconf(_SC_NPROCESSORS_ONLN);

    if (nprocs > 0L) {
        return nprocs;
    }
#endif

#if defined (_SC_NPROCESSORS_CONF)
    nprocs = sysconf(_SC_NPROCESSORS_CONF);

    if (nprocs > 0L) {
        return nprocs;
    }
#endif

    nprocs = 1L;
    return nprocs;
}

int sysinfo_make(SysInfo * sysinfo) {
    if (sysinfo == NULL) {
        errno = EINVAL;
        return -1;
    }

    int ret;

    ///////////////////////////////////////

    ret = sysinfo_arch(sysinfo->arch, 11);

    if (ret != 0) {
        return ret;
    }

    ///////////////////////////////////////

    ret = sysinfo_vers(sysinfo->vers, 11);

    if (ret != 0) {
        return ret;
    }

    ///////////////////////////////////////

    int ncpu = sysinfo_ncpu();

    if (ncpu < 0) {
        return ncpu;
    }

    sysinfo->ncpu = ncpu;

    ///////////////////////////////////////

    sysinfo->euid = geteuid();
    sysinfo->egid = getegid();

    return 0;
}

void sysinfo_dump(SysInfo * sysinfo) {
    printf("sysinfo.ncpu: %u\n", sysinfo->ncpu);
    printf("sysinfo.arch: %s\n", sysinfo->arch);
    printf("sysinfo.vers: %s\n", sysinfo->vers);
    printf("sysinfo.euid: %u\n", sysinfo->euid);
    printf("sysinfo.egid: %u\n", sysinfo->egid);
}
