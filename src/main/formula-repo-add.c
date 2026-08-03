#include <stdio.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg formula-repo-add  <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]
 */
int xcpkg_main_formula_repo_add(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    if (argv[3] == NULL) {
        fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    int pinned  = 0;
    int enabled = 1;

    const char * branch = NULL;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "--pin") == 0) {
            pinned = 1;
        } else if (strcmp(argv[i], "--unpin") == 0) {
            pinned = 0;
        } else if (strcmp(argv[i], "--enable") == 0) {
            enabled = 1;
        } else if (strcmp(argv[i], "--disable") == 0) {
            enabled = 0;
        } else if (strncmp(argv[i], "--branch=", 9) == 0) {
            if (argv[i][9] == '\0') {
                fprintf(stderr, "--branch=<VALUE>, <VALUE> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            } else {
                branch = &argv[i][9];
            }
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    }

    return xcpkg_formula_repo_add(argv[2], argv[3], branch, pinned, enabled);
}
