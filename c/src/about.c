#include <stdio.h>

#include <limits.h>

#include "core/self.h"

#include "xcpkg.h"

int xcpkg_about(const bool verbose) {
    char buf[PATH_MAX];

    int ret = xcpkg_home_dir(buf, NULL);

    if (ret != XCPKG_OK) {
        return ret;
    }

    printf("xcpkg.version : %s\n", XCPKG_VERSION_STRING);
    printf("xcpkg.homedir : %s\n", buf);

    ret = selfpath(buf);

    if (ret == -1) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    printf("xcpkg.exepath : %s\n", buf);

    printf("xcpkg.website : %s\n\n", "https://github.com/leleliu008/xcpkg");
   
    return xcpkg_buildinfo();
}
