#include <string.h>
#include <stdbool.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg search <REGEX> [-v] [--json | --yaml]
 */
int xcpkg_main_search(int argc, char* argv[]) {
    const char * targetPlatformName = NULL;

    char verbose = false;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-p") == 0) {
            targetPlatformName = argv[++i];

            if (targetPlatformName == NULL) {
                LOG_ERROR1("-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> is unspecified.");
                return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
            }

            if (targetPlatformName[0] == '\0') {
                LOG_ERROR1("-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.");
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
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    }

    int ret = xcpkg_search(argv[2], targetPlatformName, verbose);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s search <REGEX>, <REGEX> is not given.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s search <REGEX>, <REGEX> is empty string.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        LOG_ERROR1("HOME environment variable is not set.");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        LOG_ERROR1("PATH environment variable is not set.");
    } else if (ret == XCPKG_ERROR) {
        LOG_ERROR1("occurs error.");
    }

    return ret;
}
