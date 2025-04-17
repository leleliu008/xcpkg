#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"
#include "uppm.h"

int xcpkg_tree(const char * packageName, const char * targetPlatformSpec, size_t argc, char* argv[]) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (targetPlatformSpec == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t packageInstalledDIRCapacity = xcpkgHomeDIRLength + strlen(targetPlatformSpec) + strlen(packageName) + 16U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/%s/%s", xcpkgHomeDIR, targetPlatformSpec, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(packageInstalledDIR, &st) != 0) {
        return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathLength = packageInstalledDIRCapacity + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s%s", packageInstalledDIR, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (stat(receiptFilePath, &st) != 0 || (!S_ISREG(st.st_mode))) {
        return XCPKG_ERROR_PACKAGE_IS_BROKEN;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * const str = "/uppm/installed/tree/bin/tree";

    size_t treeCommandPathCapacity = xcpkgHomeDIRLength + strlen(str) + sizeof(char);
    char   treeCommandPath[treeCommandPathCapacity];

    ret = snprintf(treeCommandPath, treeCommandPathCapacity, "%s%s", xcpkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (stat(treeCommandPath, &st) == 0) {
        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", treeCommandPath);
            return XCPKG_ERROR;
        }
    } else {
        ret = uppm_formula_repo_sync_official_core();

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = uppm_install("tree", false, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t n = argc + 5U;
    char*  p[n];

    p[0] = treeCommandPath;
    p[1] = (char*)"--dirsfirst";
    p[2] = (char*)"-a";

    for (size_t i = 0U; i < argc; i++) {
        p[3U + i] = argv[i];
    }

    p[n - 2U] = packageInstalledDIR;
    p[n - 1U]   = NULL;

    execv(treeCommandPath, p);

    perror(treeCommandPath);

    return XCPKG_ERROR;
}
