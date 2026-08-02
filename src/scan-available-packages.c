#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <dirent.h>

#include "xcpkg.h"

typedef struct {
        const char * targetPlatformName;
        const bool verbose;
        XCPKGPackageCallback packageCallback;
        const void * p1;
              void * p2;
} Payload4;

static int xcpkg_formula_repo_scan_callback(XCPKGFormulaRepo * formulaRepo, const void * p1, void * p2) {
    char * formulaRepoPath  = formulaRepo->path;

    size_t formulaDIRCapacity = strlen(formulaRepoPath) + 10U;
    char   formulaDIR[formulaDIRCapacity];

    int ret = snprintf(formulaDIR, formulaDIRCapacity, "%s/formula", formulaRepoPath);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////

    DIR * dir = opendir(formulaDIR);

    if (dir == NULL) {
        if (errno == ENOENT) {
            return XCPKG_OK;
        } else {
            perror(formulaDIR);
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
            perror(formulaDIR);
            closedir(dir);
            return XCPKG_ERROR;
        }
    }

    char * fileName = dir_entry->d_name;

    //puts(dir_entry->d_name);

    size_t fileNameLength = strlen(fileName);

    if (fileNameLength <= 4U) {
        goto loop;
    }

    char * p = fileName + fileNameLength - 4U;

    if (strcmp(p, ".yml") != 0) {
        goto loop;
    }

    ////////////////////////////////////

    p[0] = '\0';

    ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(fileName);

    if (ret != XCPKG_OK) {
        goto loop;
    }

    ////////////////////////////////////

    p[0] = '/';

    size_t formulaFilePathCapacity = formulaDIRCapacity +fileNameLength + 1U;
    char   formulaFilePath[formulaFilePathCapacity];

    ret = snprintf(formulaFilePath, formulaFilePathCapacity, "%s/%s", formulaFilePath, fileName);

    if (ret < 0) {
        perror(NULL);
        closedir(dir);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////

    p[0] = '\0';

    const Payload4 * payload4 = p1;

    ret = payload4->packageCallback(payload4->targetPlatformName, fileName, formulaFilePath, payload4->verbose, (*((size_t*)p2))++, payload4->p1, payload4->p2);

    if (ret == XCPKG_OK) {
        goto loop;
    }

    closedir(dir);
    return ret;
}

int xcpkg_scan_the_available_packages(const char * targetPlatformName, const bool verbose, XCPKGPackageCallback packageCallback, const void * p1, void * p2) {
    const Payload4 payload = {
        .targetPlatformName = targetPlatformName,
        .verbose = verbose,
        .packageCallback = packageCallback,
        .p1 = p1,
        .p2 = p2
    };

    size_t counter = 0U;

    return xcpkg_formula_repo_scan(xcpkg_formula_repo_scan_callback, &payload, &counter);
}
