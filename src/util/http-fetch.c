#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

#include "../util.h"

/**
 *  xcpkg util http-fetch <URL> [-v] [--uri=<URI>] [--sha256=<SHA256SUM>] [-o <OUTPUT-PATH>]
 */
int xcpkg_util_http_fetch(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        fprintf(stderr, "USAGE: %s %s %s <URL> , <URL> is unspecified.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "USAGE: %s %s %s <URL> , <URL> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR;
    }

    const char * url = argv[3];
    const char * uri = NULL;

    const char * sha256 = NULL;

    const char * outputPath = NULL;

    bool verbose = false;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strncmp(argv[i], "--uri=", 6) == 0) {
            uri = &argv[i][6];

            if (uri[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [--uri=<URI>] , <URI> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        } else if (strncmp(argv[i], "--sha256=", 9) == 0) {
            sha256 = &argv[i][9];

            if (sha256[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [--sha256=<SHA256SUM>] , <URI> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }

            if (strlen(sha256) != 64U) {
                fprintf(stderr, "USAGE: %s %s %s <URL> [--sha256=<SHA256SUM>] , <URI> should be a 64 length string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        } else if (strcmp(argv[i], "-o") == 0) {
            outputPath = argv[++i];

            if (outputPath == NULL) {
                fprintf(stderr, "USAGE: %s %s %s <URL> [-o <OUTPUT-PATH>] , <OUTPUT-PATH> is unspecified.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }

            if (outputPath[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [-o <OUTPUT-PATH>] , <OUTPUT-PATH> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            fprintf(stderr, "USAGE: %s %s %s <URL> [-v]\n", argv[0], argv[1], argv[2]);
            return XCPKG_ERROR;
        }
    }

    return xcpkg_http_fetch(url, uri, sha256, outputPath, verbose);
}
