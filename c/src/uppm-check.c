#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/regex/regex.h"

#include "uppm.h"
#include "xcpkg.h"

int uppm_check_if_the_given_argument_matches_package_name_pattern(const char * arg) {
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

int uppm_check_if_the_given_package_is_available(const char * packageName, const char * uppmHomeDIR, const size_t uppmHomeDIRLength) {
    fprintf(stderr, "uppm_check_if_the_given_package_is_available() 0 packageName=%s\n", packageName);
    int ret = uppm_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    fprintf(stderr, "uppm_check_if_the_given_package_is_available() 1 packageName=%s\n", packageName);
    size_t formulaFilePathCapacity = strlen(packageName) + 41U;
    char   formulaFilePath[formulaFilePathCapacity];

    ret = snprintf(formulaFilePath, formulaFilePathCapacity, "%s/uppm/repos.d/official-core/formula/%s.yml", uppmHomeDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    fprintf(stderr, "uppm_check_if_the_given_package_is_available() 2 packageName=%s\n", packageName);
    struct stat st;

    if (stat(formulaFilePath, &st) == 0) {
        return XCPKG_OK;
    } else {
        return XCPKG_ERROR_PACKAGE_NOT_AVAILABLE;
    }
}

int uppm_check_if_the_given_package_is_installed(const char * packageName, const char * uppmHomeDIR, const size_t uppmHomeDIRLength) {
    int ret = uppm_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    struct stat st;

    size_t packageInstalledDIRLength = uppmHomeDIRLength + strlen(packageName) + 12U;
    char   packageInstalledDIR[packageInstalledDIRLength];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRLength, "%s/installed/%s", uppmHomeDIR, packageName);

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

    size_t receiptFilePathLength = packageInstalledDIRLength + 19U;
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/.uppm/receipt.yml", packageInstalledDIR);

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

int uppm_check_if_the_given_package_is_outdated(const char * packageName, const char * uppmHomeDIR, const size_t uppmHomeDIRLength) {
    UPPMFormula * formula = NULL;
    UPPMReceipt * receipt = NULL;

    int ret = uppm_formula_lookup(uppmHomeDIR, uppmHomeDIRLength, packageName, &formula);

    if (ret != XCPKG_OK) {
        goto finalize;
    }

    ret = uppm_receipt_parse(uppmHomeDIR, uppmHomeDIRLength, packageName, &receipt);

    if (ret != XCPKG_OK) {
        goto finalize;
    }

    if (strcmp(receipt->version, formula->version) == 0) {
        ret = XCPKG_ERROR_PACKAGE_NOT_OUTDATED;
    }

finalize:
    uppm_formula_free(formula);
    uppm_receipt_free(receipt);
    return ret;
}
