#include <stdio.h>
#include <string.h>

#include "../base/sha256sum.h"
#include "../xcpkg.h"

#include "../util.h"

/**
 *  xcpkg util sha256sum <input/file/path>
 *  xcpkg util sha256sum < input/file/path
 */
int xcpkg_util_sha256sum(int argc, char* argv[]) {
    if (argv[3] == NULL || strcmp(argv[3], "-") == 0) {
        char outputBuf[65] = {0};

        if (sha256sum_of_stream(outputBuf, stdin) == 0) {
            printf("%s\n", outputBuf);
            return XCPKG_OK;
        } else {
            perror(NULL);
            return XCPKG_ERROR;
        }
    }

    if (strcmp(argv[3], "-h") == 0 || strcmp(argv[3], "--help") == 0) {
        fprintf(stderr, "Usage: %s %s %s [FILEPATH]\n", argv[0], argv[1], argv[2]);
        return XCPKG_OK;
    } else {
        char outputBuf[65] = {0};

        if (sha256sum_of_file(outputBuf, argv[3]) == 0) {
            printf("%s\n", outputBuf);
            return XCPKG_OK;
        } else {
            return XCPKG_ERROR;
        }
    }
}
