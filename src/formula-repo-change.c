#include "xcpkg.h"

int xcpkg_formula_repo_config(const char * formulaRepoName, const char * url, const char * branch, int pinned, int enabled) {
    XCPKGFormulaRepo * formulaRepo = NULL;

    int ret = xcpkg_formula_repo_lookup(formulaRepoName, &formulaRepo);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (url == NULL) {
        url = formulaRepo->url;
    }

    if (branch == NULL) {
        branch = formulaRepo->branch;
    }

    if (pinned == -1) {
        pinned = formulaRepo->pinned;
    }

    if (enabled == -1) {
        enabled = formulaRepo->enabled;
    }

    ret = xcpkg_formula_repo_config_write(formulaRepo->path, url, branch, pinned, enabled, formulaRepo->createdAt, formulaRepo->updatedAt);

    xcpkg_formula_repo_free(formulaRepo);

    return ret;
}
