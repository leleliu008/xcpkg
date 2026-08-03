#include <stdio.h>

#include "../xcpkg.h"

/**
 *  xcpkg formula-set <PACKAGE-NAME> <KEY> <VALUE>
 */
int xcpkg_main_formula_set(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> <KEY> <VALUE>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (argv[2][0] == '\0') {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> <KEY> <VALUE>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (argv[3] == NULL) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> <KEY> <VALUE>, <KEY> is not given.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> <KEY> <VALUE>, <KEY> is empty string.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (argv[4] == NULL) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> <KEY> <VALUE>, <VALUE> is not given.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (argv[4][0] == '\0') {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> <KEY> <VALUE>, <VALUE> is empty string.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    int ret = xcpkg_formula_mapping_set(argv[2], NULL, argv[3], argv[4]);

    if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
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
