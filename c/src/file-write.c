#include <stddef.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "xcpkg.h"

int xcpkg_write_file(const char * fp, const char * str, size_t strLen) {
    int fd = open(fp, O_WRONLY | O_CREAT | O_TRUNC, 0666);

    if (fd == -1) {
        perror(fp);
        return XCPKG_ERROR;
    }

    if (strLen == 0U) strLen = strlen(str);

    ssize_t writtenSize = write(fd, str, strLen);

    if (writtenSize == -1) {
        perror(fp);
        close(fd);
        return XCPKG_ERROR;
    }

    if (writtenSize == (ssize_t)strLen) {
        return XCPKG_OK;
    } else {
        fprintf(stderr, "file not fully written: %s\n", fp);
        return XCPKG_ERROR;
    }
}
