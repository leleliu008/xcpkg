#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

#include "../util.h"

/**
 *  xcpkg util uncompress <FILEPATH> [-v] [-C <DIR>] [--strip-components=<N>]
 */
int xcpkg_util_uncompress(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        fprintf(stderr, "USAGE: %s %s %s <FILEPATH> , <FILEPATH> is unspecified.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "USAGE: %s %s %s <FILEPATH> , <FILEPATH> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    const char * unpackDIR = NULL;

    size_t stripComponentNumber = 1;

    bool verbose = false;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-C") == 0) {
            unpackDIR = argv[++i];

            if (unpackDIR == NULL) {
                fprintf(stderr, "USAGE: %s %s %s <FILEPATH> [-C <DIR>] , <DIR> is unspecified.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }

            if (unpackDIR[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <FILEPATH> [-C <DIR>] , <DIR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        } else if (strncmp(argv[i], "--strip-components=", 19) == 0) {
            const char * p = &argv[i][19];

            if (p[0] == '\0') {
                fprintf(stderr, "USAGE: %s %s %s <URL> [--strip-components=<N>] , <N> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }

            for (int j = 0; ; j++) {
                if (p[j] == '\0') {
                    break;
                }

                if (p[j] < '0' || p[j] > '9') {
                    fprintf(stderr, "USAGE: %s %s %s <URL> [--strip-components=<N>] , <N> should be a integer.\n", argv[0], argv[1], argv[2]);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            }

            stripComponentNumber = atoi(p);
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            fprintf(stderr, "USAGE: %s %s %s <URL> [-v]\n", argv[0], argv[1], argv[2]);
            return 1;
        }
    }

    return xcpkg_uncompress(argv[3], unpackDIR, stripComponentNumber, verbose);
}
