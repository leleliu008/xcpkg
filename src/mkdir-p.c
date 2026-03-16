#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_mkdir_p(const char * dir, const bool verbose) {
    if (dir == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (dir[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (dir[0] == '/' && dir[1] == '\0') {
        return XCPKG_OK;
    }

    size_t cap = strlen(dir) + 1U;

    char buf[cap];

    struct stat st;

    for (size_t i = 0U; ; i++) {
        buf[i] = dir[i];

        if (dir[i] != '/' && dir[i] != '\0') {
            continue;
        }

        if (dir[i] == '/') {
            if (i == 0U) {
                continue;
            } else {
                buf[i] = '\0';
            }
        }

        if (stat(buf, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", buf);
                return XCPKG_ERROR;
            }
        } else {
            if (verbose) printf("mkdir %s\n", buf);

            if (mkdir(buf, S_IRWXU) != 0) {
                if (errno == EEXIST) {
                    if (stat(buf, &st) == 0) {
                        if (!S_ISDIR(st.st_mode)) {
                            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", buf);
                            return XCPKG_ERROR;
                        }
                    }
                } else {
                    perror(buf);
                    return XCPKG_ERROR;
                }
            }
        }

        if (dir[i] == '\0') {
            return XCPKG_OK;
        }

        if (dir[i] == '/') {
            buf[i] = '/';
        }
    }
}
