#include <stdio.h>

#include "../xcpkg.h"

/**
 *  xcpkg formula-repo-sync <FORMULA-REPO-NAME>
 */
int xcpkg_main_formula_repo_sync(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    return xcpkg_formula_repo_sync_(argv[2]);
}
