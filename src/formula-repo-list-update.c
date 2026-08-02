#include <string.h>

#include "xcpkg.h"

static int xcpkg_formula_repo_scan_callback(XCPKGFormulaRepo * formulaRepo, const void * p1 __attribute__((unused)), void * p2) {
    if (strcmp(formulaRepo->name, "official-core") == 0) {
        (*((int*)p2)) = 1;
    }

    int ret = xcpkg_formula_repo_sync(formulaRepo);

    if (ret != XCPKG_OK) {
        return ret;
    }

    return XCPKG_OK;
}

int xcpkg_formula_repo_list_update() {
    int officialCoreIsThere = 0;

    int ret = xcpkg_formula_repo_scan(xcpkg_formula_repo_scan_callback, NULL, &officialCoreIsThere);

    if (ret == XCPKG_OK) {
        return ret;
    }

    if (officialCoreIsThere == 1) {
        return XCPKG_OK;
    }

    return xcpkg_formula_repo_add("official-core", "https://github.com/leleliu008/xcpkg-formula-repository-official-core", "master", false, true);
}
