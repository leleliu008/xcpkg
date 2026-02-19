#include "xcpkg.h"

int xcpkg_extract_filename_from_url(const char * url, char buf[], const size_t bufCapacity) {
    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (buf == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (bufCapacity == 0U) {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    int slashIndex = -1;

    int len = 0;

    for (int i = 0; ; i++) {
        if (url[i] == '?' || url[i] == '\0') {
            len = i;
            break;
        }

        if (url[i] == '/') {
            slashIndex = i;
        }
    }

    const char * p;

    size_t capacity;

    if (slashIndex == -1) {
        p = url;
        capacity = len + 1;
    } else {
        p = url + slashIndex + 1;
        capacity = len - slashIndex;
    }

    size_t n = (capacity > bufCapacity) ? bufCapacity : capacity;

    for (size_t i = 0; i < n; i++) {
        buf[i] = p[i];
    }

    buf[n] = '\0';

    return XCPKG_OK;
}
