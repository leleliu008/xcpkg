#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_get_home_dir(const char ** p, size_t * len, bool create) {
    if (p == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    const char * const xcpkgHomeDIR = getenv("XCPKG_HOME");

    if (create) {
        struct stat st;

        if (stat(xcpkgHomeDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", xcpkgHomeDIR);
                return XCPKG_ERROR;
            }
        } else {
            if (mkdir(xcpkgHomeDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(xcpkgHomeDIR);
                    return XCPKG_ERROR;
                }
            }
        }
    }

    (*p) = xcpkgHomeDIR;

    if (len != NULL) {
        (*len) = strlen(xcpkgHomeDIR);
    }

    return XCPKG_OK;
}
