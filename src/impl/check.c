#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "../xcpkg.h"

int xcpkg_check_if_the_given_argument_matches_package_name_pattern(const char * arg) {
    if (arg == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (arg[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    for (int i = 0; i < 50; i++) {
        if (arg[i] == '\0') {
            return XCPKG_OK;
        }

        if (arg[i] == '+' || arg[i] == '-' || arg[i] == '_' || arg[i] == '.' || arg[i] == '@') {
            continue;
        }

        if (arg[i] >= '0' && arg[i] <= '9') {
            continue;
        }

        if (arg[i] >= 'A' && arg[i] <= 'Z') {
            continue;
        }

        if (arg[i] >= 'a' && arg[i] <= 'z') {
            continue;
        }

        return XCPKG_ERROR_PACKAGE_NAME_IS_INVALID;
    }

    return XCPKG_ERROR_PACKAGE_NAME_IS_TOOLONG;
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

typedef struct {
    const char * packageName;
    const char * targetPlatformName;
} Payload3;

static int xcpkg_formula_repo_scan_callback(XCPKGFormulaRepo * formulaRepo, const void * p1, void * p2) {
    const Payload3 * Payload3 = p1;

    char * formulaRepoPath = formulaRepo->path;

    size_t formulaFilePathCapacity = strlen(formulaRepoPath) + strlen(Payload3->packageName) + 15U;
    char   formulaFilePath[formulaFilePathCapacity];

    int ret = snprintf(formulaFilePath, formulaFilePathCapacity, "%s/formula/%s.yml", formulaRepoPath, Payload3->packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (lstat(formulaFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return XCPKG_SCAN_BREAK;
    }

    return XCPKG_OK;
}

int xcpkg_check_if_the_given_package_is_available(const char * packageName, const char * targetPlatformName) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    const Payload3 payload = {
        .packageName = packageName,
        .targetPlatformName = targetPlatformName
    };

    return xcpkg_formula_repo_scan(xcpkg_formula_repo_scan_callback, &payload, NULL);
}

int xcpkg_check_if_the_given_package_is_installed(const char * packageName, const char * targetPlatformSpec) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    struct stat st;

    char packageInstalledDIR[PATH_MAX];

    ret = snprintf(packageInstalledDIR, PATH_MAX, "%s/installed/%s/%s", getenv("XCPKG_HOME"), targetPlatformSpec, packageName);

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

    size_t receiptFilePathCapacity = ret + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT) + 2U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/%s", packageInstalledDIR, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

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
