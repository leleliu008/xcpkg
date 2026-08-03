#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"

/**
 *  xcpkg uninstall <PACKAGE-SPEC>... [-v]
 */
int xcpkg_main_uninstall(int argc, char* argv[]) {
    int packageIndexArray[argc];
    int packageIndexArraySize = 0;

    bool verbose = false;

    char * targetPlatformSpec = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strncmp(argv[i], "--target=", 9) == 0) {
            targetPlatformSpec = &argv[i][9];

            if (targetPlatformSpec[0] == '\0') {
                fprintf(stderr, "--target=<TARGET-PLATFORM-SPEC>, <TARGET-PLATFORM-SPEC> should be a non-empty string.\n");
                return XCPKG_ERROR;
            }
        } else {
            packageIndexArray[packageIndexArraySize] = i;
            packageIndexArraySize++;
        }
    }

    if (packageIndexArraySize == 0) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is unspecified.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    for (int i = 0; i < packageIndexArraySize; i++) {
        const char * package = argv[packageIndexArray[i]];

        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(package, NULL, &packageName, &platformSpec, buf);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s uninstall <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s uninstall <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
            fprintf(stderr, "Usage: %s uninstall <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> does not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s uninstall <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <TARGET-SPEC> does not match pattern A-B-C\n", argv[0]);
        }

        if (platformSpec == NULL) {
            platformSpec = buf;
        }

        ret = xcpkg_uninstall(packageName, platformSpec, verbose);

        if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> does not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package '%s' is not installed.\n", packageName);
        } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}
