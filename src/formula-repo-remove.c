#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_formula_repo_remove(const char * formulaRepoName) {
    if (formulaRepoName == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoName[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(formulaRepoName, "official-core") == 0) {
        fprintf(stderr, "official-core formula repo is not allowed to delete.\n");
        return XCPKG_ERROR;
    }

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t formulaRepoPathCapacity = xcpkgHomeDIRLength + strlen(formulaRepoName) + 10U;
    char   formulaRepoPath[formulaRepoPathCapacity];

    ret = snprintf(formulaRepoPath, formulaRepoPathCapacity, "%s/repos.d/%s", xcpkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaRepoPath, &st) != 0) {
        fprintf(stderr, "formula repo not found: %s\n", formulaRepoName);
        return XCPKG_ERROR;
    }

    size_t formulaRepoConfigFilePathCapacity = formulaRepoPathCapacity + sizeof(XCPKG_FORMULA_REPO_CONFIG_FILPATH_RELATIVE_TO_REPO_ROOT);
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s%s", formulaRepoPath, XCPKG_FORMULA_REPO_CONFIG_FILPATH_RELATIVE_TO_REPO_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (stat(formulaRepoConfigFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return xcpkg_rm_rf(formulaRepoPath, false, false);
    } else {
        fprintf(stderr, "formula repo is broken: %s\n", formulaRepoName);
        return XCPKG_ERROR;
    }
}
