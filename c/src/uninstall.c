#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_uninstall(const char * packageName, const char * targetPlatformSpec, const bool verbose) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t packageInstalledRootDIRCapacity = xcpkgHomeDIRLength + strlen(targetPlatformSpec) + 14U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed/%s", xcpkgHomeDIR, targetPlatformSpec);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t packageInstalledLinkDIRCapacity = packageInstalledRootDIRCapacity + strlen(packageName) + 2U;
    char   packageInstalledLinkDIR[packageInstalledLinkDIRCapacity];

    ret = snprintf(packageInstalledLinkDIR, packageInstalledLinkDIRCapacity, "%s/%s", packageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (lstat(packageInstalledLinkDIR, &st) == 0) {
        if (S_ISLNK(st.st_mode)) {
            size_t receiptFilePathCapacity = packageInstalledLinkDIRCapacity + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);
            char   receiptFilePath[receiptFilePathCapacity];

            ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s%s", packageInstalledLinkDIR, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (lstat(receiptFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
                char buf[256] = {0};

                ssize_t readSize = readlink(packageInstalledLinkDIR, buf, 255);

                if (readSize == -1) {
                    perror(packageInstalledLinkDIR);
                    return XCPKG_ERROR;
                } else if (readSize != 64) {
                    // package is broken by other tools?
                    return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
                }

                size_t packageInstalledRealDIRCapacity = packageInstalledRootDIRCapacity + 66U;
                char   packageInstalledRealDIR[packageInstalledRealDIRCapacity];

                ret = snprintf(packageInstalledRealDIR, packageInstalledRealDIRCapacity, "%s/%s", packageInstalledRootDIR, buf);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                if (lstat(packageInstalledRealDIR, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        if (unlink(packageInstalledLinkDIR) == 0) {
                            if (verbose) {
                                printf("rm %s\n", packageInstalledLinkDIR);
                            }
                        } else {
                            perror(packageInstalledLinkDIR);
                            return XCPKG_ERROR;
                        }

                        return xcpkg_rm_rf(packageInstalledRealDIR, false, verbose);
                    } else {
                        // package is broken by other tools?
                        return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
                    }
                } else {
                    // package is broken by other tools?
                    return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
                }
            } else {
                // package is broken. is not installed completely?
                return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
            }
        } else {
            return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
        }
    } else {
        return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }
}
