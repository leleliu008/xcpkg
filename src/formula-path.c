#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_formula_path(const char * packageName, const char * targetPlatformName, char out[]) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    XCPKGFormulaRepoList * formulaRepoList = NULL;

    ret = xcpkg_formula_repo_list(&formulaRepoList);

    if (ret != XCPKG_OK) {
        xcpkg_formula_repo_list_free(formulaRepoList);
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    struct stat st;

    size_t packageNameLength = strlen(packageName);

    for (size_t i = 0U; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t formulaFilePathCapacity = strlen(formulaRepoPath) + packageNameLength + 15U;
        char   formulaFilePath[formulaFilePathCapacity];

        ret = snprintf(formulaFilePath, formulaFilePathCapacity, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (stat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            xcpkg_formula_repo_list_free(formulaRepoList);
            strncpy(out, formulaFilePath, formulaFilePathCapacity);
            return XCPKG_OK;
        }
    }

    xcpkg_formula_repo_list_free(formulaRepoList);
    return XCPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}
