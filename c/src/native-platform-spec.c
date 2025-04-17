#include <stdio.h>
#include <string.h>

#include "core/sysinfo.h"

#include "xcpkg.h"

int xcpkg_get_native_platform_spec(char buf[], size_t bufSize, size_t * len) {
    if (buf == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (bufSize == 0U) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    char arch[11] = {0};

    if (sysinfo_arch(arch, 10) != 0) {
        return XCPKG_ERROR;
    }

    char vers[11] = {0};

    if (sysinfo_vers(vers, 10) != 0) {
        return XCPKG_ERROR;
    }

    int dot = -1;

    for (int i = 0; i < 10; i++) {
        if (vers[i] == '.') {
            if (dot == -1) {
                dot = 1;
            } else {
                vers[i] = '\0';
                break;
            }
        }
    }

    char tmpBuf[50];

    int ret = snprintf(tmpBuf, 50, "MacOSX-%s-%s", vers, arch);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t n = (bufSize > (size_t)ret) ? ret : bufSize;

    strncpy(buf, tmpBuf, n);

    buf[n] = '\0';

    if (len != NULL) {
        *len = n;
    }

    return 0;
}
