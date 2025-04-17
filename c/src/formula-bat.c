#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include "xcpkg.h"

int xcpkg_formula_bat(const char * packageName, const char * targetPlatformName) {
    char formulaFilePath[PATH_MAX];

    int ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * const str = "/uppm/installed/bat/bin/bat";

    size_t batCommandPathCapacity = xcpkgHomeDIRLength + strlen(str) + sizeof(char);
    char   batCommandPath[batCommandPathCapacity];

    ret = snprintf(batCommandPath, batCommandPathCapacity, "%s%s", xcpkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    execlp(batCommandPath, batCommandPath, "--paging=never", formulaFilePath, NULL);
    perror(batCommandPath);

    return XCPKG_ERROR;
}
