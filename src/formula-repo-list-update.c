#include <stdio.h>
#include <string.h>

#include "xcpkg.h"

int xcpkg_formula_repo_list_update() {
    XCPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = xcpkg_formula_repo_list(&formulaRepoList);

    if (ret == XCPKG_OK) {
        bool officialCoreIsThere = false;

        for (size_t i = 0U; i < formulaRepoList->size; i++) {
            XCPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];

            if (strcmp(formulaRepo->name, "official-core") == 0) {
                officialCoreIsThere = true;
            }

            ret = xcpkg_formula_repo_sync(formulaRepo);

            if (ret != XCPKG_OK) {
                break;
            }
        }

        xcpkg_formula_repo_list_free(formulaRepoList);

        if (!officialCoreIsThere) {
            const char * const formulaRepoUrl = "https://github.com/leleliu008/xcpkg-formula-repository-official-core";
            ret = xcpkg_formula_repo_add("official-core", formulaRepoUrl, "master", false, true);
        }
    }

    return ret;
}
