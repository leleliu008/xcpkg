#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/regex/regex.h"

#include "xcpkg.h"

int xcpkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg) {
    if (arg == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (arg[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (regex_matched(arg, XCPKG_PACKAGE_NAME_PATTERN) == 0) {
        return XCPKG_OK;
    } else {
        if (errno == 0) {
            return XCPKG_ERROR_ARG_IS_INVALID;
        } else {
            perror(NULL);
            return XCPKG_ERROR;
        }
    }
}

int xcpkg_check_if_the_given_argument_matches_platform_spec_pattern(const char * p) {
    if (p == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (p[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    for (int i = 0; ; i++) {
        if (p[i] == '\0') {
            return XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID;
        }

        if (p[i] == '-') {
            int support = 0;

            for (int j = 0; supportedTargetPlatformNames[j] != NULL; j++) {
                if (strncmp(p, supportedTargetPlatformNames[j], i) == 0) {
                    support = 1;
                    break;
                }
            }

            if (support == 0) {
                return XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID;
            } else {
                p += i + 1;
                break;
            }
        }
    }

    //////////////////////////////////////////////

    int k = -1;

    for (int i = 0; ; i++) {
        if (p[i] == '\0' || p[i] == '-') {
            return XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID;
        }

        if (p[i] == '.') {
            k = i + 1;
            break;
        }

        if (p[i] < '0' || p[i] > '9') {
            return XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID;
        }
    }

    for (int i = k; ; i++) {
        if (p[i] == '\0') {
            return XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID;
        }

        if (p[i] == '-') {
            p += i + 1;
            break;
        }

        if (p[i] < '0' || p[i] > '9') {
            return XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID;
        }
    }

    //////////////////////////////////////////////

    for (int i = 0; ; i++) {
        if (p[i] == '-') {
            return XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID;
        }

        if (p[i] == '\0') {
            break;
        }
    }

    return XCPKG_OK;
}

int xcpkg_check_if_the_given_package_is_available(const char * packageName, const char * targetPlatformName) {
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

    struct stat st;

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        char * formulaRepoPath = formulaRepoList->repos[i]->path;

        size_t formulaFilePathLength = strlen(formulaRepoPath) + strlen(packageName) + 15U;
        char   formulaFilePath[formulaFilePathLength];

        ret = snprintf(formulaFilePath, formulaFilePathLength, "%s/formula/%s.yml", formulaRepoPath, packageName);

        if (ret < 0) {
            perror(NULL);
            xcpkg_formula_repo_list_free(formulaRepoList);
            return XCPKG_ERROR;
        }

        if (lstat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            xcpkg_formula_repo_list_free(formulaRepoList);
            return XCPKG_OK;
        }
    }

    xcpkg_formula_repo_list_free(formulaRepoList);
    return XCPKG_ERROR_PACKAGE_NOT_AVAILABLE;
}

int xcpkg_check_if_the_given_package_is_installed(const char * packageName, const char * targetPlatformSpec) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    struct stat st;

    size_t packageInstalledDIRCapacity = xcpkgHomeDIRLength + strlen(targetPlatformSpec) + strlen(packageName) + 15U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/%s/%s", xcpkgHomeDIR, targetPlatformSpec, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (lstat(packageInstalledDIR, &st) == 0) {
        if (!S_ISLNK(st.st_mode)) {
            return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else {
        return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    size_t receiptFilePathCapacity = packageInstalledDIRCapacity + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s%s", packageInstalledDIR, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (lstat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            return XCPKG_OK;
        } else {
            return XCPKG_ERROR_PACKAGE_IS_BROKEN;
        }
    } else {
        return XCPKG_ERROR_PACKAGE_IS_BROKEN;
    }
}

int xcpkg_check_if_the_given_package_is_outdated(const char * packageName, const char * targetPlatformSpec) {
    XCPKGFormula * formula = NULL;
    XCPKGReceipt * receipt = NULL;

    char targetPlatformName[51];

    for (int i = 0; i < 50; i++) {
        if (targetPlatformSpec[i] == '\0') {
            return XCPKG_ERROR_ARG_IS_INVALID;
        }

        if (targetPlatformSpec[i] == '-') {
            targetPlatformName[i] = '\0';
            break;
        }

        targetPlatformName[i] = targetPlatformSpec[i];
    }

    int ret = xcpkg_formula_load(packageName, targetPlatformName, NULL, &formula);

    if (ret != XCPKG_OK) {
        goto finalize;
    }

    ret = xcpkg_receipt_parse(packageName, targetPlatformSpec, &receipt);

    if (ret != XCPKG_OK) {
        goto finalize;
    }

    if (strcmp(receipt->version, formula->version) == 0) {
        ret = XCPKG_ERROR_PACKAGE_NOT_OUTDATED;
    }

finalize:
    xcpkg_formula_free(formula);
    xcpkg_receipt_free(receipt);
    return ret;
}
