#include <stdio.h>

#include "../xcpkg.h"

/**
 *  xcpkg formula-repo-del <FORMULA-REPO-NAME>
 */
int xcpkg_main_formula_repo_del(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    return xcpkg_formula_repo_remove(argv[2]);
}
