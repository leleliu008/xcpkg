#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_formula_repo_lookup(const char * formulaRepoName, XCPKGFormulaRepo * * formulaRepoPP) {
    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t formulaRepoNameLength = strlen(formulaRepoName);

    size_t formulaRepoDIRPathCapacity = xcpkgHomeDIRLength + formulaRepoNameLength + 10U;
    char   formulaRepoDIRPath[formulaRepoDIRPathCapacity];

    ret = snprintf(formulaRepoDIRPath, formulaRepoDIRPathCapacity, "%s/repos.d/%s", xcpkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaRepoDIRPath, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", formulaRepoDIRPath);
            return XCPKG_ERROR;
        }
    } else {
        return XCPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    size_t formulaRepoConfigFilePathCapacity = formulaRepoDIRPathCapacity + sizeof(XCPKG_FORMULA_REPO_CONFIG_FILPATH_RELATIVE_TO_REPO_ROOT);
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s%s", formulaRepoDIRPath, XCPKG_FORMULA_REPO_CONFIG_FILPATH_RELATIVE_TO_REPO_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (!((stat(formulaRepoConfigFilePath, &st) == 0) && S_ISREG(st.st_mode))) {
        return XCPKG_ERROR_FORMULA_REPO_NOT_FOUND;
    }

    XCPKGFormulaRepo * formulaRepo = NULL;

    ret = xcpkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

    if (ret != XCPKG_OK) {
        return ret;
    }

    formulaRepo->name = strdup(formulaRepoName);

    if (formulaRepo->name == NULL) {
        xcpkg_formula_repo_free(formulaRepo);
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    formulaRepo->path = strdup(formulaRepoDIRPath);

    if (formulaRepo->path == NULL) {
        xcpkg_formula_repo_free(formulaRepo);
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    (*formulaRepoPP) = formulaRepo;
    return XCPKG_OK;
}
