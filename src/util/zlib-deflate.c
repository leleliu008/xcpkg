#include <stdio.h>
#include <string.h>

#include "../core/zlib-flate.h"
#include "../core/log.h"
#include "../xcpkg.h"

#include "../util.h"

/**
 *  xcpkg util zlib-deflate -L <LEVEL> < input/file/path
 */
int xcpkg_util_zlib_deflate(int argc, char* argv[]) {
    int level = 1;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-L") == 0) {
            char * p = argv[i + 1];

            if (p == NULL) {
                fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR;
            }

            if (strlen(p) != 1) {
                fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR;
            }

            if (p[0] < '1' || p[0] > '9') {
                fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
                return XCPKG_ERROR;
            }

            level = atoi(p);

            i++;
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            fprintf(stderr, "Usage: %s %s %s [-L N] (N>=1 && N <=9) , The smaller the N, the faster the speed and the lower the compression ratio.\n", argv[0], argv[1], argv[2]);
            return XCPKG_ERROR;
        }
    }

    return zlib_deflate_file_to_file(stdin, stdout, level);
}
