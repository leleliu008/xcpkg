#include <stdio.h>

#include <unistd.h>

#include "xcpkg.h"

int xcpkg_formula_repo_list_printf() {
    XCPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = xcpkg_formula_repo_list(&formulaRepoList);

    if (ret == XCPKG_OK) {
        for (size_t i = 0; i < formulaRepoList->size; i++) {
            if (i > 0) {
                if (isatty(STDOUT_FILENO)) {
                    printf("\n");
                } else {
                    printf("---\n");
                }
            }

            xcpkg_formula_repo_info(formulaRepoList->repos[i]);
        }

        xcpkg_formula_repo_list_free(formulaRepoList);
    }

    return ret;
}
