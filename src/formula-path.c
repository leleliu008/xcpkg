#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

static int available = 0;

typedef struct {
          size_t packageNameLength;
    const char * packageName;
    const char * targetPlatformName;
          char * p;
} Payload2;

static int xcpkg_formula_repo_scan_callback(XCPKGFormulaRepo * formulaRepo, const void * payload) {
    const Payload2 * payload2 = payload;

    char * formulaRepoPath = formulaRepo->path;

    size_t formulaFilePathCapacity = strlen(formulaRepoPath) + payload2->packageNameLength + 15U;
    char   formulaFilePath[formulaFilePathCapacity];

    int ret = snprintf(formulaFilePath, formulaFilePathCapacity, "%s/formula/%s.yml", formulaRepoPath, payload2->packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        strncpy(payload2->p, formulaFilePath, formulaFilePathCapacity);
        available = 1;
        return XCPKG_SCAN_BREAK;
    }

    return XCPKG_OK;
}

int xcpkg_formula_path(const char * packageName, const char * targetPlatformName, char out[]) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    const Payload2 payload = {
        .packageNameLength = strlen(packageName),
        .packageName = packageName,
        .targetPlatformName = targetPlatformName,
        .p = out
    };

    ret = xcpkg_formula_repo_scan(xcpkg_formula_repo_scan_callback, &payload);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    if (available) {
        return XCPKG_OK;
    } else {
        return XCPKG_ERROR_PACKAGE_NOT_AVAILABLE;
    }
}
