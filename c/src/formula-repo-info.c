#include <time.h>
#include <stdio.h>

#include <unistd.h>

#include "core/log.h"

#include "xcpkg.h"

int xcpkg_formula_repo_info_(const char * formulaRepoName) {
    XCPKGFormulaRepo * formulaRepo = NULL;

    int ret = xcpkg_formula_repo_lookup(formulaRepoName, &formulaRepo);

    if (ret != XCPKG_OK) {
        return ret;
    }

    xcpkg_formula_repo_info(formulaRepo);
    xcpkg_formula_repo_free(formulaRepo);

    return XCPKG_OK;
}

int xcpkg_formula_repo_info(XCPKGFormulaRepo * formulaRepo) {
    if (formulaRepo == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (isatty(STDOUT_FILENO)) {
        printf("name: %s%s%s\n", COLOR_GREEN, formulaRepo->name, COLOR_OFF);
    } else {
        printf("name: %s\n", formulaRepo->name);
    }

    printf("path: %s\n", formulaRepo->path);
    printf("url:  %s\n", formulaRepo->url);
    printf("branch: %s\n", formulaRepo->branch);
    printf("pinned: %s\n", formulaRepo->pinned ? "yes" : "no");
    printf("enabled: %s\n", formulaRepo->enabled ? "yes" : "no");

    ////////////////////////////////////////////////////////////

    time_t tt = (time_t)atol(formulaRepo->createdAt);
    struct tm *tms = localtime(&tt);

    char buff[26] = {0};
    strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

    buff[24] = buff[23];
    buff[23] = buff[22];
    buff[22] = ':';

    printf("created: %s\n", buff);

    ////////////////////////////////////////////////////////////

    if (formulaRepo->updatedAt != NULL) {
        time_t tt = (time_t)atol(formulaRepo->updatedAt);
        struct tm *tms = localtime(&tt);

        char buff[26] = {0};
        strftime(buff, 26, "%Y-%m-%d %H:%M:%S%z", tms);

        buff[24] = buff[23];
        buff[23] = buff[22];
        buff[22] = ':';

        printf("updated: %s\n", buff);
    }

    return XCPKG_OK;
}
