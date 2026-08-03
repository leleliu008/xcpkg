#include <string.h>
#include <stdbool.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg about
 */
int xcpkg_main_about(int argc, char* argv[]) {
    bool verbose = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
            break;
        }
    }

    int ret = xcpkg_about(verbose);

    if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        LOG_ERROR1("HOME environment variable is not set.");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        LOG_ERROR1("PATH environment variable is not set.");
    } else if (ret == XCPKG_ERROR) {
        LOG_ERROR1("occurs error.");
    }

    return ret;
}
