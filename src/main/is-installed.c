#include <stdio.h>

#include "../xcpkg.h"

/**
 *  xcpkg is-installed <PACKAGE-SPEC>
 */
int xcpkg_main_is_installed(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s is-installed <PACKAGE-SPEC>, <PACKAGE-SPEC> is unspecified.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    } else if (argv[2][0] == '\0') {
        fprintf(stderr, "Usage: %s is-installed <PACKAGE-SPEC>, <PACKAGE-SPEC> must be a non-empty string.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    const char * packageName = NULL;

    const char * platformSpec = NULL;

    char buf[51];

    int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s is-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s is-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s is-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> does not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
        fprintf(stderr, "Usage: %s is-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <TARGET-SPEC> does not match pattern A-B-C\n", argv[0]);
    }

    if (platformSpec == NULL) {
        platformSpec = buf;
    }

    ret = xcpkg_check_if_the_given_package_is_installed(packageName, platformSpec);

    if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> does not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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
