#include <string.h>

#include "xcpkg.h"

int xcpkg_reinstall(const char * packageName, const char * targetPlatformSpec, const XCPKGInstallOptions * installOptions) {
    XCPKGInstallOptions installOptionsCopy;

    memcpy(&installOptionsCopy, installOptions, sizeof(XCPKGInstallOptions));

    installOptionsCopy.force = true;

    return xcpkg_install(packageName, targetPlatformSpec, &installOptionsCopy);
}
