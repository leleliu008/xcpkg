#include <stdio.h>
#include <stdarg.h>

#include <unistd.h>
#include <limits.h>

#include "../xcpkg.h"

int xcpkg_get_dir(char dir[], size_t * len, bool create, const char * fmt, ...) {
    va_list ap;

    va_start(ap, fmt);
    int n = vsnprintf(dir, PATH_MAX, fmt, ap);
    va_end(ap);

    if (n < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////

    if (create) {
        int ret = xcpkg_mkdir_p(dir, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    ////////////////////////////////////////

    if (len != NULL) {
        (*len) = n;
    }

    return XCPKG_OK;
}
