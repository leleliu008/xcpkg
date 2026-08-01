#include <string.h>

#include <unistd.h>

#include "xcpkg.h"

int xcpkg_get_home_dir(const char ** p, size_t * len, bool create) {
    if (p == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    const char * const xcpkgHomeDIR = getenv("XCPKG_HOME");

    if (create) {
        int ret = xcpkg_mkdir_p(xcpkgHomeDIR, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    (*p) = xcpkgHomeDIR;

    if (len != NULL) {
        (*len) = strlen(xcpkgHomeDIR);
    }

    return XCPKG_OK;
}
