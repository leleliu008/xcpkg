#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg fetch <PACKAGE-NAME> [-v]
 */
int xcpkg_main_fetch(int argc, char* argv[]) {
    const char * targetPlatformName = NULL;

    bool verbose = false;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-p") == 0) {
            targetPlatformName = argv[++i];

            if (targetPlatformName == NULL) {
                fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> is unspecified.\n");
                return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
            }

            if (targetPlatformName[0] == '\0') {
                fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            bool isSupported = false;

            for (int j = 0; supportedTargetPlatformNames[j] != NULL; j++) {
                if (strcmp(targetPlatformName, supportedTargetPlatformNames[j]) == 0) {
                    isSupported = true;
                    break;
                }
            }

            if (!isSupported) {
                LOG_ERROR2("unknown target platform name: ", targetPlatformName);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    }

    int ret = xcpkg_fetch(argv[2], targetPlatformName, verbose);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is unspecified.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> does not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
        fprintf(stderr, "package '%s' is not available for target '%s'\n", argv[2], targetPlatformName);
    } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    return ret;
}
