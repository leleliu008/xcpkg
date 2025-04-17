#include <string.h>

#include "xcpkg.h"

int xcpkg_examine_filetype_from_url(const char * url, char buf[], const size_t bufSize) {
    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (buf == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (bufSize == 0U) {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    size_t urlLength = 0U;

    int i = -1;
    int j = -1;
    int k = -1;

    for (;;) {
        char c = url[urlLength];

        if ((c == '?') || (c == '\0')) {
            break;
        } else {
            if (c == '/') {
                i = urlLength;
            } else if (c == '.') {
                j = k;
                k = urlLength;
            }

            urlLength++;
        }
    }

    if (urlLength == 0U) {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    if (k <= i) {
        return 0;
    }

    if (j > 0) {
        const char * p = url + j;

        if (strcmp(p, ".tar.gz") == 0) {
            size_t n = bufSize > 4U ? 4U : bufSize;
            strncpy(buf, ".tgz", n);
            buf[n] = '\0';
            return XCPKG_OK;
        } else if (strcmp(p, ".tar.xz") == 0) {
            size_t n = bufSize > 4U ? 4U : bufSize;
            strncpy(buf, ".txz", n);
            buf[n] = '\0';
            return XCPKG_OK;
        } else if (strcmp(p, ".tar.lz") == 0) {
            size_t n = bufSize > 4U ? 4U : bufSize;
            strncpy(buf, ".tlz", n);
            buf[n] = '\0';
            return XCPKG_OK;
        } else if (strcmp(p, ".tar.bz2") == 0) {
            size_t n = bufSize > 5U ? 5U : bufSize;
            strncpy(buf, ".tbz2", n);
            buf[n] = '\0';
            return XCPKG_OK;
        }
    }

    size_t n = urlLength - k;
    strncpy(buf, url + k, bufSize > n ? n : bufSize);
    buf[n] = '\0';

    return XCPKG_OK;
}
