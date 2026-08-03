#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg info <PACKAGE-NAME> [--json | --yaml | <KEY>]
 */
int xcpkg_main_info_available(int argc, char* argv[]) {
    const char * targetPlatformName = NULL;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
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
                return XCPKG_ERROR_ARG_IS_UNKNOWN;
            }
        }
    }

    int ret = xcpkg_show_available_info(argv[2], targetPlatformName, argv[3]);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not given.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is empty string.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> does not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_ARG_IS_UNKNOWN) {
        fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], unknown KEY: %s\n", argv[0], argv[3]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
        fprintf(stderr, "package '%s' is not available for target '%s'\n", argv[2], targetPlatformName);
    } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
        fprintf(stderr, "package '%s' is not installed.\n", argv[2]);
    } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    return ret;
}
