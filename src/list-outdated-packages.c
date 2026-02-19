#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "xcpkg.h"

static int _list_dir(const char * targetPlatformSpec, const char * packageInstalledRootDIR, const size_t packageInstalledRootDIRCapacity, const bool verbose) {

    size_t targetPlatformSpecLength = strlen(targetPlatformSpec);

    struct stat st;

    if (!((stat(targetPlatformSpec, &st) == 0) && S_ISDIR(st.st_mode))) {
        return XCPKG_OK;
    }

    DIR * dir = opendir(targetPlatformSpec);

    if (dir == NULL) {
        perror(targetPlatformSpec);
        return XCPKG_ERROR;
    }

    for (;;) {
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
            continue;
        }

        size_t packageInstalledDIRCapacity = targetPlatformSpecLength + strlen(dir_entry->d_name) + 2U;
        char   packageInstalledDIR[packageInstalledDIRCapacity];

        int ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", targetPlatformSpec, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return XCPKG_ERROR;
        }

        if (lstat(packageInstalledDIR, &st) == 0) {
            if (!S_ISLNK(st.st_mode)) {
                continue;
            }
        } else {
            continue;
        }

        size_t receiptFilePathCapacity = packageInstalledDIRCapacity + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);
        char   receiptFilePath[receiptFilePathCapacity];

        ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s%s", packageInstalledDIR, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return XCPKG_ERROR;
        }

        if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
            //printf("%s\n", dir_entry->d_name);

            XCPKGReceipt * receipt = NULL;

            ret = xcpkg_receipt_parse(dir_entry->d_name, targetPlatformSpec, &receipt);

            if (ret != XCPKG_OK) {
                closedir(dir);
                xcpkg_receipt_free(receipt);
                receipt = NULL;
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

            if (ret == XCPKG_OK) {
                if (strcmp(receipt->version, formula->version) != 0) {
                    printf("%s %s => %s\n", dir_entry->d_name, receipt->version, formula->version);
                }
            }

            xcpkg_formula_free(formula);
            xcpkg_receipt_free(receipt);

            formula = NULL;
            receipt = NULL;
        }
    }
}

int xcpkg_list_the__outdated_packages(const char * targetPlatformName, const bool verbose) {
    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

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

    for (;;) {
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
            continue;
        }

        ret = xcpkg_check_if_the_given_argument_matches_platform_spec_pattern(p);

        if (ret != XCPKG_OK) {
            continue;
        }

        if (targetPlatformName != NULL && targetPlatformName[0] != '\0') {
            if (strncmp(targetPlatformName, p, strlen(p)) != 0) {
                continue;
            }
        }

        ret = _list_dir(p, packageInstalledRootDIR, packageInstalledRootDIRCapacity, verbose);

        if (ret != XCPKG_OK) {
            closedir(dir);
            return ret;
        }
    }
}
