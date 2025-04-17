#include <stdio.h>
#include <string.h>

#include "xcpkg.h"

int xcpkg_inspect_package(const char * package, const char * userSpecifiedTargetPlatformSpec, const char ** packageName, const char ** targetPlatformSpecP, char targetPlatformSpec[]) {
    if (package == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (package[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////

    int slashIndex = -1;

    for (int i = 0; ; i++) {
        if (package[i] == '\0') {
            break;
        }

        if (package[i] == '/') {
            slashIndex = i;
            break;
        }
    }

    //////////////////////////////////////////////

    if (slashIndex == 0) {
        return XCPKG_ERROR_PACKAGE_SPEC_IS_INVALID;
    } else if (slashIndex == -1) {
        int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(package);

        if (ret != XCPKG_OK) {
            return ret;
        }

        if (userSpecifiedTargetPlatformSpec == NULL) {
            const char * XCPKG_DEFAULT_TARGET = getenv("XCPKG_DEFAULT_TARGET");

            if (XCPKG_DEFAULT_TARGET == NULL || XCPKG_DEFAULT_TARGET[0] == '\0') {
                ret = xcpkg_get_native_platform_spec(targetPlatformSpec, 50, NULL);

                if (ret != XCPKG_OK) {
                    return XCPKG_ERROR;
                }
            } else {
                int ret = xcpkg_check_if_the_given_argument_matches_platform_spec_pattern(XCPKG_DEFAULT_TARGET);

                if (ret != XCPKG_OK) {
                    return ret;
                }

                (*targetPlatformSpecP) = XCPKG_DEFAULT_TARGET;
            }
        } else {
            int ret = xcpkg_check_if_the_given_argument_matches_platform_spec_pattern(userSpecifiedTargetPlatformSpec);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }

        (*packageName) = package;
    } else {
        const char * p = package + slashIndex + 1;

        int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(p);

        if (ret != XCPKG_OK) {
            return ret;
        }

        strncpy(targetPlatformSpec, package, slashIndex);

        targetPlatformSpec[slashIndex] = '\0';

        ret = xcpkg_check_if_the_given_argument_matches_platform_spec_pattern(targetPlatformSpec);

        if (ret != XCPKG_OK) {
            return ret;
        }

        (*packageName) = p;
    }

    return XCPKG_OK;
}
