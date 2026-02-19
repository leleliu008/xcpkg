#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>

#include "xcpkg.h"

static int _list_dir(const char * formulaDIR, const char * targetPlatformName, const bool verbose, XCPKGPackageNameFilter packageNameFilter, const void * payload, size_t * counter) {
    DIR * dir = opendir(formulaDIR);

    if (dir == NULL) {
        perror(formulaDIR);
        return XCPKG_ERROR;
    }

    char * fileName;
    char * fileNameSuffix;
    size_t fileNameLength;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(formulaDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        fileName = dir_entry->d_name;

        fileNameLength = strlen(fileName);

        if (fileNameLength > 4) {
            fileNameSuffix = fileName + fileNameLength - 4;

            if (strcmp(fileNameSuffix, ".yml") == 0) {
                fileName[fileNameLength - 4] = '\0';

                int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(fileName);

                if (ret == XCPKG_OK) {
                    ret = packageNameFilter(fileName, targetPlatformName, verbose, *counter, payload);

                    (*counter)++;

                    if (ret != XCPKG_OK) {
                        closedir(dir);
                        return ret;
                    }
                }
            }
        }
    }
}

int xcpkg_list_the_available_packages(const char * targetPlatformName, const bool verbose, XCPKGPackageNameFilter packageNameFilter, const void * payload) {
    XCPKGFormulaRepoList * formulaRepoList = NULL;

    int ret = xcpkg_formula_repo_list(&formulaRepoList);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    size_t j = 0U;

    struct stat st;

    for (size_t i = 0U; i < formulaRepoList->size; i++) {
        char * formulaRepoPath  = formulaRepoList->repos[i]->path;

        size_t formulaDIR2Capacity = strlen(formulaRepoPath) + 10U;
        char   formulaDIR2[formulaDIR2Capacity];

        ret = snprintf(formulaDIR2, formulaDIR2Capacity, "%s/formula", formulaRepoPath);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (stat(formulaDIR2, &st) == 0 && S_ISDIR(st.st_mode)) {
            ret = _list_dir(formulaDIR2, targetPlatformName, verbose, packageNameFilter, payload, &j);

            if (ret < 0) {
                xcpkg_formula_repo_list_free(formulaRepoList);
                return ret;
            }
        }
    }

    xcpkg_formula_repo_list_free(formulaRepoList);

    return XCPKG_OK;
}

static size_t j = 0U;

static int package_name_filter(const char * packageName, const char * targetPlatformName, const bool verbose, size_t i, const void * payload) {
    if (verbose) {
        if (j != 0U) {
            printf("\n");
        }

        j++;

        return xcpkg_available_info(packageName, targetPlatformName, NULL);
    } else {
        printf("%s\n", packageName);
        return XCPKG_OK;
    }
}

int xcpkg_show_the_available_packages(const char * targetPlatformName, const bool verbose) {
    return xcpkg_list_the_available_packages(targetPlatformName, verbose, package_name_filter, NULL);
}
