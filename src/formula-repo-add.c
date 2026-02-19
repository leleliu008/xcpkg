#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_formula_repo_add(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled) {
    if (formulaRepoName == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoName[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (formulaRepoUrl == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (formulaRepoUrl[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (branchName == NULL || branchName[0] == '\0') {
        branchName = (char*)"master";
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t formulaRepoDIRCapacity = xcpkgHomeDIRLength + strlen(formulaRepoName) + 10U;
    char   formulaRepoDIR[formulaRepoDIRCapacity];

    ret = snprintf(formulaRepoDIR, formulaRepoDIRCapacity, "%s/repos.d/%s", xcpkgHomeDIR, formulaRepoName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (stat(formulaRepoDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            fprintf(stderr, "formula repo '%s' already exist.\n", formulaRepoName);
            return XCPKG_ERROR_FORMULA_REPO_HAS_EXIST;
        } else {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", formulaRepoDIR);
            return XCPKG_ERROR;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    printf("Adding formula repo : %s => %s\n", formulaRepoName, formulaRepoUrl);

    size_t branchNameLength = strlen(branchName);

    size_t remoteRefPathLength = branchNameLength + 12U;
    char   remoteRefPath[remoteRefPathLength];

    ret = snprintf(remoteRefPath, remoteRefPathLength, "refs/heads/%s", branchName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t remoteTrackingRefPathLength = branchNameLength + 21U;
    char   remoteTrackingRefPath[remoteTrackingRefPathLength];

    ret = snprintf(remoteTrackingRefPath, remoteTrackingRefPathLength, "refs/remotes/origin/%s", branchName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_git_sync(sessionDIR, formulaRepoUrl, remoteRefPath, remoteTrackingRefPath, branchName, 0);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];

    ret = snprintf(ts, 11, "%ld", time(NULL));

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_formula_repo_config_write(sessionDIR, formulaRepoUrl, branchName, pinned, enabled, ts, NULL);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoRootDIRLength = xcpkgHomeDIRLength + 9U;
    char   formulaRepoRootDIR[formulaRepoRootDIRLength];

    for (size_t i = 0; i < xcpkgHomeDIRLength; i++) {
        formulaRepoRootDIR[i] = xcpkgHomeDIR[i];
    }

    char * p = formulaRepoRootDIR + xcpkgHomeDIRLength;

    const char * s = "/repos.d";

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (s[i] == '\0') {
            break;
        }
    }

    if (lstat(formulaRepoRootDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(formulaRepoRootDIR) != 0) {
                perror(formulaRepoRootDIR);
                return XCPKG_ERROR;
            }

            if (mkdir(formulaRepoRootDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(formulaRepoRootDIR);
                    return XCPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(formulaRepoRootDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(formulaRepoRootDIR);
                return XCPKG_ERROR;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    if (rename(sessionDIR, formulaRepoDIR) == 0) {
        return XCPKG_OK;
    } else {
        perror(sessionDIR);
        return XCPKG_ERROR;
    }
}
