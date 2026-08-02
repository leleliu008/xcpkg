#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "core/regex/regex.h"

#include "xcpkg.h"

static int xcpkg_scan_the_available_packages_callback(const char * targetPlatformName, const char * packageName, const char * formulaFilePath, const bool verbose, size_t i, const void * regPattern, void * _) {
    if (regex_matched(packageName, (const char *)regPattern) == 0) {
        if (verbose) {
            if (i != 0U) {
                printf("\n");
            }

            return xcpkg_print_available_info(packageName, targetPlatformName, NULL, formulaFilePath);
        } else {
            puts(packageName);
            return XCPKG_OK;
        }
    } else {
        if (errno == 0) {
            return XCPKG_OK;
        } else {
            perror(NULL);
            return XCPKG_ERROR;
        }
    }
}

int xcpkg_search(const char * regPattern, const char * targetPlatformName, const bool verbose) {
    if (regPattern == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (regPattern[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    return xcpkg_scan_the_available_packages(targetPlatformName, verbose, xcpkg_scan_the_available_packages_callback, regPattern, NULL);
}
