#include <stdio.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"


/**
 *  xcpkg formula-repo-conf <FORMULA-REPO-NAME> [--url=VALUE --branch=VALUE --pin/--unpin --enable/--disable]
 */
int xcpkg_main_formula_repo_conf(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> [--url=VALUE --branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    if (argv[3] == NULL) {
        fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> [--url=VALUE --branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    int pinned  = -1;
    int enabled = -1;

    const char * branch = NULL;
    const char * url = NULL;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "--pin") == 0) {
            pinned = 1;
        } else if (strcmp(argv[i], "--unpin") == 0) {
            pinned = 0;
        } else if (strcmp(argv[i], "--enable") == 0) {
            enabled = 1;
        } else if (strcmp(argv[i], "--disable") == 0) {
            enabled = 0;
        } else if (strncmp(argv[i], "--url=", 6) == 0) {
            if (argv[i][6] == '\0') {
                fprintf(stderr, "--url=<VALUE>, <VALUE> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            } else {
                url = &argv[i][6];
            }
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

    return xcpkg_formula_repo_config(argv[2], url, branch, pinned, enabled);
}
