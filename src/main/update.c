#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg update
 */
int xcpkg_main_update(int argc, char* argv[]) {
    int ret = xcpkg_formula_repo_list_update();

    if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        LOG_ERROR1("HOME environment variable is not set.");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        LOG_ERROR1("PATH environment variable is not set.");
    } else if (ret == XCPKG_ERROR) {
        LOG_ERROR1("occurs error.");
    }

    return ret;
}
