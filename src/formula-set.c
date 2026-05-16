#include <string.h>

#include <unistd.h>
#include <limits.h>

#include "xcpkg.h"

int xcpkg_formula_mapping_set(const char * packageName, const char * targetPlatformName, const char * key, const char * value) {
    if (key == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (value == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (key[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////////////////////////////////////

    char formulaFilePath[PATH_MAX];

    int ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char yqCommandPath[PATH_MAX];

    ret = xcpkg_get_command_path_of_uppm_package("yq", "yq", yqCommandPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t cap = strlen(key) + strlen(value) + 10U;
    char   buf[cap];

    ret = snprintf(buf, cap, ".%s = \"%s\"\n", key, value);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    execl(yqCommandPath, yqCommandPath, "-i", buf, formulaFilePath, NULL);

    perror(yqCommandPath);

    return XCPKG_ERROR;
}
