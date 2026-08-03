#include "../xcpkg.h"

/**
 *  xcpkg show <PACKAGE-SPEC> [--json | --yaml | <KEY>]
 */
int xcpkg_main_info_installed(int argc, char* argv[]) {
    const char * packageName = NULL;

    const char * platformSpec = NULL;

    char buf[51];

    int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s show <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s show <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s show <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> does not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
        fprintf(stderr, "Usage: %s show <PACKAGE-SPEC> [KEY], <TARGET-SPEC> does not match pattern A-B-C\n", argv[0]);
    }

    if (platformSpec == NULL) {
        platformSpec = buf;
    }

    ret = xcpkg_show_installed_info(packageName, platformSpec, argv[3]);

    if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
        fprintf(stderr, "package '%s' is not available for target '%s'\n", argv[2], platformSpec);
    } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
        fprintf(stderr, "package '%s' is not installed.\n", argv[2]);
    } else if (ret == XCPKG_ERROR_ARG_IS_UNKNOWN) {
        fprintf(stderr, "Usage: %s show <PACKAGE-NAME> [KEY], unknown KEY: %s\n", argv[0], argv[3]);
    } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    return ret;
}
