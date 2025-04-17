#include <string.h>

#include "xcpkg.h"

int xcpkg_get_platform_id_by_name(const char * const platformName, XCPKGPlatformID * const platformID) {
    if (platformName == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (platformName[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (strcmp(platformName, "linux") == 0) {
        *platformID = XCPKGPlatformID_MacOSX;
    } else if (strcmp(platformName, "macos") == 0) {
        *platformID = XCPKGPlatformID_MacOSX;
    } else if (strcmp(platformName, "freebsd") == 0) {
        *platformID = XCPKGPlatformID_MacOSX;
    } else if (strcmp(platformName, "openbsd") == 0) {
        *platformID = XCPKGPlatformID_MacOSX;
    } else if (strcmp(platformName, "netbsd") == 0) {
        *platformID = XCPKGPlatformID_MacOSX;
    } else if (strcmp(platformName, "dragonflybsd") == 0) {
        *platformID = XCPKGPlatformID_MacOSX;
    } else {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    return XCPKG_OK;
}
