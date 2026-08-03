#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

#include "../util.h"

/**
 *  xcpkg util rm-rf <FILEPATH> [-v]
 */
int xcpkg_util_rm_rf(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        fprintf(stderr, "USAGE: %s %s %s <FILEPATH> , <FILEPATH> is unspecified.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "USAGE: %s %s %s <FILEPATH> , <FILEPATH> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    bool verbose = false;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            fprintf(stderr, "USAGE: %s %s %s <FILEPATH> [-v]\n", argv[0], argv[1], argv[2]);
            return 1;
        }
    }

    return xcpkg_rm_rf(argv[3], false, verbose);
}
