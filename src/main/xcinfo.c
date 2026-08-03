#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <sys/stat.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg xcinfo [DEVELOPER-DIR]
 */
int xcpkg_main_xcinfo(int argc, char* argv[]) {
    for (int i = 2; i < argc; i++) {
        if (strncmp(argv[i], "--developer-dir=", 15) == 0) {
            const char * developerDIR = &argv[i][15];

            if (developerDIR[0] == '\0') {
                fprintf(stderr, "--developer-dir=<DEVELOPER-DIR>, <DEVELOPER-DIR> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            struct stat st;

            if (stat(developerDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
                if (setenv("DEVELOPER_DIR", developerDIR, 1) != 0) {
                    perror("DEVELOPER_DIR");
                    return XCPKG_ERROR;
                }
            } else {
                fprintf(stderr, "--developer-dir=<DEVELOPER-DIR>, <DEVELOPER-DIR> '%s' directory does not exist.\n", developerDIR);
                return XCPKG_ERROR;
            }
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    }

    XCPKGToolChain toolchain = {0};

    int ret = xcpkg_toolchain_find(&toolchain);

    if (ret != XCPKG_OK) {
        return ret;
    }

    xcpkg_toolchain_dump(&toolchain);
    xcpkg_toolchain_free(&toolchain);

    return XCPKG_OK;
}
