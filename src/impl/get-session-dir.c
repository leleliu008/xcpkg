#include <stdio.h>

#include <unistd.h>
#include <limits.h>

#include "../xcpkg.h"

int xcpkg_get_session_dir(char sessionDIR[], size_t * len) {
    int n = snprintf(sessionDIR, PATH_MAX, "%s/run/%d", getenv("XCPKG_HOME"), getpid());

    if (n < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////

    int ret = xcpkg_mkdir_p(sessionDIR, false);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////

    if (len != NULL) {
        (*len) = n;
    }

    return XCPKG_OK;
}
