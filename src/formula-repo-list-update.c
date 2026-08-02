#include <string.h>

#include "xcpkg.h"

static int officialCoreIsThere = 0;

static int xcpkg_formula_repo_scan_callback(XCPKGFormulaRepo * formulaRepo, const void * payload __attribute__((unused))) {
    if (strcmp(formulaRepo->name, "official-core") == 0) {
        officialCoreIsThere = 1;
    }

    int ret = xcpkg_formula_repo_sync(formulaRepo);

    if (ret != XCPKG_OK) {
        return ret;
    }

    return XCPKG_OK;
}

int xcpkg_formula_repo_list_update() {
    int ret = xcpkg_formula_repo_scan(xcpkg_formula_repo_scan_callback, NULL);

    if (ret == XCPKG_OK) {
        return ret;
    }

    if (officialCoreIsThere == 1) {
        return XCPKG_OK;
    }

    return xcpkg_formula_repo_add("official-core", "https://github.com/leleliu008/xcpkg-formula-repository-official-core", "master", false, true);
}
