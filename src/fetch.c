#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

static int xcpkg_fetch_git(const char * packageName, XCPKGFormula * formula, const char * xcpkgDownloadsDIR, size_t xcpkgDownloadsDIRLength) {
    size_t gitRepositoryDIRCapacity = xcpkgDownloadsDIRLength + strlen(packageName) + 6U;
    char   gitRepositoryDIR[gitRepositoryDIRCapacity];

    int ret = snprintf(gitRepositoryDIR, gitRepositoryDIRCapacity, "%s/%s.git", xcpkgDownloadsDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(gitRepositoryDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", gitRepositoryDIR);
            return XCPKG_ERROR;
        }
    } else {
        if (mkdir(gitRepositoryDIR, S_IRWXU) != 0) {
            perror(gitRepositoryDIR);
            return XCPKG_ERROR;
        }
    }

    const char * remoteRef;

    if (formula->git_sha == NULL) {
        remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
    } else {
        remoteRef = formula->git_sha;
    }
    
    return xcpkg_git_sync(gitRepositoryDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master", formula->git_nth);
}

int xcpkg_fetch(const char * packageName, const char * targetPlatformName, const bool verbose) {
    XCPKGFormula * formula = NULL;

    int ret = xcpkg_formula_load(packageName, targetPlatformName, NULL, &formula);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t xcpkgDownloadsDIRCapacity = xcpkgHomeDIRLength + 11U;
    char   xcpkgDownloadsDIR[xcpkgDownloadsDIRCapacity];

    ret = snprintf(xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "%s/downloads", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(xcpkgDownloadsDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(xcpkgDownloadsDIR) != 0) {
                perror(xcpkgDownloadsDIR);
                xcpkg_formula_free(formula);
                return XCPKG_ERROR;
            }

            if (mkdir(xcpkgDownloadsDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(xcpkgDownloadsDIR);
                    xcpkg_formula_free(formula);
                    return XCPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(xcpkgDownloadsDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(xcpkgDownloadsDIR);
                xcpkg_formula_free(formula);
                return XCPKG_ERROR;
            }
        }
    }

    ///////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        ret = xcpkg_fetch_git(packageName, formula, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity);
    } else {
        if (formula->src_is_dir) {
            fprintf(stderr, "src_url is point to local dir, so no need to fetch.\n");
        } else {
            ret = xcpkg_http_fetch_then_unpack(formula->src_url, formula->src_uri, formula->src_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, NULL, 0, verbose);
        }
    }

    if (ret != XCPKG_OK) {
        goto finalize;
    }

    if (formula->fix_url != NULL) {
        ret = xcpkg_http_fetch_then_unpack(formula->fix_url, formula->fix_uri, formula->fix_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, NULL, 0, verbose);

        if (ret != XCPKG_OK) {
            goto finalize;
        }
    }

    if (formula->res_url != NULL) {
        ret = xcpkg_http_fetch_then_unpack(formula->res_url, formula->res_uri, formula->res_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, NULL, 0, verbose);

        if (ret != XCPKG_OK) {
            goto finalize;
        }
    }

finalize:
    xcpkg_formula_free(formula);
    return ret;
}
