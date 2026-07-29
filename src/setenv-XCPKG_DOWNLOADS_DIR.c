#include <stdio.h>

#include <limits.h>

#include "xcpkg.h"

int xcpkg_setenv_XCPKG_DOWNLOADS_DIR() {
    char p[PATH_MAX];

    const char * const xcpkgDownloadsDIR = getenv("XCPKG_DOWNLOADS_DIR");

    if (xcpkgDownloadsDIR != NULL && xcpkgDownloadsDIR[0] != '\0') {
        return XCPKG_OK;
    }

    const char * const xcpkgHomeDIR = getenv("XCPKG_HOME");

    int ret = snprintf(p, PATH_MAX, "%s/downloads", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("XCPKG_DOWNLOADS_DIR", xcpkgDownloadsDIR, 1) != 0) {
        perror("XCPKG_DOWNLOADS_DIR");
        return XCPKG_ERROR;
    } else {
        return XCPKG_OK;
    }
}
