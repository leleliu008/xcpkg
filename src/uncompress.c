#include <time.h>
#include <math.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/tar.h"

#include "sha256sum.h"

#include "xcpkg.h"

int xcpkg_uncompress(const char * filePath, const char * unpackDIR, const size_t stripComponentsNumber, const bool verbose) {
    if (filePath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (filePath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (unpackDIR == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (unpackDIR[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    int ret = tar_extract(unpackDIR, filePath, ARCHIVE_EXTRACT_TIME, verbose, stripComponentsNumber);

    if (ret == 0) {
        return XCPKG_OK;
    } else {
        return abs(ret) + XCPKG_ERROR_ARCHIVE_BASE;
    }
}
