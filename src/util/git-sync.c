#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

#include "../util.h"

/**
 *  xcpkg util git-sync <REMOTE-URL> [-v] [-C <REPOSITORY-PATH>] [--ref=<REF>] [--depth=<N>] [-B <CHECKOUT-BRANCH-NAME>]
 */
int xcpkg_util_git_sync(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        fprintf(stderr, "USAGE: %s %s %s <URL> , <URL> is unspecified.\n", argv[0], argv[1], argv[2]);
        return 1;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "USAGE: %s %s %s <URL> , <URL> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return 1;
    }

    const char * remoteUrl = argv[3];
    const char * remoteRef = "HEAD";
    const char * remoteTrackingRef = "refs/remotes/origin/master";
    const char * checkoutBranchName = "master";
    const char * gitRepositoryDIRPath = NULL;

    size_t depth = 1U;

    bool verbose = false;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strncmp(argv[i], "--ref=", 6) == 0) {
            char * p = &argv[i][6];

            if (p[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [--ref=<REF>] , <REF> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            remoteRef = p;
        } else if (strncmp(argv[i], "--depth=", 8) == 0) {
            const char * p = &argv[i][8];

            if (p[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [--depth=<N>] , <N> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            for (int j = 0; p[j] != '\0'; j++) {
                if (p[j] < '0' || p[j] > '9') {
                    fprintf(stderr, "USAGE: %s %s %s <URL> [--depth=<N>] , <N> should be a integer.\n", argv[0], argv[1], argv[2]);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            }

            depth = atoi(p);
        } else if (strcmp(argv[i], "-C") == 0) {
            gitRepositoryDIRPath = argv[++i];

            if (gitRepositoryDIRPath == NULL) {
                fprintf(stderr, "USAGE: %s %s %s <URL> [-C <REPOSITORY-PATH>] , <REPOSITORY-PATH> is unspecified.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            if (gitRepositoryDIRPath[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [-C <REPOSITORY-PATH>] , <REPOSITORY-PATH> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }
        } else if (strcmp(argv[i], "-B") == 0) {
            checkoutBranchName = argv[++i];

            if (checkoutBranchName == NULL) {
                fprintf(stderr, "USAGE: %s %s %s <URL> [-B <CHECKOUT-BRANCH-NAME>] , <CHECKOUT-BRANCH-NAME> is unspecified.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            if (checkoutBranchName[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [-B <CHECKOUT-BRANCH-NAME>] , <CHECKOUT-BRANCH-NAME> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            fprintf(stderr, "USAGE: %s %s %s <URL> [-v]\n", argv[0], argv[1], argv[2]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    }

    return xcpkg_git_sync(gitRepositoryDIRPath, remoteUrl, remoteRef, remoteTrackingRef, checkoutBranchName, depth);
}
