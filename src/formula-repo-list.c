#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_formula_repo_list(XCPKGFormulaRepoList * * out) {
    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t xcpkgFormulaRepoDIRCapacity = xcpkgHomeDIRLength + 9U;
    char   xcpkgFormulaRepoDIR[xcpkgFormulaRepoDIRCapacity];

    ret = snprintf(xcpkgFormulaRepoDIR, xcpkgFormulaRepoDIRCapacity, "%s/repos.d", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if ((stat(xcpkgFormulaRepoDIR, &st) != 0) || (!S_ISDIR(st.st_mode))) {
        XCPKGFormulaRepoList* formulaRepoList = (XCPKGFormulaRepoList*)calloc(1, sizeof(XCPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            return XCPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            (*out) = formulaRepoList;
            return XCPKG_OK;
        }
    }

    DIR * dir = opendir(xcpkgFormulaRepoDIR);

    if (dir == NULL) {
        perror(xcpkgFormulaRepoDIR);
        return XCPKG_ERROR;
    }

    size_t capcity = 5;

    XCPKGFormulaRepoList * formulaRepoList = NULL;

    ret = XCPKG_OK;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                break;
            } else {
                perror(xcpkgFormulaRepoDIR);
                closedir(dir);
                xcpkg_formula_repo_list_free(formulaRepoList);
                return XCPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t formulaRepoPathCapacity = xcpkgFormulaRepoDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   formulaRepoPath[formulaRepoPathCapacity];

        ret = snprintf(formulaRepoPath, formulaRepoPathCapacity, "%s/%s", xcpkgFormulaRepoDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            xcpkg_formula_repo_list_free(formulaRepoList);
            return XCPKG_ERROR;
        }

        size_t formulaRepoConfigFilePathCapacity = formulaRepoPathCapacity + strlen(XCPKG_FORMULA_REPO_CONFIG_FILENAME) + 1U;
        char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

        ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/%s", formulaRepoPath, XCPKG_FORMULA_REPO_CONFIG_FILENAME);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            xcpkg_formula_repo_list_free(formulaRepoList);
            return XCPKG_ERROR;
        }

        if (stat(formulaRepoConfigFilePath, &st) != 0) {
            continue;
        }

        XCPKGFormulaRepo * formulaRepo = NULL;

        ret = xcpkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

        if (ret != XCPKG_OK) {
            xcpkg_formula_repo_free(formulaRepo);
            xcpkg_formula_repo_list_free(formulaRepoList);
            goto finalize;
        }

        if (formulaRepoList == NULL) {
            formulaRepoList = (XCPKGFormulaRepoList*)calloc(1, sizeof(XCPKGFormulaRepoList));

            if (formulaRepoList == NULL) {
                xcpkg_formula_repo_free(formulaRepo);
                xcpkg_formula_repo_list_free(formulaRepoList);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            formulaRepoList->repos = (XCPKGFormulaRepo**)calloc(capcity, sizeof(XCPKGFormulaRepo*));

            if (formulaRepoList->repos == NULL) {
                xcpkg_formula_repo_free(formulaRepo);
                xcpkg_formula_repo_list_free(formulaRepoList);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }
        }

        if (capcity == formulaRepoList->size) {
            capcity += 5;
            XCPKGFormulaRepo ** formulaRepoArray = (XCPKGFormulaRepo**)realloc(formulaRepoList->repos, capcity * sizeof(XCPKGFormulaRepo*));

            if (formulaRepoArray == NULL) {
                xcpkg_formula_repo_free(formulaRepo);
                xcpkg_formula_repo_list_free(formulaRepoList);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            } else {
                formulaRepoList->repos = formulaRepoArray;
            }
        }

        formulaRepo->name = strdup(dir_entry->d_name);

        if (formulaRepo->name == NULL) {
            xcpkg_formula_repo_free(formulaRepo);
            xcpkg_formula_repo_list_free(formulaRepoList);
            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }

        formulaRepo->path = strdup(formulaRepoPath);

        if (formulaRepo->path == NULL) {
            xcpkg_formula_repo_free(formulaRepo);
            xcpkg_formula_repo_list_free(formulaRepoList);
            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }

        formulaRepoList->repos[formulaRepoList->size] = formulaRepo;
        formulaRepoList->size += 1;
    }

    if (formulaRepoList == NULL) {
        formulaRepoList = (XCPKGFormulaRepoList*)calloc(1, sizeof(XCPKGFormulaRepoList));

        if (formulaRepoList == NULL) {
            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
            goto finalize;
        }
    }

finalize:
    if (ret == XCPKG_OK) {
        (*out) = formulaRepoList;
    } else {
        xcpkg_formula_repo_list_free(formulaRepoList);
    }

    closedir(dir);

    return ret;
}

void xcpkg_formula_repo_list_free(XCPKGFormulaRepoList * formulaRepoList) {
    if (formulaRepoList == NULL) {
        return;
    }

    if (formulaRepoList->repos == NULL) {
        free(formulaRepoList);
        return;
    }

    for (size_t i = 0; i < formulaRepoList->size; i++) {
        XCPKGFormulaRepo * formulaRepo = formulaRepoList->repos[i];
        xcpkg_formula_repo_free(formulaRepo);
    }

    free(formulaRepoList->repos);
    formulaRepoList->repos = NULL;

    free(formulaRepoList);
}
