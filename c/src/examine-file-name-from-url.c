#include <string.h>

#include "xcpkg.h"

int xcpkg_examine_filename_from_url(const char * url, char buf[], const size_t bufSize) {
    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (buf == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (bufSize == 0U) {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    int slashIndex = -1;

    int i = 0;

    for (;;) {
        char c = url[i];

        if ((c == '?') || (c == '\0')) {
            break;
        }

        if (c == '/') {
            slashIndex = i;
        }

        i++;
    }

    if (slashIndex == -1) {
        size_t n = i;
        strncpy(buf, url, bufSize > n ? n : bufSize);
        buf[n] = '\0';
    } else {
        size_t n = i - slashIndex - 1;
        strncpy(buf, url + slashIndex + 1, bufSize > n ? n : bufSize);
        buf[n] = '\0';
    }

    return XCPKG_OK;
}
