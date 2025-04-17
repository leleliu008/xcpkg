#include <errno.h>
#include <stdio.h>
#include <string.h>

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

    size_t i = 0U;
    size_t j = 0U;

    for (;;) {
        char c = toFilePath[i];

        if (c == '\0') {
            break;
        }

        if (c == '/') {
            j = i;
        }

        i++;
    }

    if (j > 0U) {
        char outputDIR[j + 2U];

        strncpy(outputDIR, toFilePath, j);

        outputDIR[j] = '\0';

        int ret = xcpkg_mkdir_p(outputDIR, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

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
