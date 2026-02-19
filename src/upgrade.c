#include <string.h>

#include "xcpkg.h"

int xcpkg_upgrade(const char * packageName, const char * targetPlatformSpec, const XCPKGInstallOptions * installOptions) {
    int ret = xcpkg_check_if_the_given_package_is_outdated(packageName, targetPlatformSpec);

    if (ret != XCPKG_OK) {
        return ret;
    }

    XCPKGInstallOptions installOptions2;

    memcpy(&installOptions2, installOptions, sizeof(XCPKGInstallOptions));

    installOptions2.force = true;

    return xcpkg_install(packageName, targetPlatformSpec, &installOptions2);
}
