#include <errno.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/sysinfo.h"
#include "core/self.h"
#include "core/tar.h"
#include "core/log.h"

#include "xcpkg.h"

int xcpkg_upgrade_self(const bool verbose) {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chdir(sessionDIR) != 0) {
        perror(sessionDIR);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_http_fetch_to_file("https://raw.githubusercontent.com/leleliu008/xcpkg/master/xcpkg-main-latest-release-version", "xcpkg-main-latest-release-version", verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char latestVersionStr[11] = {0};

    ret = xcpkg_read_the_first_n_bytes_of_a_file("xcpkg-main-latest-release-version", 10, latestVersionStr);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    int latestVersionMajor = 0;
    int latestVersionMinor = 0;
    int latestVersionPatch = 0;

    char * p = latestVersionStr;

    ////////////////////////////////////////////////

    for (int i = 0; ; i++) {
        if (p[i] == '.') {
            p[i] = '\0';

            latestVersionMajor = atoi(p);

            p[i] = '.';

            p += i + 1;

            break;
        }

        if (p[i] < '0' || p[i] > '9') {
            fprintf(stderr, "1invalid version format: %s\n", latestVersionStr);
            return XCPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////

    for (int i = 0; ; i++) {
        if (p[i] == '.') {
            p[i] = '\0';

            latestVersionMinor = atoi(p);

            p[i] = '.';

            p += i + 1;

            break;
        }

        if (p[i] < '0' || p[i] > '9') {
            fprintf(stderr, "2invalid version format: %s\n", latestVersionStr);
            return XCPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////

    for (int i = 0; ; i++) {
        if (p[i] == '\0') {
            latestVersionPatch = atoi(p);
            break;
        }

        if (p[i] < '0' || p[i] > '9') {
            fprintf(stderr, "p=%s, p[%d]=%d\n", p, i, p[i]);
            fprintf(stderr, "3invalid version format: %s\n", latestVersionStr);
            return XCPKG_ERROR;
        }
    }

    printf("latestVersionStr=%s\n",   latestVersionStr);
    printf("latestVersionMajor=%d\n", latestVersionMajor);
    printf("latestVersionMinor=%d\n", latestVersionMinor);
    printf("latestVersionPatch=%d\n", latestVersionPatch);

    //////////////////////////////////////////////////////////////////////////////////

    if (latestVersionMajor < XCPKG_VERSION_MAJOR) {
        LOG_SUCCESS1("this software is already the latest version.");
        return XCPKG_OK;
    }

    if (latestVersionMajor == XCPKG_VERSION_MAJOR) {
        if (latestVersionMinor < XCPKG_VERSION_MINOR) {
            LOG_SUCCESS1("this software is already the latest version.");
            return XCPKG_OK;
        }

        if (latestVersionMinor == XCPKG_VERSION_MINOR) {
            if (latestVersionPatch <= XCPKG_VERSION_PATCH) {
                LOG_SUCCESS1("this software is already the latest version.");
                return XCPKG_OK;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////

    char osArch[11] = {0};

    if (sysinfo_arch(osArch, 10) != 0) {
        return XCPKG_ERROR;
    }

    char osVers[11] = {0};

    if (sysinfo_vers(osVers, 10) != 0) {
        return XCPKG_ERROR;
    }

    int osVersMajor = 0;

    for (int i = 0; i < 31; i++) {
        if (osVers[i] == '\0') {
            break;
        }

        if (osVers[i] == '.') {
            osVers[i] = '\0';
            osVersMajor = atoi(osVers);
            break;
        }
    }

    if (osVersMajor < 10) {
        fprintf(stderr, "MacOSX %d.x is not supported.\n", osVersMajor);
        return XCPKG_ERROR;
    }

    if (osVersMajor > 15) {
        osVersMajor = 15;
    }

    size_t tarballFileNameCapacity = 64U;
    char   tarballFileName[tarballFileNameCapacity];

    ret = snprintf(tarballFileName, tarballFileNameCapacity, "xcpkg-%s-macos-%d.0-%s.tar.xz", latestVersionStr, osVersMajor, osArch);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    } else {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t tarballUrlCapacity = tarballFileNameCapacity + strlen(latestVersionStr) + 66U;
    char   tarballUrl[tarballUrlCapacity];

    ret = snprintf(tarballUrl, tarballUrlCapacity, "https://github.com/leleliu008/xcpkg/releases/download/%s/%s", latestVersionStr, tarballFileName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t tarballFilePathLength = sessionDIRLength + tarballFileNameCapacity + 2U;
    char   tarballFilePath[tarballFilePathLength];

    ret = snprintf(tarballFilePath, tarballFilePathLength, "%s/%s", sessionDIR, tarballFileName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_http_fetch_to_file(tarballUrl, tarballFilePath, verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = tar_extract(sessionDIR, tarballFilePath, 0, verbose, 1);

    if (ret != 0) {
        return abs(ret) + XCPKG_ERROR_ARCHIVE_BASE;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t upgradableExecutableFilePathCapacity = sessionDIRLength + 10U;
    char   upgradableExecutableFilePath[upgradableExecutableFilePathCapacity];

    ret = snprintf(upgradableExecutableFilePath, upgradableExecutableFilePathCapacity, "%s/bin/xcpkg", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    char selfPath[PATH_MAX];

    ret = selfpath(selfPath);

    if (ret == -1) {
        perror(NULL);
        ret = XCPKG_ERROR;
        goto finally;
    }

    char * realPath = realpath(selfPath, selfPath);

    if (realPath == NULL) {
        perror(NULL);
        ret = XCPKG_ERROR;
        goto finally;
    }

    //////////////////////////////////////////////////////////////////////////////////

    if (rename(upgradableExecutableFilePath, realPath) != 0) {
        if (errno == EXDEV) {
            if (unlink(realPath) != 0) {
                perror(realPath);
                ret = XCPKG_ERROR;
                goto finally;
            }

            ret = xcpkg_copy_file(upgradableExecutableFilePath, realPath);

            if (ret != XCPKG_OK) {
                goto finally;
            }

            if (chmod(realPath, S_IRWXU) != 0) {
                perror(realPath);
                ret = XCPKG_ERROR;
            }
        } else {
            perror(realPath);
            ret = XCPKG_ERROR;
            goto finally;
        }
    }

finally:
    if (ret == XCPKG_OK) {
        fprintf(stderr, "xcpkg is up to date with version %s\n", latestVersionStr);
    } else {
        fprintf(stderr, "Can't upgrade self. the latest version of executable was downloaded to %s, you can manually replace the current running program with it.\n", upgradableExecutableFilePath);
    }

    return ret;
}
