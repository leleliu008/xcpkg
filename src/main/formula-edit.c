#include <stdio.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg formula-edit <PACKAGE-NAME> [--editor=<EDITOR>]
 */
int xcpkg_main_formula_edit(int argc, char* argv[]) {
    const char * editor = NULL;

    for (int i = 3; i < argc; i++) {
        if (strncmp(argv[i], "--editor=", 9) == 0) {
            editor = &argv[i][9];
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    }

    int ret = xcpkg_formula_edit(argv[2], NULL, editor);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> does not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
        fprintf(stderr, "package '%s' is not available.\n", argv[2]);
    } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    return ret;
}
