#include <string.h>

#include "xcpkg.h"

int xcpkg_extract_version_from_git_ref(const char * ref, char versionBuf[], size_t versionBufCapacity) {
    if (ref == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (ref[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    int slashIndex = -1;
    int nulIndex = -1;

    for (int i = 0; ; i++) {
        if (ref[i] == '\0') {
            nulIndex = i;
            break;
        }

        if (ref[i] == '/') {
            slashIndex = i;
        }
    }

    size_t capcity;

    const char * p;

    if (slashIndex == -1) {
        p = ref;
        capcity = nulIndex + 1;
    } else {
        p = ref + slashIndex + 1;
        capcity = nulIndex - slashIndex;
    }

    if (p[0] == 'v') {
        p++;
        capcity--;
    }

    for (int i = 0; p[i] != '\0'; i++) {
        if (p[i] == '.') {
            continue;
        }

        if (p[i] < '0' && p[i] > '9') {
            return XCPKG_OK;
        }
    }

    strncpy(versionBuf, p, (capcity > versionBufCapacity ) ? versionBufCapacity : capcity);

    return XCPKG_OK;
}
