#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>

#include "xcpkg.h"

int xcpkg_formula_cat(const char * packageName, const char * targetPlatformName) {
    char formulaFilePath[PATH_MAX];

    int ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    int fd = open(formulaFilePath, O_RDONLY);

    if (fd == -1) {
        perror(formulaFilePath);
        return XCPKG_ERROR;
    }

    printf("formula: %s\n", formulaFilePath);

    char buf[1024];

    for(;;) {
        ssize_t readSize = read(fd, buf, 1024);

        if (readSize == -1) {
            perror(formulaFilePath);
            close(fd);
            return XCPKG_ERROR;
        }

        if (readSize == 0) {
            close(fd);
            return XCPKG_OK;
        }

        ssize_t writeSize = write(STDOUT_FILENO, buf, readSize);

        if (writeSize == -1) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }

        if (writeSize != readSize) {
            fprintf(stderr, "not fully written to stdout.");
            close(fd);
            return XCPKG_ERROR;
        }
    }
}
