#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>

#include <sys/stat.h>

#include "xcpkg.h"

static int _scan_dir(const char * targetPlatformSpec) {
    DIR * dir = opendir(targetPlatformSpec);

    if (dir == NULL) {
        if (errno == ENOENT) {
            XCPKG_OK;
        }

        if (errno == ENOTDIR) {
            XCPKG_OK;
        }

        perror(targetPlatformSpec);
        return XCPKG_ERROR;
    }

    size_t targetPlatformSpecLength = strlen(targetPlatformSpec);

loop:
    errno = 0;

    struct dirent * dir_entry = readdir(dir);

    if (dir_entry == NULL) {
        if (errno == 0) {
            closedir(dir);
            return XCPKG_OK;
        } else {
            perror(targetPlatformSpec);
            closedir(dir);
            return XCPKG_ERROR;
        }
    }

    if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
        goto loop;
    }

    size_t packageInstalledDIRCapacity = targetPlatformSpecLength + strlen(dir_entry->d_name) + 2U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    int ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", targetPlatformSpec, dir_entry->d_name);

    if (ret < 0) {
        perror(NULL);
        closedir(dir);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (lstat(packageInstalledDIR, &st) == 0) {
        if (!S_ISLNK(st.st_mode)) {
            goto loop;
        }
    } else {
        goto loop;
    }

    size_t receiptFilePathCapacity = packageInstalledDIRCapacity + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT) + 1U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/%s", packageInstalledDIR, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        closedir(dir);
        return XCPKG_ERROR;
    }

    if (lstat(receiptFilePath, &st) != 0 || !S_ISREG(st.st_mode)) {
        goto loop;
    }

    //printf("%s\n", dir_entry->d_name);

    XCPKGReceipt * receipt = NULL;

    ret = xcpkg_receipt_parse(dir_entry->d_name, targetPlatformSpec, &receipt);

    if (ret != XCPKG_OK) {
        closedir(dir);
        return ret;
    }

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

    XCPKGFormula * formula = NULL;

    ret = xcpkg_formula_load(dir_entry->d_name, targetPlatformName, NULL, &formula);

    if (ret != XCPKG_OK) {
        closedir(dir);
        xcpkg_receipt_free(receipt);
        return ret;
    }

    if (receipt->version != NULL && formula->version != NULL && strcmp(receipt->version, formula->version) != 0) {
        printf("%s %s => %s\n", dir_entry->d_name, receipt->version, formula->version);
    }

    xcpkg_formula_free(formula);
    xcpkg_receipt_free(receipt);

    goto loop;
}

int xcpkg_list_the__outdated_packages(const char * targetPlatformName, const bool verbose) {
    const char * xcpkgHomeDIR;
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_get_home_dir(&xcpkgHomeDIR, &xcpkgHomeDIRLength, false);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t packageInstalledRootDIRCapacity = xcpkgHomeDIRLength + 11U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (chdir(packageInstalledRootDIR) != 0) {
        if (errno == ENOENT) {
            return XCPKG_OK;
        } else {
            perror(NULL);
            return XCPKG_ERROR;
        }
    }

    DIR * dir = opendir(".");

    if (dir == NULL) {
        perror(packageInstalledRootDIR);
        return XCPKG_ERROR;
    }

loop:
    errno = 0;

    struct dirent * dir_entry = readdir(dir);

    if (dir_entry == NULL) {
        if (errno == 0) {
            closedir(dir);
            return XCPKG_OK;
        } else {
            perror(packageInstalledRootDIR);
            closedir(dir);
            return XCPKG_ERROR;
        }
    }

    const char * p = dir_entry->d_name;

    if ((strcmp(p, ".") == 0) || (strcmp(p, "..") == 0)) {
        goto loop;
    }

    ret = xcpkg_check_if_the_given_argument_matches_platform_spec_pattern(p);

    if (ret != XCPKG_OK) {
        goto loop;
    }

    if (targetPlatformName != NULL && targetPlatformName[0] != '\0') {
        if (strncmp(targetPlatformName, p, strlen(p)) != 0) {
            goto loop;
        }
    }

    ret = _scan_dir(p);

    if (ret == XCPKG_OK) {
        goto loop;
    }

    closedir(dir);
    return ret;
}
