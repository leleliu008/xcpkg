#include <string.h>

#include "xcpkg.h"

int xcpkg_extract_filetype_from_url(const char * url, char buf[], const size_t bufCapacity) {
    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    if (buf == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (bufCapacity == 0U) {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    int urlLength = 0U;

    int slashIndex = -1;

    int dotIndex1 = -1;
    int dotIndex2 = -1;

    for (int i = 0; ; i++) {
        if (url[i] == '?' || url[i] == '\0') {
            urlLength = i;
            break;
        }

        if (url[i] == '/') {
            slashIndex = i;
        } else if (url[i] == '.') {
            dotIndex1 = dotIndex2;
            dotIndex2 = i;
        }
    }

    if (slashIndex == -1) {
        return XCPKG_OK;
    }

    if (dotIndex2 == -1) {
        return XCPKG_OK;
    }

    if (dotIndex1 > 0) {
        const char * p = url + dotIndex1 + 1;

        if (strncmp(p, "tar.gz", 6) == 0) {
            size_t n = bufCapacity > 5U ? 5U : bufCapacity;
            strncpy(buf, ".tgz", n);
            buf[n] = '\0';
            return XCPKG_OK;
        } else if (strncmp(p, "tar.xz", 6) == 0) {
            size_t n = bufCapacity > 5U ? 5U : bufCapacity;
            strncpy(buf, ".txz", n);
            buf[n] = '\0';
            return XCPKG_OK;
        } else if (strncmp(p, "tar.lz", 6) == 0) {
            size_t n = bufCapacity > 5U ? 5U : bufCapacity;
            strncpy(buf, ".tlz", n);
            buf[n] = '\0';
            return XCPKG_OK;
        } else if (strncmp(p, "tar.bz2", 7) == 0) {
            size_t n = bufCapacity > 6U ? 6U : bufCapacity;
            strncpy(buf, ".tbz2", n);
            buf[n] = '\0';
            return XCPKG_OK;
        }
    }

    const char * p = url + dotIndex2;

    size_t capacity = urlLength - dotIndex2 + 1;

    size_t n = bufCapacity > capacity ? capacity : bufCapacity;

    for (size_t i = 0; i < n; i++) {
        buf[i] = p[i];
    }

    buf[n] = '\0';

    return XCPKG_OK;
}
