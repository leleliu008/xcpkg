#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "../xcpkg.h"

int xcpkg_formula_repo_scan(XCPKGFormulaRepoScanCallback callback, const void * p1, void * p2) {
    const char * xcpkgHomeDIR;
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_get_home_dir(&xcpkgHomeDIR, &xcpkgHomeDIRLength, false);

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

    ///////////////////////////////////////////

    DIR * dir = opendir(xcpkgFormulaRepoDIR);

    if (dir == NULL) {
        if (errno == ENOENT) {
            return XCPKG_OK;
        } else {
            perror(xcpkgFormulaRepoDIR);
            return XCPKG_ERROR;
        }
    }

loop:
    errno = 0;

    struct dirent * dir_entry = readdir(dir);

    if (dir_entry == NULL) {
        if (errno == 0) {
            closedir(dir);
            return XCPKG_OK;
        } else {
            perror(xcpkgFormulaRepoDIR);
            closedir(dir);
            return XCPKG_ERROR;
        }
    }

    //puts(dir_entry->d_name);

    if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
        goto loop;
    }

    size_t formulaRepoPathCapacity = xcpkgFormulaRepoDIRCapacity + strlen(dir_entry->d_name) + 2U;
    char   formulaRepoPath[formulaRepoPathCapacity];

    ret = snprintf(formulaRepoPath, formulaRepoPathCapacity, "%s/%s", xcpkgFormulaRepoDIR, dir_entry->d_name);

    if (ret < 0) {
        perror(NULL);
        closedir(dir);
        return XCPKG_ERROR;
    }

    size_t formulaRepoConfigFilePathCapacity = formulaRepoPathCapacity + strlen(XCPKG_FORMULA_REPO_CONFIG_FILENAME) + 1U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/%s", formulaRepoPath, XCPKG_FORMULA_REPO_CONFIG_FILENAME);

    if (ret < 0) {
        perror(NULL);
        closedir(dir);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaRepoConfigFilePath, &st) != 0) {
        goto loop;
    }

    XCPKGFormulaRepo * formulaRepo = NULL;

    ret = xcpkg_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

    if (ret != XCPKG_OK) {
        closedir(dir);
        xcpkg_formula_repo_free(formulaRepo);
        return ret;
    }

    formulaRepo->name = strdup(dir_entry->d_name);

    if (formulaRepo->name == NULL) {
        closedir(dir);
        xcpkg_formula_repo_free(formulaRepo);
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    formulaRepo->path = strdup(formulaRepoPath);

    if (formulaRepo->path == NULL) {
        closedir(dir);
        xcpkg_formula_repo_free(formulaRepo);
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    ret = callback(formulaRepo, p1, p2);

    xcpkg_formula_repo_free(formulaRepo);

    if (ret == XCPKG_SCAN_BREAK) {
        closedir(dir);
        return XCPKG_OK;
    }

    if (ret == XCPKG_OK) {
        goto loop;
    }

    closedir(dir);
    return ret;
}
