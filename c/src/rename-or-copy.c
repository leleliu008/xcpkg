#include <errno.h>
#include <stdio.h>

#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_rename_or_copy_file(const char * fromFilePath, const char * toFilePath) {
    if (fromFilePath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (toFilePath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (fromFilePath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (toFilePath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    size_t slashIndex = 0U;

    for (size_t i = 0U; ; i++) {
        if (toFilePath[i] == '\0') {
            break;
        }

        if (toFilePath[i] == '/') {
            slashIndex = i;
        }
    }

    if (slashIndex > 0U) {
        char outputDIR[slashIndex + 1U];

        for (size_t i = 0U; i < slashIndex; i++) {
            outputDIR[i] = toFilePath[i];
        }

        outputDIR[slashIndex] = '\0';

        int ret = xcpkg_mkdir_p(outputDIR, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    struct stat st;

    if (lstat(fromFilePath, &st) == -1) {
        perror(fromFilePath);
        return XCPKG_ERROR;
    }

    if (S_ISREG(st.st_mode)) {
        if (rename(fromFilePath, toFilePath) == 0) {
            return XCPKG_OK;
        } else {
            if (errno == EXDEV) {
                return xcpkg_copy_file(fromFilePath, toFilePath);
            } else {
                perror(toFilePath);
                return XCPKG_ERROR;
            }
        }
    }

    if (S_ISDIR(st.st_mode)) {
        fprintf(stderr, "%s is expected to be a regular file, but it is actually a directory.\n", fromFilePath);
        return XCPKG_ERROR;
    }

    if (S_ISLNK(st.st_mode)) {
        if (stat(fromFilePath, &st) == 0) {
            if (S_ISREG(st.st_mode)) {
                return xcpkg_copy_file(fromFilePath, toFilePath);
            }

            if (S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s is expected to be a regular file, but it is actually a directory.\n", fromFilePath);
                return XCPKG_ERROR;
            }

            fprintf(stderr, "%s is expected to be a regular file, but it is actually not.\n", fromFilePath);
            return XCPKG_ERROR;
        } else {
            perror(fromFilePath);
            return XCPKG_ERROR;
        }
    }

    fprintf(stderr, "%s is expected to be a regular file, but it is actually not.\n", fromFilePath);
    return XCPKG_ERROR;
}
