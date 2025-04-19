#include <string.h>
#include <stdbool.h>

#include "xcpkg.h"

int xcpkg_extract_version_from_src_url(const char * url, char versionBuf[], size_t versionBufCapacity) {
    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    int slashIndex = -1;
    int nulIndex = -1;

    for (int i = 0; ; i++) {
        if (url[i] == '\0') {
            nulIndex = i;
            break;
        }

        if (url[i] == '/') {
            slashIndex = i;
        }
    }

    if (slashIndex == -1) {
        return XCPKG_ERROR_INVALID_URL;
    }

    /////////////////////////////

    const char * p = url + slashIndex + 1;

    char buf[nulIndex - slashIndex];

    int dotIndex1 = -1;
    int dotIndex2 = -1;

    for (int i = 0; ; i++) {
        buf[i] = p[i];

        if (p[i] == '\0') {
            break;
        }

        if (p[i] == '.') {
            dotIndex1 = dotIndex2;
            dotIndex2 = i;
        }
    }

    /////////////////////////////

    /**
     * v2023.2.tar.gz
     * V0.3.0.tar.gz
     * R70.11.tar.gz
     * r2.5.tar.gz
     * mksh-R59c.tgz
     * gping-v1.16.1.tar.gz
     * expat-2.6.4.tar.lz
     * tectonic@0.15.0.tar.gz
     * mediainfo_24.05.tar.bz2
     * upx-4.2.4-src.tar.xz
     * v5.7.0-stable.tar.gz
     * v0.4.4-alpha.tar.gz
     * v0.15.0-beta.tar.gz
     * helix-25.01.1-source.tar.xz
     * llvm-project-18.1.8.src.tar.xz
     * libiberty_20230104.orig.tar.xz
     */

    bool gotFileNameExtension = false;


    if (dotIndex1 > 0) {
        const char * q = buf + dotIndex1 + 1;

        if (strcmp(q, "tar.gz") == 0 ||
            strcmp(q, "tar.xz") == 0 ||
            strcmp(q, "tar.lz") == 0 ||
            strcmp(q, "tar.bz2") == 0) {
            buf[dotIndex1] = '\0';
            nulIndex = dotIndex1;
            gotFileNameExtension = true;
        }
    }

    if (!gotFileNameExtension && dotIndex2 > 0) {
        const char * q = buf + dotIndex2 + 1;

        if (strcmp(q, "tgz") == 0 ||
            strcmp(q, "txz") == 0 ||
            strcmp(q, "tlz") == 0 ||
            strcmp(q, "tbz2") == 0 ||
            strcmp(q, "zip") == 0 ||
            strcmp(q, "crate") == 0) {
            buf[dotIndex2] = '\0';
            nulIndex = dotIndex2;
            gotFileNameExtension = true;
        }
    }

    if (!gotFileNameExtension) {
        return XCPKG_OK;
    }

    /////////////////////////////

    int dotIndex = -1;

    for (int i = nulIndex - 1; i >= 0; i--) {
        if (buf[i] == '.' || buf[i] == '-') {
            dotIndex = i;
            break;
        }
    }

    if (dotIndex > 0) {
        const char * q = buf + dotIndex + 1;

        if (strcmp(q, "orig") == 0 || strcmp(q, "src") == 0 || strcmp(q, "source") == 0 || strcmp(q, "stable") == 0 || strcmp(q, "alpha") == 0 || strcmp(q, "beta") == 0) {
            buf[dotIndex] = '\0';
        }
    }

    /////////////////////////////

    int hyphenIndex = -1;

    for (int i = 0; ; i++) {
        if (buf[i] == '\0') {
            break;
        }

        if (buf[i] == '-' || buf[i] == '_' || buf[i] == '@') {
            if (i > 0) {
                int m = i - 1;
                int n = i + 1;

                if (buf[m] >= '0' && buf[m] <= '9' && buf[n] >= '0' && buf[n] <= '9') {
                    buf[i] = '.';
                } else {
                    buf[i] = '-';
                    hyphenIndex = i;
                }
            } else {
                buf[i] = '-';
                hyphenIndex = i;
            }
        }
    }

    /////////////////////////////

    if (hyphenIndex == -1) {
        p = buf;
    } else {
        p = buf + hyphenIndex + 1;
    }

    switch (p[0]) {
        case 'v':
        case 'V':
        case 'r':
        case 'R': p++;
    }

    size_t i;

    for (i = 0U; i < versionBufCapacity; i++) {
        if (p[i] == '\0') {
            break;
        }

        versionBuf[i] = p[i];
    }
    
    versionBuf[i] = '\0';

    return XCPKG_OK;
}
