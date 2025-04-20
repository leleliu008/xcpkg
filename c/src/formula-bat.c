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

    char batCommandPath[PATH_MAX];

    ret = xcpkg_get_command_path_of_uppm_package("bat", "bat", batCommandPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    execlp(batCommandPath, batCommandPath, "--paging=never", formulaFilePath, NULL);
    perror(batCommandPath);

    return XCPKG_ERROR;
}
