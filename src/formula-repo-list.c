#include <stdio.h>

#include "xcpkg.h"

static int xcpkg_formula_repo_scan_callback(XCPKGFormulaRepo * formulaRepo, const void * payload __attribute__((unused))) {
    printf("---\n");

    return xcpkg_formula_repo_info(formulaRepo);
}

int xcpkg_formula_repo_list() {
    return xcpkg_formula_repo_scan(xcpkg_formula_repo_scan_callback, NULL);
}
