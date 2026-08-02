#include <stdio.h>

#include "xcpkg.h"

static int xcpkg_scan_the_available_packages_callback(const char * targetPlatformName, const char * packageName, const char * formulaFilePath, const bool verbose, size_t i, const void * p1 __attribute__((unused)), void * p2 __attribute__((unused))) {
    if (verbose) {
        if (i != 0U) {
            printf("\n");
        }

        return xcpkg_print_available_info(packageName, targetPlatformName, NULL, formulaFilePath);
    } else {
        printf("%s\n", packageName);
        return XCPKG_OK;
    }
}

int xcpkg_list_the_available_packages(const char * targetPlatformName, const bool verbose) {
    return xcpkg_scan_the_available_packages(targetPlatformName, verbose, xcpkg_scan_the_available_packages_callback, NULL, NULL);
}
