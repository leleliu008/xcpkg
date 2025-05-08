#include <unistd.h>
#include <limits.h>

#include "xcpkg.h"

int xcpkg_formula_bat(const char * packageName, const char * targetPlatformName, size_t argc, char* argv[]) {
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

    size_t argn = argc + 3U;
    char*  args[argn];

    args[0] = batCommandPath;

    char ** p = &args[1];

    for (size_t i = 0U; i < argc; i++) {
        p[i] = argv[i];
    }

    args[argn - 2U] = formulaFilePath;
    args[argn - 1U] = NULL;

    execv(batCommandPath, args);

    perror(batCommandPath);

    return XCPKG_ERROR;
}
