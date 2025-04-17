#include <errno.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include "core/sysinfo.h"
#include "core/tar.h"

#include "xcpkg.h"


int xcpkg_setup(const bool verbose) {
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

    ret = xcpkg_http_fetch_to_file("https://raw.githubusercontent.com/leleliu008/xcpkg/master/xcpkg-core-latest-release-version", "xcpkg-core-latest-release-version", verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char xcpkgCoreLatestVersion[11] = {0};

    ret = xcpkg_read_the_first_n_bytes_of_a_file("xcpkg-core-latest-release-version", 10, xcpkgCoreLatestVersion);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char osArch[10] = {0};

    if (sysinfo_arch(osArch, 10) != 0) {
        return XCPKG_ERROR;
    }

    char osVers[10] = {0};

    if (sysinfo_vers(osVers, 10) != 0) {
        return XCPKG_ERROR;
    }

    int osVersMajor = 0;

    for (int i = 0; i < 10; i++) {
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

    size_t xcpkgCoreTarballFilenameCapacity = 100U;
    char   xcpkgCoreTarballFilename[xcpkgCoreTarballFilenameCapacity];

    ret = snprintf(xcpkgCoreTarballFilename, xcpkgCoreTarballFilenameCapacity, "xcpkg-core-%s-macos-%d.0-%s.tar.xz", xcpkgCoreLatestVersion, osVersMajor, osArch);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    } else {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t urlCapacity = 256U;
    char   url[urlCapacity];

    ret = snprintf(url, urlCapacity, "https://github.com/leleliu008/xcpkg/releases/download/xcpkg-core-%s/%s", xcpkgCoreLatestVersion, xcpkgCoreTarballFilename);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_http_fetch_to_file(url, xcpkgCoreTarballFilename, verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = tar_extract(sessionDIR, xcpkgCoreTarballFilename, ARCHIVE_EXTRACT_TIME, verbose, 1);

    if (ret != 0) {
        return abs(ret) + XCPKG_ERROR_ARCHIVE_BASE;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_http_fetch_to_file("https://raw.githubusercontent.com/leleliu008/xcpkg/master/core/xcpkg-install", "xcpkg-install", verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_http_fetch_to_file("https://curl.se/ca/cacert.pem", "cacert.pem", verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    if (chdir(xcpkgHomeDIR) != 0) {
        perror(xcpkgHomeDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    const char * const xcpkgCoreDIR = "core";

    for (;;) {
        if (rename(sessionDIR, xcpkgCoreDIR) == 0) {
            return XCPKG_OK;
        } else {
            if (errno == ENOTEMPTY || errno == EEXIST) {
                if (lstat(xcpkgCoreDIR, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = xcpkg_rm_rf(xcpkgCoreDIR, false, verbose);

                        if (ret != XCPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(xcpkgCoreDIR) != 0) {
                            perror(xcpkgCoreDIR);
                            return XCPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(xcpkgCoreDIR);
                return XCPKG_ERROR;
            }
        }
    }
}
