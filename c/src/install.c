#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/printenv.h"
#include "core/sysinfo.h"
#include "core/self.h"
#include "core/exe.h"
#include "core/tar.h"
#include "core/log.h"

#include "sha256sum.h"
#include "xcpkg.h"
#include "uppm.h"

typedef struct {
    const char * name;
    const char * value;
} KV;

typedef struct {
    const char * name;
    bool         value;
} KB;

typedef struct {
    const char * name;
    size_t       value;
} KU;


static int download_via_http(const char * url, const char * uri, const char * expectedSHA256SUM, const char * downloadDIR, size_t downloadDIRLength, const char * unpackDIR, size_t unpackDIRLength, const bool verbose) {
    char fileType[21] = {0};

    int ret = xcpkg_examine_filetype_from_url(url, fileType, 20);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t fileNameCapacity = strlen(expectedSHA256SUM) + strlen(fileType) + 1U;
    char   fileName[fileNameCapacity];

    ret = snprintf(fileName, fileNameCapacity, "%s%s", expectedSHA256SUM, fileType);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t filePathCapacity = downloadDIRLength + fileNameCapacity + 1U;
    char   filePath[filePathCapacity];

    ret = snprintf(filePath, filePathCapacity, "%s/%s", downloadDIR, fileName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    bool needFetch = true;

    struct stat st;

    if (stat(filePath, &st) == 0 && S_ISREG(st.st_mode)) {
        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, filePath);

        if (ret != 0) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            needFetch = false;

            if (verbose) {
                fprintf(stderr, "%s already have been fetched.\n", filePath);
            }
        }
    }

    if (needFetch) {
        size_t tmpStrCapacity = strlen(url) + 30U;
        char   tmpStr[tmpStrCapacity];

        ret = snprintf(tmpStr, tmpStrCapacity, "%s|%ld|%d", url, time(NULL), getpid());

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        char tmpFileName[65] = {0};

        ret = sha256sum_of_string(tmpFileName, tmpStr);

        if (ret != 0) {
            return XCPKG_ERROR;
        }

        size_t tmpFilePathCapacity = downloadDIRLength + 65U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

        if (ret != XCPKG_OK) {
            if (uri != NULL) {
                ret = xcpkg_http_fetch_to_file(uri, tmpFilePath, verbose, verbose);
            }
        }

        if (ret != XCPKG_OK) {
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, tmpFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            if (rename(tmpFilePath, filePath) == 0) {
                if (verbose) {
                    printf("%s\n", filePath);
                }
            } else {
                perror(filePath);
                return XCPKG_ERROR;
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
            return XCPKG_ERROR_SHA256_MISMATCH;
        }
    }

    if (strcmp(fileType, ".zip") == 0 ||
        strcmp(fileType, ".tgz") == 0 ||
        strcmp(fileType, ".txz") == 0 ||
        strcmp(fileType, ".tlz") == 0 ||
        strcmp(fileType, ".tbz2") == 0 ||
        strcmp(fileType, ".crate") == 0) {

        ret = tar_extract(unpackDIR, filePath, ARCHIVE_EXTRACT_TIME, verbose, 1);

        if (ret != 0) {
            return abs(ret) + XCPKG_ERROR_ARCHIVE_BASE;
        }
    } else {
        size_t toFilePathCapacity = unpackDIRLength + fileNameCapacity + 1U;
        char   toFilePath[toFilePathCapacity];

        ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", unpackDIR, fileName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_copy_file(filePath, toFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

static int setup_rust_toolchain(const XCPKGInstallOptions * installOptions, const char * sessionDIR, const size_t sessionDIRLength) {
    const char * cargoHomeDIR = getenv("CARGO_HOME");

    if (cargoHomeDIR == NULL || cargoHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
            return XCPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t defaultCargoHomeDIRCapacity = strlen(userHomeDIR) + 8U;
        char   defaultCargoHomeDIR[defaultCargoHomeDIRCapacity];

        int ret = snprintf(defaultCargoHomeDIR, defaultCargoHomeDIRCapacity, "%s/.cargo", userHomeDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CARGO_HOME", defaultCargoHomeDIR, 1) != 0) {
            perror("CARGO_HOME");
            return XCPKG_ERROR;
        }

        cargoHomeDIR = getenv("CARGO_HOME");
    }

    size_t cargoHomeDIRLength = strlen(cargoHomeDIR);

    size_t rustupCommandPathCapacity = cargoHomeDIRLength + 12U;
    char   rustupCommandPath[rustupCommandPathCapacity];

    int ret = snprintf(rustupCommandPath, rustupCommandPathCapacity, "%s/bin/rustup", cargoHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t cargoCommandPathCapacity = cargoHomeDIRLength + 11U;
    char   cargoCommandPath[cargoCommandPathCapacity];

    ret = snprintf(cargoCommandPath, cargoCommandPathCapacity, "%s/bin/cargo", cargoHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    const bool  cargoExist = (stat( cargoCommandPath, &st) == 0) && S_ISREG(st.st_mode);
    const bool rustupExist = (stat(rustupCommandPath, &st) == 0) && S_ISREG(st.st_mode);

    if (!(cargoExist && rustupExist)) {
        LOG_INFO("rustup and cargo commands are required, but they are not found on this machine, xcpkg will install them via running shell script.");

        size_t rustupInitScriptFilePathCapacity = sessionDIRLength + 16U;
        char   rustupInitScriptFilePath[rustupInitScriptFilePathCapacity];

        ret = snprintf(rustupInitScriptFilePath, rustupInitScriptFilePathCapacity, "%s/rustup-init.sh", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_http_fetch_to_file("https://sh.rustup.rs", rustupInitScriptFilePath, installOptions->verbose_net, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_fork_exec2(3, "bash", rustupInitScriptFilePath, "-y");

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return XCPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t newPATHCapacity = cargoHomeDIRLength + strlen(PATH) + 6U;
    char   newPATH[newPATHCapacity];

    ret = snprintf(newPATH, newPATHCapacity, "%s/bin:%s", cargoHomeDIR, PATH);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("PATH", newPATH, 1) != 0) {
        perror("PATH");
        return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

typedef int (*setenv_fn)(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity);

static int setenv_CPPFLAGS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t includeDIRCapacity = packageInstalledDIRCapacity + 9U;
    char   includeDIR[includeDIRCapacity];

    int ret = snprintf(includeDIR, includeDIRCapacity, "%s/include", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(includeDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const CPPFLAGS = getenv("CPPFLAGS");

        if (CPPFLAGS == NULL || CPPFLAGS[0] == '\0') {
            size_t newCPPFLAGSCapacity = includeDIRCapacity + 3U;
            char   newCPPFLAGS[newCPPFLAGSCapacity];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSCapacity, "-I%s", includeDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return XCPKG_ERROR;
            }
        } else {
            size_t newCPPFLAGSCapacity = includeDIRCapacity + strlen(CPPFLAGS) + 4U;
            char   newCPPFLAGS[newCPPFLAGSCapacity];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSCapacity, "-I%s %s", includeDIR, CPPFLAGS);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_LDFLAGS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t libDIRCapacity = packageInstalledDIRCapacity + 5U;
    char   libDIR[libDIRCapacity];

    int ret = snprintf(libDIR, libDIRCapacity, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(libDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const LDFLAGS = getenv("LDFLAGS");

        if (LDFLAGS == NULL || LDFLAGS[0] == '\0') {
            size_t newLDFLAGSCapacity = libDIRCapacity + 3U;
            char   newLDFLAGS[newLDFLAGSCapacity];

            ret = snprintf(newLDFLAGS, newLDFLAGSCapacity, "-L%s", libDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return XCPKG_ERROR;
            }
        } else {
            size_t newLDFLAGSCapacity = (libDIRCapacity << 1U) + strlen(LDFLAGS) + 15U;
            char   newLDFLAGS[newLDFLAGSCapacity];

            ret = snprintf(newLDFLAGS, newLDFLAGSCapacity, "%s -L%s -Wl,-rpath,%s", LDFLAGS, libDIR, libDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_PKG_CONFIG_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    const char * a[2] = { "lib", "share" };

    for (int i = 0; i < 2; i++) {
        size_t pkgconfigDIRCapacity = packageInstalledDIRCapacity + 20U;
        char   pkgconfigDIR[pkgconfigDIRCapacity];

        int ret = snprintf(pkgconfigDIR, pkgconfigDIRCapacity, "%s/%s/pkgconfig", packageInstalledDIR, a[i]);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (stat(pkgconfigDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
            const char * const PKG_CONFIG_PATH = getenv("PKG_CONFIG_PATH");

            if (PKG_CONFIG_PATH == NULL || PKG_CONFIG_PATH[0] == '\0') {
                if (setenv("PKG_CONFIG_PATH", pkgconfigDIR, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return XCPKG_ERROR;
                }
            } else {
                size_t newPKG_CONFIG_PATHCapacity = pkgconfigDIRCapacity + strlen(PKG_CONFIG_PATH) + 2U;
                char   newPKG_CONFIG_PATH[newPKG_CONFIG_PATHCapacity];

                ret = snprintf(newPKG_CONFIG_PATH, newPKG_CONFIG_PATHCapacity, "%s:%s", PKG_CONFIG_PATH, pkgconfigDIR);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                if (setenv("PKG_CONFIG_PATH", newPKG_CONFIG_PATH, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return XCPKG_ERROR;
                }
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    size_t binDIRCapacity = packageInstalledDIRCapacity + 5U;
    char   binDIR[binDIRCapacity];

    int ret = snprintf(binDIR, binDIRCapacity, "%s/bin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t sbinDIRCapacity = packageInstalledDIRCapacity + 6U;
    char   sbinDIR[sbinDIRCapacity];

    ret = snprintf(sbinDIR, sbinDIRCapacity, "%s/sbin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    const bool  binDIRExists = stat( binDIR, &st) == 0 && S_ISDIR(st.st_mode);
    const bool sbinDIRExists = stat(sbinDIR, &st) == 0 && S_ISDIR(st.st_mode);

    if (binDIRExists || sbinDIRExists) {
        const char * const PATH = getenv("PATH");

        if (PATH == NULL || PATH[0] == '\0') {
            return XCPKG_ERROR_ENV_PATH_NOT_SET;
        }

        if (binDIRExists && sbinDIRExists) {
            size_t newPATHLength = binDIRCapacity + sbinDIRCapacity + strlen(PATH) + 3U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s:%s", binDIR, sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return XCPKG_ERROR;
            }
        } else if (binDIRExists) {
            size_t newPATHLength = binDIRCapacity + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", binDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return XCPKG_ERROR;
            }
        } else if (sbinDIRExists) {
            size_t newPATHLength = sbinDIRCapacity + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_ACLOCAL_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t shareDIRCapacity = packageInstalledDIRCapacity + 7U;
    char   shareDIR[shareDIRCapacity];

    int ret = snprintf(shareDIR, shareDIRCapacity, "%s/share", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html

    size_t aclocalDIRCapacity = shareDIRCapacity + 9U;
    char   aclocalDIR[aclocalDIRCapacity];

    ret = snprintf(aclocalDIR, aclocalDIRCapacity, "%s/aclocal", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(aclocalDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const ACLOCAL_PATH = getenv("ACLOCAL_PATH");

        if (ACLOCAL_PATH == NULL || ACLOCAL_PATH[0] == '\0') {
            if (setenv("ACLOCAL_PATH", aclocalDIR, 1) != 0) {
                perror("ACLOCAL_PATH");
                return XCPKG_ERROR;
            }
        } else {
            size_t newACLOCAL_PATHLength = aclocalDIRCapacity + strlen(ACLOCAL_PATH) + 2U;
            char   newACLOCAL_PATH[newACLOCAL_PATHLength];

            ret = snprintf(newACLOCAL_PATH, newACLOCAL_PATHLength, "%s:%s", aclocalDIR, ACLOCAL_PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("ACLOCAL_PATH", newACLOCAL_PATH, 1) != 0) {
                perror("ACLOCAL_PATH");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_XDG_DATA_DIRS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t shareDIRCapacity = packageInstalledDIRCapacity + 7U;
    char   shareDIR[shareDIRCapacity];

    int ret = snprintf(shareDIR, shareDIRCapacity, "%s/share", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    // https://gi.readthedocs.io/en/latest/tools/g-ir-scanner.html#environment-variables
    // https://help.gnome.org/admin//system-admin-guide/2.32/mimetypes-database.html.en

    size_t girSearchDIRCapacity = shareDIRCapacity + 9U;
    char   girSearchDIR[girSearchDIRCapacity];

    ret = snprintf(girSearchDIR, girSearchDIRCapacity, "%s/gir-1.0", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t mimeSearchDIRLength = shareDIRCapacity + 6U;
    char   mimeSearchDIR[mimeSearchDIRLength];

    ret = snprintf(mimeSearchDIR, mimeSearchDIRLength, "%s/mime", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if ((stat(girSearchDIR, &st) == 0 && S_ISDIR(st.st_mode)) || (stat(mimeSearchDIR, &st) == 0 && S_ISDIR(st.st_mode))) {
        const char * const XDG_DATA_DIRS = getenv("XDG_DATA_DIRS");

        if (XDG_DATA_DIRS == NULL || XDG_DATA_DIRS[0] == '\0') {
            if (setenv("XDG_DATA_DIRS", shareDIR, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return XCPKG_ERROR;
            }
        } else {
            size_t newXDG_DATA_DIRSLength = shareDIRCapacity + strlen(XDG_DATA_DIRS) + 2U;
            char   newXDG_DATA_DIRS[newXDG_DATA_DIRSLength];

            ret = snprintf(newXDG_DATA_DIRS, newXDG_DATA_DIRSLength, "%s:%s", shareDIR, XDG_DATA_DIRS);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("XDG_DATA_DIRS", newXDG_DATA_DIRS, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}


#define BUILD_SYSTEM_TYPE_CMAKE     1
#define BUILD_SYSTEM_TYPE_CONFIGURE 2

#define NATIVE_PACKAGE_ID_LIBZ     1
#define NATIVE_PACKAGE_ID_OPENSSL  2

#define NATIVE_PACKAGE_ID_AUTOCONF 5
#define NATIVE_PACKAGE_ID_AUTOMAKE 6
#define NATIVE_PACKAGE_ID_LIBTOOL  7
#define NATIVE_PACKAGE_ID_TEXINFO  8
#define NATIVE_PACKAGE_ID_HELP2MAN 9
#define NATIVE_PACKAGE_ID_INTLTOOL 10

#define NATIVE_PACKAGE_ID_LIBEXPAT 18
#define NATIVE_PACKAGE_ID_PERL_XML_PARSER 19

typedef struct {
    const char * name;

    const char * srcUrl;
    const char * srcUri;
    const char * srcSha;

    const char * dep_upp;

    const char * buildConfigureArgs;

    int buildSystemType;

    int depPackageIDArray[10];
} NativePackage;

static int getNativePackageInfoByID(int packageID, NativePackage * nativePackage) {
    switch (packageID) {
        case NATIVE_PACKAGE_ID_LIBEXPAT:
            nativePackage->name = "libexpat";
            nativePackage->srcUrl = "https://github.com/libexpat/libexpat/releases/download/R_2_6_4/expat-2.6.4.tar.xz";
            nativePackage->srcSha = "a695629dae047055b37d50a0ff4776d1d45d0a4c842cf4ccee158441f55ff7ee";
            nativePackage->buildConfigureArgs = "-DEXPAT_BUILD_DOCS=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_FUZZERS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TOOLS=OFF";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_LIBZ:
            nativePackage->name = "libz";
            nativePackage->srcUrl = "https://zlib.net/zlib-1.3.1.tar.gz";
            nativePackage->srcSha = "9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23";
            nativePackage->buildConfigureArgs = "-DZLIB_BUILD_EXAMPLES=OFF";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_OPENSSL:
            nativePackage->name = "openssl";
            nativePackage->srcUrl = "https://www.openssl.org/source/openssl-3.4.1.tar.gz";
            nativePackage->srcSha = "002a2d6b30b58bf4bea46c43bdd96365aaf8daa6c428782aa4feee06da197df3";
            nativePackage->dep_upp = "perl";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_TEXINFO:
            nativePackage->name = "texinfo";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/texinfo/texinfo-7.2.tar.xz";
            nativePackage->srcSha = "0329d7788fbef113fa82cb80889ca197a344ce0df7646fe000974c5d714363a6";
            nativePackage->dep_upp = "perl";
            nativePackage->buildConfigureArgs = "--with-included-regex --enable-threads=posix --disable-nls";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_HELP2MAN:
            nativePackage->name = "help2man";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz";
            nativePackage->srcSha = "4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f";
            nativePackage->dep_upp = "perl";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_INTLTOOL:
            nativePackage->name = "intltool";
            nativePackage->srcUrl = "https://launchpad.net/intltool/trunk/0.51.0/+download/intltool-0.51.0.tar.gz";
            nativePackage->srcSha = "67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL_XML_PARSER;
            nativePackage->buildConfigureArgs = "";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBTOOL:
            nativePackage->name = "libtool";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/libtool/libtool-2.5.4.tar.xz";
            nativePackage->srcSha = "f81f5860666b0bc7d84baddefa60d1cb9fa6fceb2398cc3baca6afaa60266675";
            nativePackage->buildConfigureArgs = "--enable-ltdl-install";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOCONF:
            nativePackage->name = "autoconf";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/autoconf/autoconf-2.72.tar.gz";
            nativePackage->srcSha = "afb181a76e1ee72832f6581c0eddf8df032b83e2e0239ef79ebedc4467d92d6e";
            nativePackage->dep_upp = "perl";
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOMAKE:
            nativePackage->name = "automake";
            nativePackage->srcUrl = "https://ftp.gnu.org/gnu/automake/automake-1.17.tar.xz";
            nativePackage->srcSha = "8920c1fc411e13b90bf704ef9db6f29d540e76d232cb3b2c9f4dc4cc599bd990";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_AUTOCONF;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_PERL_XML_PARSER:
            nativePackage->name = "perl-XML-Parser";
            nativePackage->srcUrl = "https://cpan.metacpan.org/authors/id/T/TO/TODDR/XML-Parser-2.46.tar.gz";
            nativePackage->srcSha = "d331332491c51cccfb4cb94ffc44f9cd73378e618498d4a37df9e043661c515d";
            nativePackage->dep_upp = "perl";
            nativePackage->depPackageIDArray[0] = NATIVE_PACKAGE_ID_LIBEXPAT;
            nativePackage->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        default:
            fprintf(stderr, "unknown native package id: %d\n", packageID);
            return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

typedef struct {
    const char * name;
    const char * libs;
    const char * env1;
    const char * env2;
} XX;

static int install_native_package(
        int nativePackageID,
        const char * downloadsDIR,
        const size_t downloadsDIRLength,
        const char * packageWorkingTopDIR,
        const size_t packageWorkingTopDIRCapacity,
        const char * nativePackageInstalledRootDIR,
        const size_t nativePackageInstalledRootDIRCapacity,
        const size_t njobs,
        const XCPKGInstallOptions * installOptions) {
    NativePackage nativePackage = {0};

    int ret = getNativePackageInfoByID(nativePackageID, &nativePackage);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 10; i++) {
        if (nativePackage.depPackageIDArray[i] == 0) {
            break;
        }

        ret = install_native_package(nativePackage.depPackageIDArray[i], downloadsDIR, downloadsDIRLength, packageWorkingTopDIR, packageWorkingTopDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, njobs, installOptions);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * packageName = nativePackage.name;
    const char * srcUrl = nativePackage.srcUrl;
    const char * srcUri = nativePackage.srcUri;
    const char * srcSha = nativePackage.srcSha;
    const char * buildConfigureArgs = nativePackage.buildConfigureArgs;
    int          buildSystemType = nativePackage.buildSystemType;

    if (buildConfigureArgs == NULL) {
        buildConfigureArgs = "";
    }

    printf("native package '%s' is being installed.\n", packageName);

    //////////////////////////////////////////////////////////////////////////////

    size_t packageNameLength = strlen(packageName);

    size_t receiptFilePathLength = nativePackageInstalledRootDIRCapacity + packageNameLength + 14U;
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/receipt.txt", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char buf[65] = {0};

            ret = xcpkg_read_the_first_n_bytes_of_a_file(receiptFilePath, 64, buf);

            if (ret != XCPKG_OK) {
                return ret;
            }

            if (strcmp(buf, srcSha) == 0) {
                fprintf(stderr, "native package '%s' already has been installed.\n", packageName);

                size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
                char   packageInstalledDIR[packageInstalledDIRCapacity];

                ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                setenv_fn funs[6] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

                for (int i = 0; i < 6; i++) {
                    ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

                    if (ret != XCPKG_OK) {
                        return ret;
                    }
                }

                return XCPKG_OK;
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", receiptFilePath);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t nativePackageWorkingTopDIRLength = packageWorkingTopDIRCapacity + packageNameLength + 14U;
    char   nativePackageWorkingTopDIR[nativePackageWorkingTopDIRLength];

    ret = snprintf(nativePackageWorkingTopDIR, nativePackageWorkingTopDIRLength, "%s/native-build-%s", packageWorkingTopDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t nativePackageWorkingSrcDIRLength = nativePackageWorkingTopDIRLength + 5U;
    char   nativePackageWorkingSrcDIR[nativePackageWorkingSrcDIRLength];

    ret = snprintf(nativePackageWorkingSrcDIR, nativePackageWorkingSrcDIRLength, "%s/src", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = download_via_http(srcUrl, srcUri, srcSha, downloadsDIR, downloadsDIRLength, nativePackageWorkingSrcDIR, nativePackageWorkingSrcDIRLength, installOptions->verbose_net);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(nativePackageWorkingSrcDIR) != 0) {
        perror(nativePackageWorkingSrcDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t strBufSize = packageNameLength + strlen(srcUrl) + strlen(srcSha) + 50U;
    char   strBuf[strBufSize];

    ret = snprintf(strBuf, strBufSize, "%s:%s:%s:%ld:%d", packageName, srcUrl, srcSha, time(NULL), getpid());

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char packageInstalledSHA[65] = {0};

    ret = sha256sum_of_string(packageInstalledSHA, strBuf);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + 66U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t nativePackageWorkingLibDIRLength = nativePackageWorkingTopDIRLength + 5U;
    char   nativePackageWorkingLibDIR[nativePackageWorkingLibDIRLength];

    ret = snprintf(nativePackageWorkingLibDIR, nativePackageWorkingLibDIRLength, "%s/lib", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t nativePackageWorkingIncludeDIRLength = nativePackageWorkingTopDIRLength + 9U;
    char   nativePackageWorkingIncludeDIR[nativePackageWorkingIncludeDIRLength];

    ret = snprintf(nativePackageWorkingIncludeDIR, nativePackageWorkingIncludeDIRLength, "%s/include", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t nativePackageWorkingPkgconfigDIRLength = nativePackageWorkingTopDIRLength + 15U;
    char   nativePackageWorkingPkgconfigDIR[nativePackageWorkingPkgconfigDIRLength];

    ret = snprintf(nativePackageWorkingPkgconfigDIR, nativePackageWorkingPkgconfigDIRLength, "%s/lib/pkgconfig", nativePackageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", nativePackageWorkingLibDIR, 1) != 0) {
        perror("PKG_CONFIG_LIBDIR");
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledLibraryDIRLength = packageInstalledDIRCapacity + 5U;
    char   packageInstalledLibraryDIR[packageInstalledLibraryDIRLength];

    ret = snprintf(packageInstalledLibraryDIR, packageInstalledLibraryDIRLength, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    const char * const LDFLAGS = getenv("LDFLAGS");

    if (LDFLAGS == NULL || LDFLAGS[0] == '\0') {
        size_t newLDFLAGSLength = packageInstalledLibraryDIRLength + 12U;
        char   newLDFLAGS[newLDFLAGSLength];

        ret = snprintf(newLDFLAGS, newLDFLAGSLength, "-Wl,-rpath,%s", packageInstalledLibraryDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
            perror("LDFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t newLDFLAGSLength = packageInstalledLibraryDIRLength + strlen(LDFLAGS) + 15U;
        char   newLDFLAGS[newLDFLAGSLength];

        ret = snprintf(newLDFLAGS, newLDFLAGSLength, "-Wl,-rpath,%s %s", packageInstalledLibraryDIR, LDFLAGS);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
            perror("LDFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (nativePackageID == NATIVE_PACKAGE_ID_INTLTOOL) {
        size_t perl5LibDIRCapacity = nativePackageInstalledRootDIRCapacity + 27U;
        char   perl5LibDIR[perl5LibDIRCapacity];

        ret = snprintf(perl5LibDIR, perl5LibDIRCapacity, "%s/perl-XML-Parser/lib/perl5", nativePackageInstalledRootDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("PERL5LIB", perl5LibDIR, 1) != 0) {
            perror("PERL5LIB");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (buildSystemType == BUILD_SYSTEM_TYPE_CMAKE) {
        size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildConfigureArgs) + 140U;
        char   configurePhaseCmd[configurePhaseCmdLength];

        ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "cmake -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=%s -DEXPAT_SHARED_LIBS=OFF -DCMAKE_VERBOSE_MAKEFILE=%s %s -G Ninja -S . -B build.d", packageInstalledDIR, (installOptions->logLevel >= XCPKGLogLevel_verbose) ? "ON" : "OFF", buildConfigureArgs);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_fork_exec(configurePhaseCmd);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_fork_exec2(3, "cmake", "--build", "build.d");

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_fork_exec2(3, "cmake", "--install", "build.d");

        if (ret != XCPKG_OK) {
            return ret;
        }
    } else if (buildSystemType == BUILD_SYSTEM_TYPE_CONFIGURE) {
        if (nativePackageID == NATIVE_PACKAGE_ID_OPENSSL) {
            size_t configurePhaseCmdLength = (packageInstalledDIRCapacity * 3) + 100U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./config no-tests no-ssl3 no-ssl3-method no-zlib --prefix=%s --libdir=%s/lib --openssldir=%s/etc/ssl", packageInstalledDIR, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_fork_exec(configurePhaseCmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        } else {
            size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildConfigureArgs) + 32U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./configure --prefix=%s %s %s", packageInstalledDIR, (installOptions->logLevel == XCPKGLogLevel_silent) ? "--silent" : "", buildConfigureArgs);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_fork_exec(configurePhaseCmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }

        //////////////////////////////////////////////////////////////////////////////

        char buildPhaseCmd[20];

        ret = snprintf(buildPhaseCmd, 20, "gmake --jobs=%zu", njobs);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_fork_exec(buildPhaseCmd);

        if (ret != XCPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        ret = xcpkg_fork_exec2(2, "gmake", "install");

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    size_t receiptFilePath2Length = packageInstalledDIRCapacity + 14U;
    char   receiptFilePath2[receiptFilePath2Length];

    ret = snprintf(receiptFilePath2, receiptFilePath2Length, "%s/receipt.txt", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_write_file(receiptFilePath2, srcSha, 0);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (chdir(nativePackageInstalledRootDIR) != 0) {
        perror(nativePackageInstalledRootDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "native package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = xcpkg_rm_rf(packageName, false, installOptions->logLevel >= XCPKGLogLevel_verbose);

                        if (ret != XCPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return XCPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return XCPKG_ERROR;
            }
        }
    }

    setenv_fn funs[6] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

    for (int i = 0; i < 6; i++) {
        ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

static int install_dependent_packages_via_uppm(
        const char * xcpkgHomeDIR,
        const size_t xcpkgHomeDIRLength,
        const char * uppmPackageNames,
        const size_t uppmPackageNamesLength,
        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRCapacity,
        const bool   verbose) {
    size_t uppmHomeDIRCapacity = xcpkgHomeDIRLength + 6U;
    char   uppmHomeDIR[uppmHomeDIRCapacity];

    int ret = snprintf(uppmHomeDIR, uppmHomeDIRCapacity, "%s/uppm", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t uppmHomeDIRLength = ret;

    ret = uppm_formula_repo_sync_official_core(uppmHomeDIR, uppmHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t  uppmPackageNamesCapacity = uppmPackageNamesLength + 1U;
    char    uppmPackageNamesCopy[uppmPackageNamesCapacity];
    strncpy(uppmPackageNamesCopy, uppmPackageNames, uppmPackageNamesCapacity);

    fprintf(stderr, "uppm packages to be installed in order: %s\n", uppmPackageNames);

    char * q;
    char * uppmPackageName = strtok_r(uppmPackageNamesCopy, " ", &q);

    while (uppmPackageName != NULL) {
        fprintf(stderr, "=========0 %s\n", uppmPackageName);
        ret = uppm_install(uppmHomeDIR, uppmHomeDIRLength, uppmPackageName, verbose, false);

        if (ret != XCPKG_OK) {
            return ret;
        }

        fprintf(stderr, "=========1 %s\n", uppmPackageName);
        size_t uppmPackageInstalledDIRCapacity = uppmPackageInstalledRootDIRCapacity + strlen(uppmPackageName) + 2U;
        char   uppmPackageInstalledDIR[uppmPackageInstalledDIRCapacity];

        ret = snprintf(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity, "%s/%s", uppmPackageInstalledRootDIR, uppmPackageName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        fprintf(stderr, "=========2 %s\n", uppmPackageName);
        ret = setenv_PATH(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }

        fprintf(stderr, "=========3 %s\n", uppmPackageName);
        ret = setenv_ACLOCAL_PATH(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }

        fprintf(stderr, "=========4 %s\n", uppmPackageName);
        ret = setenv_XDG_DATA_DIRS(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }

        fprintf(stderr, "=========6 %s\n", uppmPackageName);
        if (strcmp(uppmPackageName, "git") == 0) {
            // https://git-scm.com/book/en/v2/Git-Internals-Environment-Variables

            size_t gitCoreDIRCapacity = uppmPackageInstalledDIRCapacity + 18U;
            char   gitCoreDIR[gitCoreDIRCapacity];

            ret = snprintf(gitCoreDIR, gitCoreDIRCapacity, "%s/libexec/git-core", uppmPackageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("GIT_EXEC_PATH" , gitCoreDIR, 1) != 0) {
                perror("GIT_EXEC_PATH");
                return XCPKG_ERROR;
            }

            size_t gitTemplateDIRCapacity = uppmPackageInstalledDIRCapacity + 26U;
            char   gitTemplateDIR[gitTemplateDIRCapacity];

            ret = snprintf(gitTemplateDIR, gitTemplateDIRCapacity, "%s/share/git-core/templates", uppmPackageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("GIT_TEMPLATE_DIR" , gitTemplateDIR, 1) != 0) {
                perror("GIT_TEMPLATE_DIR");
                return XCPKG_ERROR;
            }
        } else if (strcmp(uppmPackageName, "docbook-xsl") == 0) {
            // http://xmlsoft.org/xslt/xsltproc.html

            size_t xmlCatalogFilePathCapacity = uppmPackageInstalledDIRCapacity + 13U;
            char   xmlCatalogFilePath[xmlCatalogFilePathCapacity];

            ret = snprintf(xmlCatalogFilePath, xmlCatalogFilePathCapacity, "%s/catalog.xml", uppmPackageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("XML_CATALOG_FILES" , xmlCatalogFilePath, 1) != 0) {
                perror("XML_CATALOG_FILES");
                return XCPKG_ERROR;
            }
        }

        fprintf(stderr, "=========7 %s\n", uppmPackageName);
        uppmPackageName = strtok_r(NULL, " ", &q);
    }

    return XCPKG_OK;
}

static int generate_shell_script_file(
        const char * shellScriptFilePath,
        const char * packageName,
        const XCPKGFormula * formula,
        const XCPKGInstallOptions * installOptions,
        const SysInfo * sysinfo,
        const char * uppmPackageInstalledRootDIR,
        const char * nativePackageInstalledRootDIR,
        const char * xcpkgExeFilePath,
        const time_t ts,
        const size_t njobs,
        const bool isCrossBuild,
        const char * targetPlatformName,
        const char * targetPlatformVers,
        const char * targetPlatformArch,
        const char * xcpkgHomeDIR,
        const char * xcpkgCoreDIR,
        const char * xcpkgDownloadsDIR,
        const char * sessionDIR,
        const char * packageWorkingTopDIR,
        const char * packageWorkingSrcDIR,
        const char * packageInstalledRootDIR,
        const size_t packageInstalledRootDIRCapacity,
        const char * packageInstalledDIR,
        const char * packageMetaInfoDIR,

        const char * recursiveDependentPackageNamesString,
        const size_t recursiveDependentPackageNamesStringSize) {
    int fd = open(shellScriptFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(shellScriptFilePath);
        return XCPKG_ERROR;
    }

    int ret;

    if (installOptions->xtrace) {
        ret = dprintf(fd, "set -x\n\n");

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    ret = dprintf(fd, "set -e\n\nunset IFS\n\n");

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs1[] = {
        {"NATIVE_PLATFORM_ARCH", sysinfo->arch },
        {"NATIVE_PLATFORM_VERS", sysinfo->vers },
        {NULL, NULL }
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs1[i].name;
        const char * value = kvs1[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "NATIVE_PLATFORM_NCPU=%u\n", sysinfo->ncpu);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_PLATFORM_EUID=%u\n", geteuid());

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_PLATFORM_EGID=%u\n\n", getegid());

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV targets[] = {
        {"TARGET_PLATFORM_NAME", targetPlatformName },
        {"TARGET_PLATFORM_VERS", targetPlatformVers },
        {"TARGET_PLATFORM_ARCH", targetPlatformArch },
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = targets[i].name;
        const char * value = targets[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\n");

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs2[] = {
        {"XCPKG_VERSION", XCPKG_VERSION_STRING},
        {"XCPKG_PATH", xcpkgExeFilePath},
        {"XCPKG_HOME", xcpkgHomeDIR},
        {"XCPKG_CORE_DIR", xcpkgCoreDIR},
        {"XCPKG_DOWNLOADS_DIR", xcpkgDownloadsDIR},
        {"XCPKG_PACKAGE_INSTALLED_ROOT", packageInstalledRootDIR},
        {" UPPM_PACKAGE_INSTALLED_ROOT", uppmPackageInstalledRootDIR},
        {"NATIVE_PACKAGE_INSTALLED_ROOT", nativePackageInstalledRootDIR},
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs2[i].name;
        const char * value = kvs2[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\nCROSS_COMPILING=%d\n\n", isCrossBuild);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KB options[] = {
        {"KEEP_SESSION_DIR", installOptions->keepSessionDIR},
        {"BEAR_ENABLED", installOptions->enableBear},
        {"CCACHE_ENABLED", installOptions->enableCcache},
        {"EXPORT_COMPILE_COMMANDS_JSON", installOptions->exportCompileCommandsJson},
        {NULL,false}
    };

    for (int i = 0; ; i++) {
        const char * name  = options[i].name;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s=%d\n", name, options[i].value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "LOG_LEVEL=%d\n", installOptions->logLevel);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "BUILD_NJOBS=%zu\n", njobs);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "PROFILE=%s\n\n", installOptions->buildType == XCPKGBuildProfile_release ? "release" : "debug");

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "RECURSIVE_DEPENDENT_PACKAGE_NAMES='%s'\n", (recursiveDependentPackageNamesString == NULL) ? "" : recursiveDependentPackageNamesString);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\nTIMESTAMP_UNIX=%ld\n\n", ts);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KB kbs[] = {
        {"PACKAGE_BINBSTD", formula->binbstd},
        {"PACKAGE_SYMLINK", formula->symlink},
        {"PACKAGE_BUILD_IN_PARALLEL", formula->support_build_in_parallel},
        {"PACKAGE_BUILD_IN_BSCRIPT_DIR", formula->binbstd},
        {"PACKAGE_USE_BSYSTEM_AUTOGENSH", formula->useBuildSystemAutogen},
        {"PACKAGE_USE_BSYSTEM_AUTOTOOLS", formula->useBuildSystemAutotools},
        {"PACKAGE_USE_BSYSTEM_CONFIGURE", formula->useBuildSystemConfigure},
        {"PACKAGE_USE_BSYSTEM_CMAKE", formula->useBuildSystemCmake},
        {"PACKAGE_USE_BSYSTEM_XMAKE", formula->useBuildSystemXmake},
        {"PACKAGE_USE_BSYSTEM_GMAKE", formula->useBuildSystemGmake},
        {"PACKAGE_USE_BSYSTEM_NINJA", formula->useBuildSystemNinja},
        {"PACKAGE_USE_BSYSTEM_MESON", formula->useBuildSystemMeson},
        {"PACKAGE_USE_BSYSTEM_CARGO", formula->useBuildSystemCargo},
        {"PACKAGE_USE_BSYSTEM_GO", formula->useBuildSystemGolang},
        {"PACKAGE_USE_BSYSTEM_GN", formula->useBuildSystemGN},
        {NULL,false}
    };

    for (int i = 0; ; i++) {
        const char * name  = kbs[i].name;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s=%d\n", name, kbs[i].value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }
    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "PACKAGE_GIT_NTH=%zu\n", formula->git_nth);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs[] = {
        {"PACKAGE_FORMULA_FILEPATH", formula->path},
        {"PACKAGE_NAME", packageName},
        {"PACKAGE_SUMMARY", formula->summary},
        {"PACKAGE_VERSION", formula->version},
        {"PACKAGE_LICENSE", formula->license},
        {"PACKAGE_WEB_URL", formula->web_url},
        {"PACKAGE_GIT_URL", formula->git_url},
        {"PACKAGE_GIT_SHA", formula->git_sha},
        {"PACKAGE_GIT_REF", formula->git_ref},
        {"PACKAGE_SRC_URL", formula->src_url},
        {"PACKAGE_SRC_URI", formula->src_uri},
        {"PACKAGE_SRC_SHA", formula->src_sha},
        {"PACKAGE_FIX_URL", formula->fix_url},
        {"PACKAGE_FIX_URI", formula->fix_uri},
        {"PACKAGE_FIX_SHA", formula->fix_sha},
        {"PACKAGE_RES_URL", formula->res_url},
        {"PACKAGE_RES_URI", formula->res_uri},
        {"PACKAGE_RES_SHA", formula->res_sha},
        {"PACKAGE_DEP_PKG", formula->dep_pkg},
        {"PACKAGE_DEP_UPP", formula->dep_upp},
        {"PACKAGE_DEP_PYM", formula->dep_pym},
        {"PACKAGE_DEP_PLM", formula->dep_plm},
        {"PACKAGE_BSYSTEM", formula->bsystem},
        {"PACKAGE_BSCRIPT", formula->bscript},
        {"PACKAGE_PPFLAGS", formula->ppflags},
        {"PACKAGE_CCFLAGS", formula->ccflags},
        {"PACKAGE_XXFLAGS", formula->xxflags},
        {"PACKAGE_LDFLAGS", formula->ldflags},

        {"PACKAGE_WORKING_DIR", packageWorkingTopDIR},
        {"PACKAGE_INSTALL_DIR", packageInstalledDIR},
        {"PACKAGE_METAINF_DIR", packageMetaInfoDIR},

        {NULL, NULL},
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs[i].name;
        const char * value = kvs[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, (value == NULL) ? "" : value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s'\n", packageWorkingSrcDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/_'\n", packageWorkingSrcDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    } else {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s/%s'\n", packageWorkingSrcDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/%s/_'\n", packageWorkingSrcDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "\n NATIVE_BCACHED_DIR='%s/-'\n", packageWorkingSrcDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    } else {
        ret = dprintf(fd, "\n NATIVE_BCACHED_DIR='%s/%s/-'\n", packageWorkingSrcDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    ret = dprintf(fd, " NATIVE_INSTALL_DIR='%s/%s'\n\n", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "        SESSION_DIR='%s'\n\n", sessionDIR);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs5[] = {
        {"DOFETCH", formula->dofetch },
        {"DO12345", formula->do12345 },
        {"DOPATCH", formula->dopatch },
        {"PREPARE", formula->prepare },
        {"DOBUILD", formula->install },
        {"DOTWEAK", formula->dotweak },
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs5[i].name;
        const char * value = kvs5[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "PACKAGE_%s=%d\n", name, (value == NULL) ? 0 : 1);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    KV actions[] = {
        {"dofetch", formula->dofetch },
        {"do12345", formula->do12345 },
        {"dopatch", formula->dopatch },
        {"prepare", formula->prepare },
        {"dobuild", formula->install },
        {"dotweak", formula->dotweak },
        {"isCrossBuild", isCrossBuild ? "true" : "false" },
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = actions[i].name;
        const char * value = actions[i].value;

        if (name == NULL) {
            break;
        }

        if (value != NULL) {
            ret = dprintf(fd, "\n%s() {\n%s\n}\n", name, value);

            if (ret < 0) {
                close(fd);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (recursiveDependentPackageNamesString != NULL) {
        size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
        char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
        strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

        char * dependentPackageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

        while (dependentPackageName != NULL) {
            size_t installedDIRCapacity = packageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
            char   installedDIR[installedDIRCapacity];

            ret = snprintf(installedDIR, installedDIRCapacity, "%s/%s", packageInstalledRootDIR, dependentPackageName);

            if (ret < 0) {
                perror(NULL);
                close(fd);
                return XCPKG_ERROR;
            }

            size_t libDIRCapacity = installedDIRCapacity + 4U;
            char   libDIR[libDIRCapacity];

            ret = snprintf(libDIR, libDIRCapacity, "%s/lib", installedDIR);

            if (ret < 0) {
                perror(NULL);
                close(fd);
                return XCPKG_ERROR;
            }

            size_t includeDIRCapacity = installedDIRCapacity + 8U;
            char   includeDIR[includeDIRCapacity];

            ret = snprintf(includeDIR, includeDIRCapacity, "%s/include", installedDIR);

            if (ret < 0) {
                perror(NULL);
                close(fd);
                return XCPKG_ERROR;
            }

            for (int i = 0; ; i++) {
                char c = dependentPackageName[i];

                if (c == '\0') {
                    break;
                }

                if (c == '@' || c == '+' || c == '-' || c == '.') {
                    dependentPackageName[i] = '_';
                }
            }

            KV kvs[3] = {
                {"INSTALL", installedDIR},
                {"INCLUDE", includeDIR},
                {"LIBRARY", libDIR}
            };

            for (int i = 0; i < 3; i++) {
                ret = dprintf(fd, "%s_%s_DIR='%s'\n", dependentPackageName, kvs[i].name, kvs[i].value);

                if (ret < 0) {
                    close(fd);
                    return XCPKG_ERROR;
                }
            }

            dependentPackageName = strtok(NULL, " ");
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, ". %s/xcpkg-install\n", xcpkgCoreDIR);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    close(fd);

    return XCPKG_OK;
}

static int adjust_la_file(const char * filePath, const char * xcpkgHomeDIR, const size_t xcpkgHomeDIRLength) {
    size_t eLength = xcpkgHomeDIRLength + 14U;
    char   e[eLength];

    int ret = snprintf(e, eLength, "s|-L%s[^' ]*||g", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (pid == 0) {
        execlp("sed", "sed", "-i", "-e", "s/-Wl,--strip-debug//g", "-e", "s|-R[^' ]*||g", "-e", e, filePath, NULL);
        perror("sed");
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return XCPKG_OK;
        } else {
            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' exit with status code: %d\n", e, filePath, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' killed by signal: %d\n", e, filePath, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' stopped by signal: %d\n", e, filePath, WSTOPSIG(childProcessExitStatusCode));
            }

            return XCPKG_ERROR;
        }
    }
}

static int adjust_la_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity, const char * xcpkgHomeDIR, const size_t xcpkgHomeDIRLength) {
    size_t packageLibDIRLength = packageInstalledDIRCapacity + 5U;
    char   packageLibDIR[packageLibDIRLength];

    int ret = snprintf(packageLibDIR, packageLibDIRLength, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(packageLibDIR, &st) != 0) {
        return XCPKG_OK;
    }

    DIR * dir = opendir(packageLibDIR);

    if (dir == NULL) {
        perror(packageLibDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(packageLibDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".la") == 0) {
            size_t filePathLength = packageLibDIRLength + fileNameLength  + 2U;
            char   filePath[filePathLength];

            ret = snprintf(filePath, filePathLength, "%s/%s", packageLibDIR, dir_entry->d_name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return XCPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = adjust_la_file(filePath, xcpkgHomeDIR, xcpkgHomeDIRLength);

                if (ret != XCPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_pc_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t packagePkgconfigDIRLength = packageInstalledDIRCapacity + 15U;
    char   packagePkgconfigDIR[packagePkgconfigDIRLength];

    int ret = snprintf(packagePkgconfigDIR, packagePkgconfigDIRLength, "%s/lib/pkgconfig", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(packagePkgconfigDIR, &st) != 0) {
        return XCPKG_OK;
    }

    DIR * dir = opendir(packagePkgconfigDIR);

    if (dir == NULL) {
        perror(packagePkgconfigDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(packagePkgconfigDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".pc") == 0) {
            size_t filePathLength = packagePkgconfigDIRLength + fileNameLength  + 2U;
            char   filePath[filePathLength];

            ret = snprintf(filePath, filePathLength, "%s/%s", packagePkgconfigDIR, dir_entry->d_name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return XCPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = XCPKG_OK;

                if (ret != XCPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_macho_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    return XCPKG_OK;
}

static int backup_formulas(const char * sessionDIR, const char * packageMetaInfoDIR, const size_t packageMetaInfoDIRCapacity, const char * recursiveDependentPackageNamesString, const size_t recursiveDependentPackageNamesStringSize) {
    size_t packageInstalledFormulaDIRLength = packageMetaInfoDIRCapacity + 9U;
    char   packageInstalledFormulaDIR[packageInstalledFormulaDIRLength];

    int ret = snprintf(packageInstalledFormulaDIR, packageInstalledFormulaDIRLength, "%s/formula", packageMetaInfoDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageInstalledFormulaDIR, S_IRWXU) != 0) {
        perror(packageInstalledFormulaDIR);
        return XCPKG_ERROR;
    }

    size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
    char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
    strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

    char * packageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

    while (packageName != NULL) {
        size_t packageNameLength = strlen(packageName);

        size_t fromFilePathLength = strlen(sessionDIR) + packageNameLength  + 6U;
        char   fromFilePath[fromFilePathLength];

        ret = snprintf(fromFilePath, fromFilePathLength, "%s/%s.yml", sessionDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        size_t toFilePathLength = packageInstalledFormulaDIRLength + packageNameLength + 6U;
        char   toFilePath[toFilePathLength];

        ret = snprintf(toFilePath, toFilePathLength, "%s/%s.yml", packageInstalledFormulaDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_copy_file(fromFilePath, toFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }

        packageName = strtok(NULL, " ");
    }

    return XCPKG_OK;
}

static int generate_manifest_r(const char * dirPath, const size_t offset, FILE * installedManifestFile) {
    if (dirPath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (dirPath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return XCPKG_ERROR;
    }

    size_t dirPathLength = strlen(dirPath);

    int ret = XCPKG_OK;

    struct stat st;

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(dirPath);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathLength = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char   filePath[filePathLength];

        ret = snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return XCPKG_ERROR;
        }

        if (stat(filePath, &st) != 0) {
            perror(filePath);
            closedir(dir);
            return XCPKG_ERROR;
        }

        if (S_ISDIR(st.st_mode)) {
            ret = fprintf(installedManifestFile, "d|%s/\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return XCPKG_ERROR;
            }

            ret = generate_manifest_r(filePath, offset, installedManifestFile);

            if (ret != XCPKG_OK) {
                closedir(dir);
                return ret;
            }
        } else {
            ret = fprintf(installedManifestFile, "f|%s\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }
    }
}

int generate_manifest(const char * installedDIRPath) {
    size_t installedDIRLength = strlen(installedDIRPath);

    size_t installedManifestFilePathLength = installedDIRLength + sizeof(XCPKG_MANIFEST_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);
    char   installedManifestFilePath[installedManifestFilePathLength];

    int ret = snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s%s", installedDIRPath, XCPKG_MANIFEST_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    FILE * installedManifestFile = fopen(installedManifestFilePath, "w");

    if (installedManifestFile == NULL) {
        perror(installedManifestFilePath);
        return XCPKG_ERROR;
    }

    ret = generate_manifest_r(installedDIRPath, installedDIRLength + 1, installedManifestFile);

    fclose(installedManifestFile);

    return ret;
}

static int generate_receipt(const char * packageName, const XCPKGFormula * formula, const char * targetPlatformSpec, const SysInfo * sysinfo, const time_t ts, const char * packageMetaInfoDIR, const size_t packageMetaInfoDIRCapacity) {
    size_t receiptFilePathLength = packageMetaInfoDIRCapacity + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_METADATA_DIR);
    char   receiptFilePath[receiptFilePathLength];

    int ret = snprintf(receiptFilePath, receiptFilePathLength, "%s%s", packageMetaInfoDIR, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_METADATA_DIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    FILE * receiptFile = fopen(receiptFilePath, "w");

    if (receiptFile == NULL) {
        perror(receiptFilePath);
        return XCPKG_ERROR;
    }

    FILE * formulaFile = fopen(formula->path, "r");

    if (formulaFile == NULL) {
        perror(formula->path);
        fclose(receiptFile);
        return XCPKG_ERROR;
    }

    fprintf(receiptFile, "pkgname: %s\n", packageName);

    if (formula->version_is_calculated) {
        fprintf(receiptFile, "version: %s\n", formula->version);
    }

    if (formula->bsystem_is_calculated) {
        fprintf(receiptFile, "bsystem: %s\n", formula->bsystem);
    }

    if (formula->web_url_is_calculated) {
        fprintf(receiptFile, "web-url: %s\n", formula->web_url);
    }

    char   buff[2048];
    size_t size = 0U;

    for (;;) {
        size = fread(buff, 1, 2048, formulaFile);

        if (ferror(formulaFile)) {
            perror(formula->path);
            fclose(formulaFile);
            fclose(receiptFile);
            return XCPKG_ERROR;
        }

        if (size > 0) {
            if (fwrite(buff, 1, size, receiptFile) != size || ferror(receiptFile)) {
                perror(receiptFilePath);
                fclose(receiptFile);
                fclose(formulaFile);
                return XCPKG_ERROR;
            }
        }

        if (feof(formulaFile)) {
            fclose(formulaFile);
            break;
        }
    }

    fprintf(receiptFile, "\nbuiltfor: %s\nbuiltby: %s\nbuiltat: %ld\n\n", targetPlatformSpec, XCPKG_VERSION_STRING, ts);

    fprintf(receiptFile, "build-on:\n    os-arch: %s\n    os-vers: %s\n    os-ncpu: %u\n    os-euid: %u\n    os-egid: %u\n", sysinfo->arch, sysinfo->vers, sysinfo->ncpu, sysinfo->euid, sysinfo->egid);

    fclose(receiptFile);

    return XCPKG_OK;
}

static int install_files_to_metainfo_dir(struct stat st, const char * fromDIR, size_t fromDIRLength, const char * toDIR, size_t toDIRLength, const char * item, size_t itemLength) {
    size_t fromFilePathLength = fromDIRLength + itemLength + 2U;
    char   fromFilePath[fromFilePathLength];

    int ret = snprintf(fromFilePath, fromFilePathLength, "%s/%s", fromDIR, item);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t toFilePathLength = toDIRLength + itemLength + 2U;
    char   toFilePath[toFilePathLength];

    ret = snprintf(toFilePath, toFilePathLength, "%s/%s", toDIR, item);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (stat(fromFilePath, &st) == 0) {
        return xcpkg_copy_file(fromFilePath, toFilePath);
    }

    return XCPKG_OK;
}

static int copy_dependent_libraries(
        const char * depPackageInstalledDIR,
        const size_t depPackageInstalledDIRLength,
        const char * toDIR,
        const size_t toDIRLength) {

    size_t fromDIRCapacity = depPackageInstalledDIRLength + 5U;
    char   fromDIR[fromDIRCapacity];

    int ret = snprintf(fromDIR, fromDIRCapacity, "%s/lib", depPackageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(fromDIR, &st) != 0) {
        return XCPKG_OK;
    }

    DIR * dir = opendir(fromDIR);

    if (dir == NULL) {
        perror(fromDIR);
        return XCPKG_ERROR;
    }

    char * fileName;
    size_t fileNameLength;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(fromDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        fileName = dir_entry->d_name;

        if (strncmp(fileName, "lib", 3) == 0) {
            fileNameLength = strlen(fileName);

            if (fileNameLength > 5 && strcmp(fileName + fileNameLength - 2U, ".a") == 0) {
                size_t   fromFilePathCapacity = fromDIRCapacity + fileNameLength + 2U;
                char     fromFilePath[fromFilePathCapacity];

                ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/%s", fromDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return XCPKG_ERROR;
                }

                size_t   toFilePathCapacity = toDIRLength + fileNameLength + 2U;
                char     toFilePath[toFilePathCapacity];

                ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", toDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return XCPKG_ERROR;
                }

                ret = xcpkg_copy_file(fromFilePath, toFilePath);

                if (ret != XCPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int xcpkg_build_for_native(
        const XCPKGFormula * formula,
        const char * packageName,
        const size_t packageNameLength,
        const char * packageWorkingTopDIR,
        const size_t packageWorkingTopDIRCapacity,
        const char * nativePackageInstalledRootDIR,
        const size_t nativePackageInstalledRootDIRCapacity,
        const char * packageInstalledSHA,
        const char * shellScriptFilePath,
        const size_t shellScriptFilePathCapacity,
        const bool verbose) {
    size_t receiptFilePathLength = nativePackageInstalledRootDIRCapacity + packageNameLength + 14U;
    char   receiptFilePath[receiptFilePathLength];

    int ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/receipt.txt", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char buf[65] = {0};

            ret = xcpkg_read_the_first_n_bytes_of_a_file(receiptFilePath, 64, buf);

            if (ret != XCPKG_OK) {
                return ret;
            }

            if (strcmp(buf, formula->src_sha) == 0) {
                fprintf(stderr, "native package '%s' already has been installed.\n", packageName);

                size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
                char   packageInstalledDIR[packageInstalledDIRCapacity];

                ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                setenv_fn funs[3] = { setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

                for (int i = 0; i < 3; i++) {
                    ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

                    if (ret != XCPKG_OK) {
                        return ret;
                    }
                }

                return XCPKG_OK;
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", receiptFilePath);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t workingDIRCapacity = packageWorkingTopDIRCapacity + packageNameLength + 14U;
    char   workingDIR[workingDIRCapacity];

    ret = snprintf(workingDIR, workingDIRCapacity, "%s/native-build-%s", packageWorkingTopDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir (workingDIR, S_IRWXU) != 0) {
        perror(workingDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstallDIRCapacity = nativePackageInstalledRootDIRCapacity + 66U;
    char   packageInstallDIR[packageInstallDIRCapacity];

    ret = snprintf(packageInstallDIR, packageInstallDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_fork_exec2(3, "/bin/sh", shellScriptFilePath, "native");

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (stat(packageInstallDIR, &st) != 0) {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t receiptFilePath2Length = packageInstallDIRCapacity + 12U;
    char   receiptFilePath2[receiptFilePath2Length];

    ret = snprintf(receiptFilePath2, receiptFilePath2Length, "%s/receipt.txt", packageInstallDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_write_file(receiptFilePath2, formula->src_sha, 0);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (nativePackageInstalledRootDIR) != 0) {
        perror(nativePackageInstalledRootDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "native package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = xcpkg_rm_rf(packageName, false, verbose);

                        if (ret != XCPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return XCPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return XCPKG_ERROR;
            }
        }
    }

    size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    setenv_fn funs[3] = { setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

    for (int i = 0; i < 3; i++) {
        ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

static int xcpkg_install_package(
        const char * packageName,
        const char * targetPlatformSpec,

        const XCPKGFormula * formula,
        const XCPKGInstallOptions * installOptions,
        const XCPKGToolChain * toolchain,
        const SysInfo * sysinfo,

        const char * cpp,

        const char * ccForNativeBuild,
        const char * cxxForNativeBuild,
        const char * objcForNativeBuild,

        const char * ccForTargetBuild,
        const char * cxxForTargetBuild,
        const char * objcForTargetBuild,

        const char *  ccFlagsForNativeBuild,
        const char * cxxFlagsForNativeBuild,
        const char * cppFlagsForNativeBuild,
        const char *  ldFlagsForNativeBuild,

        const char * extraCCFlagsForTargetBuild,
        const char * extraLDFlagsForTargetBuild,

        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRCapacity,
        const char * xcpkgExeFilePath,
        const char * xcpkgHomeDIR,
        const size_t xcpkgHomeDIRLength,
        const char * xcpkgCoreDIR,
        const size_t xcpkgCoreDIRCapacity,
        const char * xcpkgDownloadsDIR,
        const size_t xcpkgDownloadsDIRCapacity,
        const char * sessionDIR,
        const size_t sessionDIRLength,
        const char * recursiveDependentPackageNamesString,
        const size_t recursiveDependentPackageNamesStringSize) {
    fprintf(stderr, "%s=============== Installing%s %s%s/%s%s %s===============%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, targetPlatformSpec, packageName, COLOR_OFF, COLOR_PURPLE, COLOR_OFF);

    const time_t ts = time(NULL);

    const size_t packageNameLength = strlen(packageName);

    const size_t targetPlatformSpecLength = strlen(targetPlatformSpec);

    char p[targetPlatformSpecLength + 1];

    char * targetPlatformName = p;
    char * targetPlatformVers = NULL;
    char * targetPlatformArch = NULL;

    for (size_t i = 0; i < targetPlatformSpecLength; i++) {
        p[i] = targetPlatformSpec[i];

        if (p[i] == '-') {
            p[i] = '\0';

            if (targetPlatformVers == NULL) {
                targetPlatformVers = p + i + 1;
            } else if (targetPlatformArch == NULL) {
                targetPlatformArch = p + i + 1;
            }
        }
    }

    p[targetPlatformSpecLength] = '\0';

    printf("targetPlatformName=%s\n", targetPlatformName);
    printf("targetPlatformVers=%s\n", targetPlatformVers);
    printf("targetPlatformArch=%s\n", targetPlatformArch);

    //////////////////////////////////////////////////////////////////////////////

    size_t njobs;

    if (formula->support_build_in_parallel) {
        if (installOptions->parallelJobsCount > 0) {
            njobs = installOptions->parallelJobsCount;
        } else {
            njobs = sysinfo->ncpu;
        }
    } else {
        njobs = 1U;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t strBufSize = packageNameLength + 50U;
    char   strBuf[strBufSize];

    int ret = snprintf(strBuf, strBufSize, "%s:%ld:%d", packageName, ts, getpid());

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char packageInstalledSHA[65] = {0};

    ret = sha256sum_of_string(packageInstalledSHA, strBuf);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingTopDIRCapacity = sessionDIRLength + targetPlatformSpecLength + packageNameLength + 4U;
    char   packageWorkingTopDIR[packageWorkingTopDIRCapacity];

    ret = snprintf(packageWorkingTopDIR, packageWorkingTopDIRCapacity, "%s/%s-%s", sessionDIR, targetPlatformSpec, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir (packageWorkingTopDIR, S_IRWXU) != 0) {
        perror(packageWorkingTopDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    const KV toolsForNativeBuild[] = {
        { "CC",        ccForNativeBuild },
        { "OBJC",      objcForNativeBuild },
        { "CXX",       cxxForNativeBuild },
        { "CPP",       cpp },
        { "AS",        toolchain->as },
        { "AR",        toolchain->ar },
        { "RANLIB",    toolchain->ranlib },
        { "LD",        toolchain->ld },
        { "NM",        toolchain->nm },
        { "SIZE",      toolchain->size },
        { "STRIP",     toolchain->strip },
        { "STRINGS",   toolchain->strings },
        { "OBJDUMP",   toolchain->objdump },
        { NULL, NULL }
    };

    for (int i = 0; ; i++) {
        const char * name  = toolsForNativeBuild[i].name;
        const char * value = toolsForNativeBuild[i].value;

        if (name == NULL) {
            break;
        }

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            char key[20];

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (unsetenv(key) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            char key[20];

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv(key, value, 1) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    const KV flagsForNativeBuild[] = {
        { "CFLAGS",    ccFlagsForNativeBuild },
        { "CXXFLAGS", cxxFlagsForNativeBuild },
        { "CPPFLAGS", cppFlagsForNativeBuild },
        { "LDFLAGS",   ldFlagsForNativeBuild },
        { NULL, NULL }
    };

    for (int i = 0; ; i++) {
        const char * name  = flagsForNativeBuild[i].name;
        const char * value = flagsForNativeBuild[i].value;

        if (name == NULL) {
            break;
        }

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            char key[20];

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (unsetenv(key) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            char key[20];

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv(key, value, 1) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // https://keith.github.io/xcode-man-pages/xcrun.1.html

    // https://clang.llvm.org/docs/CommandGuide/clang.html#envvar-MACOSX_DEPLOYMENT_TARGET
    // https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-mmacosx-version-min
    // If -mmacosx-version-min is unspecified, the default deployment target is read from MACOSX_DEPLOYMENT_TARGET environment variable

    const char* unsetenvs[] = {
        "SDKROOT",
        "MACOSX_DEPLOYMENT_TARGET",
        "PKG_CONFIG_LIBDIR",
        "PKG_CONFIG_PATH",
        "ACLOCAL_PATH",
        "XDG_DATA_DIRS",
        "PERL5LIB",
        "LIBS",
        NULL
    };

    for (int i = 0; unsetenvs[i] != NULL; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = setenv_ACLOCAL_PATH(xcpkgCoreDIR, xcpkgCoreDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ret = setenv_XDG_DATA_DIRS(xcpkgCoreDIR, xcpkgCoreDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    // these packages are not relocatable, we need to build them from source locally.
    bool needToBuildLibtool  = false;
    bool needToBuildAutomake = false;
    bool needToBuildAutoconf = false;
    bool needToBuildTexinfo  = false;
    bool needToBuildHelp2man = false;
    bool needToBuildIntltool = false;
    bool needToBuildPerlXMLParser = false;

    size_t depPackageNamesLength = (formula->dep_upp == NULL) ? 0U : strlen(formula->dep_upp);

    size_t uppmPackageNamesCapacity = depPackageNamesLength + 100U;
    char   uppmPackageNames[uppmPackageNamesCapacity];

    ret = snprintf(uppmPackageNames, 75U, "bash coreutils findutils gsed gawk grep tree pkg-config %s", installOptions->enableCcache ? " ccache" : "");

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t uppmPackageNamesLength = ret;

    bool needToInstallGmake = false;
    bool needToInstallGm4   = false;

    if (formula->dep_upp != NULL) {
        size_t  depPackageNamesCopyCapacity = depPackageNamesLength + 1U;
        char    depPackageNamesCopy[depPackageNamesCopyCapacity];
        strncpy(depPackageNamesCopy, formula->dep_upp, depPackageNamesCopyCapacity);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, "texinfo") == 0) {
                needToBuildTexinfo = true;
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "help2man") == 0) {
                needToBuildHelp2man = true;
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "intltool") == 0) {
                needToBuildIntltool = true;
                needToInstallGmake = true;
            } else if (strcmp(depPackageName, "libtool") == 0) {
                needToBuildLibtool = true;
                needToInstallGmake = true;
                needToInstallGm4   = true;
            } else if (strcmp(depPackageName, "autoconf") == 0) {
                needToBuildAutoconf = true;
                needToInstallGmake = true;
                needToInstallGm4   = true;
            } else if (strcmp(depPackageName, "automake") == 0) {
                needToBuildAutomake = true;
                needToInstallGmake = true;
                needToInstallGm4   = true;
            } else if (strcmp(depPackageName, "perl-XML-Parser") == 0) {
                needToBuildPerlXMLParser = true;
                needToInstallGmake = true;
            } else {
                int len = snprintf(uppmPackageNames + uppmPackageNamesLength, strlen(depPackageName) + 2U, " %s", depPackageName);

                if (len < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                uppmPackageNamesLength += len;
            }

            depPackageName = strtok(NULL, " ");
        }

        if (needToInstallGmake) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " gmake", 6U);
            uppmPackageNamesLength += 6U;
        }

        if (needToInstallGm4) {
            strncpy(uppmPackageNames + uppmPackageNamesLength, " gm4", 4U);
            uppmPackageNamesLength += 4U;
        }

        uppmPackageNames[uppmPackageNamesLength] = '\0';
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = install_dependent_packages_via_uppm(xcpkgHomeDIR, xcpkgHomeDIRLength, uppmPackageNames, uppmPackageNamesLength, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRCapacity, installOptions->verbose_net);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char m4Path[PATH_MAX];

    ret = exe_where("m4", m4Path);

    switch (ret) {
        case -3:
            return XCPKG_ERROR_ENV_PATH_NOT_SET;
        case -2:
            return XCPKG_ERROR_ENV_PATH_NOT_SET;
        case -1:
            perror(NULL);
            return XCPKG_ERROR;
    }

    if (ret > 0) {
        if (setenv("M4", m4Path, 1) != 0) {
            perror("M4");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int nativePackageIDArray[22] = {0};
    int nativePackageIDArraySize = 0;

    if (needToBuildLibtool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_LIBTOOL;
        nativePackageIDArraySize++;
    }

    if (needToBuildAutoconf) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOCONF;
        nativePackageIDArraySize++;
    }

    if (needToBuildAutomake) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOMAKE;
        nativePackageIDArraySize++;
    }

    if (needToBuildTexinfo) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_TEXINFO;
        nativePackageIDArraySize++;
    }

    if (needToBuildHelp2man) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_HELP2MAN;
        nativePackageIDArraySize++;
    }

    if (needToBuildIntltool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_INTLTOOL;
        nativePackageIDArraySize++;
    }

    if (needToBuildPerlXMLParser) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_PERL_XML_PARSER;
        nativePackageIDArraySize++;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t nativePackageInstalledRootDIRCapacity = xcpkgHomeDIRLength + 8U;
    char   nativePackageInstalledRootDIR[nativePackageInstalledRootDIRCapacity];

    ret = snprintf(nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, "%s/native", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < nativePackageIDArraySize; i++) {
        for (int j = 0; ; j++) {
            const char * name  = flagsForNativeBuild[j].name;
            const char * value = flagsForNativeBuild[j].value;

            if (name == NULL) {
                break;
            }

            if (value == NULL) {
                if (unsetenv(name) != 0) {
                    perror(name);
                    return XCPKG_ERROR;
                }

                char key[20];

                ret = snprintf(key, 20, "%s_FOR_BUILD", name);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                if (unsetenv(key) != 0) {
                    perror(key);
                    return XCPKG_ERROR;
                }
            } else {
                if (setenv(name, value, 1) != 0) {
                    perror(name);
                    return XCPKG_ERROR;
                }

                char key[20];

                ret = snprintf(key, 20, "%s_FOR_BUILD", name);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                if (setenv(key, value, 1) != 0) {
                    perror(key);
                    return XCPKG_ERROR;
                }
            }
        }

        ret = install_native_package(nativePackageIDArray[i], xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, packageWorkingTopDIR, packageWorkingTopDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, njobs, installOptions);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_pym != NULL) {
        const char * const s = "/python3/bin/python3 -m pip install --upgrade ";

        size_t sLength = strlen(s);

        char* a[2] = { "pip", formula->dep_pym };

        for (int i = 0; i < 2; i++) {
            size_t pipInstallCmdCapacity = nativePackageInstalledRootDIRCapacity + sLength + strlen(a[i]);
            char   pipInstallCmd[pipInstallCmdCapacity];

            ret = snprintf(pipInstallCmd, pipInstallCmdCapacity, "%s%s%s", nativePackageInstalledRootDIR, s, a[i]);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_fork_exec(pipInstallCmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_plm != NULL) {
        size_t cpanInstallCmdLength = nativePackageInstalledRootDIRCapacity + strlen(formula->dep_plm) + 24U;
        char   cpanInstallCmd[cpanInstallCmdLength];

        ret = snprintf(cpanInstallCmd, cpanInstallCmdLength, "%s/perl/bin/cpan %s", nativePackageInstalledRootDIR, formula->dep_plm);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_fork_exec(cpanInstallCmd);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        ret = setup_rust_toolchain(installOptions, sessionDIR, sessionDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingSrcDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingSrcDIR[packageWorkingSrcDIRCapacity];

    ret = snprintf(packageWorkingSrcDIR, packageWorkingSrcDIRCapacity, "%s/src", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageWorkingSrcDIR, S_IRWXU) != 0) {
        perror(packageWorkingSrcDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingFixDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingFixDIR[packageWorkingFixDIRCapacity];

    ret = snprintf(packageWorkingFixDIR, packageWorkingFixDIRCapacity, "%s/fix", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageWorkingFixDIR, S_IRWXU) != 0) {
        perror(packageWorkingFixDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingResDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingResDIR[packageWorkingResDIRCapacity];

    ret = snprintf(packageWorkingResDIR, packageWorkingResDIRCapacity, "%s/res", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageWorkingResDIR, S_IRWXU) != 0) {
        perror(packageWorkingResDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingBinDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingBinDIR[packageWorkingBinDIRCapacity];

    ret = snprintf(packageWorkingBinDIR, packageWorkingBinDIRCapacity, "%s/bin", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageWorkingBinDIR, S_IRWXU) != 0) {
        perror(packageWorkingBinDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingIncDIRCapacity = packageWorkingTopDIRCapacity + 9U;
    char   packageWorkingIncDIR[packageWorkingIncDIRCapacity];

    ret = snprintf(packageWorkingIncDIR, packageWorkingIncDIRCapacity, "%s/include", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageWorkingIncDIR, S_IRWXU) != 0) {
        perror(packageWorkingIncDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingLibDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingLibDIR[packageWorkingLibDIRCapacity];

    ret = snprintf(packageWorkingLibDIR, packageWorkingLibDIRCapacity, "%s/lib", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageWorkingLibDIR, S_IRWXU) != 0) {
        perror(packageWorkingLibDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingTmpDIRCapacity = packageWorkingTopDIRCapacity + 5U;
    char   packageWorkingTmpDIR[packageWorkingTmpDIRCapacity];

    ret = snprintf(packageWorkingTmpDIR, packageWorkingTmpDIRCapacity, "%s/tmp", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageWorkingTmpDIR, S_IRWXU) != 0) {
        perror(packageWorkingTmpDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRootDIRCapacity = xcpkgHomeDIRLength + targetPlatformSpecLength + 15U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed/%s", xcpkgHomeDIR, targetPlatformSpec);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageMetaInfoDIRCapacity = packageInstalledDIRCapacity + sizeof(XCPKG_METADATA_DIR_PATH_RELATIVE_TO_INSTALLED_ROOT);
    char   packageMetaInfoDIR[packageMetaInfoDIRCapacity];

    ret = snprintf(packageMetaInfoDIR, packageMetaInfoDIRCapacity, "%s%s", packageInstalledDIR, XCPKG_METADATA_DIR_PATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t shellScriptFilePathCapacity = packageWorkingTopDIRCapacity + 12U;
    char   shellScriptFilePath[shellScriptFilePathCapacity];

    ret = snprintf(shellScriptFilePath, shellScriptFilePathCapacity, "%s/vars.sh", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    const bool isCrossBuild = !((strcmp("MacOSX", targetPlatformName) == 0) && (strcmp(sysinfo->arch, targetPlatformArch) == 0));

    ret = generate_shell_script_file(shellScriptFilePath, packageName, formula, installOptions, sysinfo, uppmPackageInstalledRootDIR, nativePackageInstalledRootDIR, xcpkgExeFilePath, ts, njobs, isCrossBuild, targetPlatformName, targetPlatformVers, targetPlatformArch, xcpkgHomeDIR, xcpkgCoreDIR, xcpkgDownloadsDIR, sessionDIR, packageWorkingTopDIR, packageWorkingSrcDIR, packageInstalledRootDIR, packageInstalledRootDIRCapacity, packageInstalledDIR, packageMetaInfoDIR, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringSize);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dofetch != NULL) {
        ret = xcpkg_fork_exec2(3, "/bin/sh", shellScriptFilePath, "dofetch");

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        const char * remoteRef;

        if (formula->git_sha == NULL) {
            remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
        } else {
            remoteRef = formula->git_sha;
        }

        ret = xcpkg_git_sync(packageWorkingSrcDIR, formula->git_url, remoteRef, "refs/remotes/origin/master", "master", formula->git_nth);

        if (ret != XCPKG_OK) {
            return ret;
        }
    } else {
        if (formula->src_is_dir) {
            char * srcDIR = &formula->src_url[6];
            size_t srcDIRLength = strlen(srcDIR);

            size_t cmdCapacity = srcDIRLength + packageWorkingSrcDIRCapacity + 10U;
            char   cmd[cmdCapacity];

            ret = snprintf(cmd, cmdCapacity, "cp -r %s/. %s", srcDIR, packageWorkingSrcDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_fork_exec(cmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        } else {
            ret = download_via_http(formula->src_url, formula->src_uri, formula->src_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, packageWorkingSrcDIR, packageWorkingSrcDIRCapacity, installOptions->verbose_net);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    if (formula->fix_url != NULL) {
        ret = download_via_http(formula->fix_url, formula->fix_uri, formula->fix_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, packageWorkingFixDIR, packageWorkingFixDIRCapacity, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (formula->res_url != NULL) {
        ret = download_via_http(formula->res_url, formula->res_uri, formula->res_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, packageWorkingResDIR, packageWorkingResDIRCapacity, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->do12345 != NULL) {
        ret = xcpkg_build_for_native(formula, packageName, packageNameLength, packageWorkingTopDIR, packageWorkingTopDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, packageInstalledSHA, shellScriptFilePath, shellScriptFilePathCapacity, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    ///                        below is for target                             ///
    //////////////////////////////////////////////////////////////////////////////

    const KV toolsForTargetBuild[] = {
        { "CC",        ccForTargetBuild },
        { "OBJC",      objcForTargetBuild },
        { "CXX",       cxxForTargetBuild },
        { "CPP",       cpp },
        { "AS",        toolchain->as },
        { "AR",        toolchain->ar },
        { "RANLIB",    toolchain->ranlib },
        { "LD",        toolchain->ld },
        { "NM",        toolchain->nm },
        { "SIZE",      toolchain->size },
        { "STRIP",     toolchain->strip },
        { "STRINGS",   toolchain->strings },
        { "OBJDUMP",   toolchain->objdump },

        { "XCPKG_COMPILER_C",    toolchain->cc },
        { "XCPKG_COMPILER_CXX",  toolchain->cxx },
        { "XCPKG_COMPILER_OBJC", toolchain->cc },

        { NULL, NULL }
    };

    for (int i = 0; ; i++) {
        const char * name  = toolsForTargetBuild[i].name;
        const char * value = toolsForTargetBuild[i].value;

        if (name == NULL) {
            break;
        }

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char sdkName[20];

    for (int i = 0; ; i++) {
        if (targetPlatformName[i] >= 'A' && targetPlatformName[i] <= 'Z') {
            sdkName[i] = targetPlatformName[i] + 32;
        } else {
            sdkName[i] = targetPlatformName[i];
        }

        if (targetPlatformName[i] == '\0') {
            break;
        }
    }

    char sysrootForTargetBuild[PATH_MAX]; sysrootForTargetBuild[0] = '\0';

    ret = xcpkg_sdk_path(sdkName, sysrootForTargetBuild);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (sysrootForTargetBuild[0] == '\0') {
        fprintf(stderr, "Can not locate %s sdk path.\n", targetPlatformName);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t commonflagsCapacity = strlen(sysrootForTargetBuild) + 80U;
    char   commonflags[commonflagsCapacity];

    ret = snprintf(commonflags, commonflagsCapacity, "-isysroot %s -m%s-version-min=%s -arch %s -Qunused-arguments", sysrootForTargetBuild, sdkName, targetPlatformVers, targetPlatformArch);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("XCPKG_COMPILER_ARGS", commonflags, 1) != 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ccflags == NULL) {
        size_t ccflagsCapacity = commonflagsCapacity + strlen(extraCCFlagsForTargetBuild) + 2U;
        char   ccflags[ccflagsCapacity];

        ret = snprintf(ccflags, ccflagsCapacity, "%s %s", commonflags, extraCCFlagsForTargetBuild);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CFLAGS", ccflags, 1) != 0) {
            perror("CFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t ccflagsCapacity = commonflagsCapacity + strlen(extraCCFlagsForTargetBuild) + strlen(formula->ccflags) + 2U;
        char   ccflags[ccflagsCapacity];

        ret = snprintf(ccflags, ccflagsCapacity, "%s %s %s", commonflags, extraCCFlagsForTargetBuild, formula->ccflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CFLAGS", ccflags, 1) != 0) {
            perror("CFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->xxflags == NULL) {
        size_t cxxflagsCapacity = commonflagsCapacity + strlen(extraCCFlagsForTargetBuild) + 2U;
        char   cxxflags[cxxflagsCapacity];

        ret = snprintf(cxxflags, cxxflagsCapacity, "%s %s", commonflags, extraCCFlagsForTargetBuild);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CXXFLAGS", cxxflags, 1) != 0) {
            perror("CXXFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t cxxflagsCapacity = commonflagsCapacity + strlen(extraCCFlagsForTargetBuild) + strlen(formula->xxflags) + 2U;
        char   cxxflags[cxxflagsCapacity];

        ret = snprintf(cxxflags, cxxflagsCapacity, "%s %s %s", commonflags, extraCCFlagsForTargetBuild, formula->xxflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CXXFLAGS", cxxflags, 1) != 0) {
            perror("CXXFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ppflags == NULL) {
        size_t cppflagsCapacity = packageWorkingIncDIRCapacity + 2U;
        char   cppflags[cppflagsCapacity];

        ret = snprintf(cppflags, cppflagsCapacity, "-I%s", packageWorkingIncDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CPPFLAGS", cppflags, 1) != 0) {
            perror("CPPFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t cppflagsCapacity = packageWorkingIncDIRCapacity + strlen(formula->ppflags) + 4U;
        char   cppflags[cppflagsCapacity];

        ret = snprintf(cppflags, cppflagsCapacity, "-I%s %s", packageWorkingIncDIR, formula->ppflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CPPFLAGS", cppflags, 1) != 0) {
            perror("CPPFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ldflags == NULL) {
        size_t ldflagsCapacity = commonflagsCapacity + strlen(extraLDFlagsForTargetBuild) + packageWorkingLibDIRCapacity + packageInstalledRootDIRCapacity + packageNameLength + 50U;
        char   ldflags[ldflagsCapacity];

        ret = snprintf(ldflags, ldflagsCapacity, "%s %s -L%s -Wl,-rpath,%s/%s/lib", commonflags, extraLDFlagsForTargetBuild, packageWorkingLibDIR, packageInstalledRootDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("LDFLAGS", ldflags, 1) != 0) {
            perror("LDFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t ldflagsCapacity = commonflagsCapacity + strlen(extraLDFlagsForTargetBuild) + packageWorkingLibDIRCapacity + packageInstalledRootDIRCapacity + packageNameLength + strlen(formula->ldflags) + 50U;
        char   ldflags[ldflagsCapacity];

        ret = snprintf(ldflags, ldflagsCapacity, "%s %s -L%s -Wl,-rpath,%s/%s/lib %s", commonflags, extraLDFlagsForTargetBuild, packageWorkingLibDIR, packageInstalledRootDIR, packageName, formula->ldflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("LDFLAGS", ldflags, 1) != 0) {
            perror("LDFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->support_create_mostly_statically_linked_executable && installOptions->createMostlyStaticallyLinkedExecutables && recursiveDependentPackageNamesString != NULL) {
        size_t  recursiveDependentPackageNamesStringCopyCapacity = recursiveDependentPackageNamesStringSize + 1U;
        char    recursiveDependentPackageNamesStringCopy[recursiveDependentPackageNamesStringCopyCapacity];
        strncpy(recursiveDependentPackageNamesStringCopy, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringCopyCapacity);

        char * dependentPackageName = strtok(recursiveDependentPackageNamesStringCopy, " ");

        while (dependentPackageName != NULL) {
            size_t depPkgInstalledDIRCapacity = packageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
            char   depPkgInstalledDIR[depPkgInstalledDIRCapacity];

            ret = snprintf(depPkgInstalledDIR, depPkgInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, dependentPackageName);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            setenv_fn fns[5] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

            for (int i = 0; i < 5; i++) {
                ret = fns[i](depPkgInstalledDIR, depPkgInstalledDIRCapacity);

                if (ret != XCPKG_OK) {
                    return ret;
                }
            }

            ret = copy_dependent_libraries(depPkgInstalledDIR, depPkgInstalledDIRCapacity, packageWorkingLibDIR, packageWorkingLibDIRCapacity);

            if (ret != XCPKG_OK) {
                return ret;
            }

            if (isCrossBuild) {
                size_t nativePkgInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + strlen(dependentPackageName) + 1U;
                char   nativePkgInstalledDIR[nativePkgInstalledDIRCapacity];

                ret = snprintf(nativePkgInstalledDIR, nativePkgInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, dependentPackageName);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                ret = setenv_PATH(nativePkgInstalledDIR, nativePkgInstalledDIRCapacity);

                if (ret != XCPKG_OK) {
                    return ret;
                }
            } else {
                ret = setenv_PATH(depPkgInstalledDIR, depPkgInstalledDIRCapacity);

                if (ret != XCPKG_OK) {
                    return ret;
                }
            }

            dependentPackageName = strtok(NULL, " ");
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCmake) {
        // https://cmake.org/cmake/help/latest/envvar/CMAKE_GENERATOR.html
        if (setenv("CMAKE_GENERATOR", formula->useBuildSystemNinja ? "Ninja" : "Unix Makefiles", 1) != 0) {
            perror("CMAKE_GENERATOR");
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_TYPE.html
        if (setenv("CMAKE_BUILD_TYPE", installOptions->buildType == XCPKGBuildProfile_release ? "Release" : "Debug", 1) != 0) {
            perror("CMAKE_BUILD_TYPE");
            return XCPKG_ERROR;
        }

        char ns[4];

        ret = snprintf(ns, 4, "%zu", njobs);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_PARALLEL_LEVEL.html
        if (setenv("CMAKE_BUILD_PARALLEL_LEVEL", ns, 1) != 0) {
            perror("CMAKE_BUILD_PARALLEL_LEVEL");
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_EXPORT_COMPILE_COMMANDS.html
        if (setenv("CMAKE_EXPORT_COMPILE_COMMANDS", installOptions->exportCompileCommandsJson ? "ON" : "OFF", 1) != 0) {
            perror("CMAKE_EXPORT_COMPILE_COMMANDS");
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html#manual:cmake-env-variables(7)

        const char* cmakeenvs[] = {
            "CMAKE_PREFIX_PATH",
            "CMAKE_APPLE_SILICON_PROCESSOR",
            "CMAKE_BUILD_TYPE",
            "CMAKE_CONFIGURATION_TYPES",
            "CMAKE_CONFIG_TYPE",
            "CMAKE_GENERATOR_INSTANCE",
            "CMAKE_GENERATOR_PLATFORM",
            "CMAKE_GENERATOR_TOOLSET",
            "CMAKE_INSTALL_MODE",
            "CMAKE_C_COMPILER_LAUNCHER",
            "CMAKE_C_LINKER_LAUNCHER",
            "CMAKE_CXX_COMPILER_LAUNCHER",
            "CMAKE_CXX_LINKER_LAUNCHER",
            "CMAKE_MSVCIDE_RUN_PATH",
            "CMAKE_NO_VERBOSE",
            "CMAKE_OSX_ARCHITECTURES",
            "CMAKE_TOOLCHAIN_FILE",
            "DESTDIR",
            "CTEST_INTERACTIVE_DEBUG_MODE",
            "CTEST_OUTPUT_ON_FAILURE",
            "CTEST_PARALLEL_LEVEL",
            "CTEST_PROGRESS_OUTPUT",
            "CTEST_USE_LAUNCHERS_DEFAULT",
            "DASHBOARD_TEST_FROM_CTEST",
            NULL
        };

        for (int i = 0; ; i++) {
            const char * name = cmakeenvs[i];

            if (name == NULL) {
                break;
            }

            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemGolang) {
        // https://pkg.go.dev/cmd/cgo
        KV goenvs[6] = {
            { "GO111MODULE",  "auto" },
            { "CGO_ENABLED",  "0" },
            { "CGO_CFLAGS",   getenv("CFLAGS") },
            { "CGO_CXXFLAGS", getenv("CXXFLAGS") },
            { "CGO_CPPFLAGS", getenv("CPPFLAGS") },
            { "CGO_LDFLAGS",  getenv("LDFLAGS") },
        };

        for (int i = 0; i < 6; i++) {
            const char * name  = goenvs[i].name;
            const char * value = goenvs[i].value;

            if (value == NULL) {
                value = "";
            }

            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        }

        // https://golang.org/doc/install/source#environment

        if (isCrossBuild) {
            if (setenv("GOOS", targetPlatformName, 1) != 0) {
                perror("GOOS");
                return XCPKG_ERROR;
            }

            if (setenv("GOARCH", strcmp(targetPlatformArch, "x86_64") == 0 ? "amd64" : targetPlatformArch, 1) != 0) {
                perror("GOARCH");
                return XCPKG_ERROR;
            }
        } else {
            if (unsetenv("GOOS") != 0) {
                perror("GOOS");
                return XCPKG_ERROR;
            }

            if (unsetenv("GOARCH") != 0) {
                perror("GOARCH");
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        // https://docs.rs/backtrace/latest/backtrace/
        if (setenv("RUST_BACKTRACE", "1", 1) != 0) {
            perror("RUST_BACKTRACE");
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        char ns[4];

        ret = snprintf(ns, 4, "%zu", njobs);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        // https://doc.rust-lang.org/cargo/reference/environment-variables.html
        if (setenv("CARGO_BUILD_JOBS", ns, 1) != 0) {
            perror("CARGO_BUILD_JOBS");
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        const char * rustTarget;

        if (strcmp(targetPlatformArch, "arm64") == 0) {
            rustTarget = "aarch64-apple-darwin";
        } else {
            rustTarget = "x86_64-apple-darwin";
        }

        if (setenv("RUST_TARGET", rustTarget, 1) != 0) {
            perror("RUST_TARGET");
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        char p[30];

        size_t i;

        for (i = 0U; rustTarget[i] != '\0'; i++) {
            if (rustTarget[i] == '-') {
                p[i] = '_';
            } else if (rustTarget[i] >= 'a' && rustTarget[i] <= 'z') {
                p[i] = rustTarget[i] - 32;
            } else {
                p[i] = rustTarget[i];
            }
        }

        p[i] = '\0';

        size_t envNameLinkerLength = i + 21U;
        char   envNameLinker[envNameLinkerLength];

        // https://doc.rust-lang.org/cargo/reference/environment-variables.html
        // https://doc.rust-lang.org/cargo/reference/config.html#targettriplelinker
        ret = snprintf(envNameLinker, envNameLinkerLength, "CARGO_TARGET_%s_LINKER", p);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv(envNameLinker, getenv("CC"), 1) != 0) {
            perror(envNameLinker);
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        // https://doc.rust-lang.org/cargo/reference/config.html#buildrustflags
        // we want to use RUSTFLAGS
        if (unsetenv("CARGO_ENCODED_RUSTFLAGS") != 0) {
            perror("CARGO_ENCODED_RUSTFLAGS");
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        size_t rustFlagsCapacity = strlen(getenv("CC")) + 10U;
        char   rustFlags[rustFlagsCapacity];

        ret = snprintf(rustFlags, rustFlagsCapacity, "-Clinker=%s", getenv("CC"));

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("RUSTFLAGS", rustFlags, 1) != 0) {
            perror("RUSTFLAGS");
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        const char * LDFLAGS = getenv("LDFLAGS");

        if (LDFLAGS != NULL && LDFLAGS[0] != '\0') {
            size_t  ldflagsCopyCapacity = strlen(LDFLAGS) + 1U;
            char    ldflagsCopy[ldflagsCopyCapacity];
            strncpy(ldflagsCopy, LDFLAGS, ldflagsCopyCapacity);

            char * item = strtok(ldflagsCopy, " ");

            while (item != NULL) {
                const char * const RUSTFLAGS = getenv("RUSTFLAGS");

                size_t newRUSTFLAGSCapacity = strlen(RUSTFLAGS) + strlen(item) + 13U;
                char   newRUSTFLAGS[newRUSTFLAGSCapacity];

                ret = snprintf(newRUSTFLAGS, newRUSTFLAGSCapacity, "%s -Clink-arg=%s", RUSTFLAGS, item);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                // https://doc.rust-lang.org/rustc/codegen-options/index.html#link-arg
                if (setenv("RUSTFLAGS", newRUSTFLAGS, 1) != 0) {
                    perror("RUSTFLAGS");
                    return XCPKG_ERROR;
                }

                item = strtok(NULL, " ");
            }
        }

        /////////////////////////////////////////

        // https://libraries.io/cargo/cc
        // https://crates.io/crates/cc
        // https://docs.rs/cc/latest/cc/
        // https://github.com/alexcrichton/cc-rs

        const char *    cflagsForTarget = getenv("CFLAGS");
        const char *  cxxflagsForTarget = getenv("CXXFLAGS");
        const char *  cppflagsForTarget = getenv("CPPFLAGS");

        if (cppflagsForTarget == NULL) {
            cppflagsForTarget = "";
        }

        const size_t cppflagsForTargetLength = strlen(cppflagsForTarget);

        /////////////////////////////////////////

        size_t CFLAGSForTargetCapacity = strlen(cflagsForTarget) + cppflagsForTargetLength + 2U;
        char   CFLAGSForTarget[CFLAGSForTargetCapacity];

        ret = snprintf(CFLAGSForTarget, CFLAGSForTargetCapacity, "%s %s", cflagsForTarget, cppflagsForTarget);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        size_t CXXFLAGSForTargetCapacity = strlen(cxxflagsForTarget) + cppflagsForTargetLength + 2U;
        char   CXXFLAGSForTarget[CXXFLAGSForTargetCapacity];

        ret = snprintf(CXXFLAGSForTarget, CXXFLAGSForTargetCapacity, "%s %s", cxxflagsForTarget, cppflagsForTarget);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        /////////////////////////////////////////

        KV envsForTargetBuild[5] = {
            { "TARGET_CC",       getenv("CC") },
            { "TARGET_CXX",      getenv("CXX") },
            { "TARGET_AR",       getenv("AR") },
            { "TARGET_CFLAGS",   CFLAGSForTarget },
            { "TARGET_CXXFLAGS", CXXFLAGSForTarget }
        };

        for (int i = 0; i < 5; i++) {
            if (setenv(envsForTargetBuild[i].name, envsForTargetBuild[i].value, 1) != 0) {
                perror(envsForTargetBuild[i].name);
                return XCPKG_ERROR;
            }
        }

        /////////////////////////////////////////

        if (isCrossBuild) {
            const char *    cflagsForNative = getenv("CFLAGS_FOR_BUILD");
            const char *  cxxflagsForNative = getenv("CXXFLAGS_FOR_BUILD");
            const char *  cppflagsForNative = getenv("CPPFLAGS_FOR_BUILD");

            if (cppflagsForNative == NULL) {
                cppflagsForNative = "";
            }

            const size_t cppflagsForNativeLength = strlen(cppflagsForNative);

            /////////////////////////////////////////

            size_t CFLAGSForNativeCapacity = strlen(cflagsForNative) + cppflagsForNativeLength + 2U;
            char   CFLAGSForNative[CFLAGSForNativeCapacity];

            ret = snprintf(CFLAGSForNative, CFLAGSForNativeCapacity, "%s %s", cflagsForNative, cppflagsForNative);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            /////////////////////////////////////////

            size_t CXXFLAGSForNativeCapacity = strlen(cxxflagsForNative) + cppflagsForNativeLength + 2U;
            char   CXXFLAGSForNative[CXXFLAGSForTargetCapacity];

            ret = snprintf(CXXFLAGSForNative, CXXFLAGSForNativeCapacity, "%s %s", cxxflagsForNative, cppflagsForNative);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            /////////////////////////////////////////

            KV envsForNativeBuild[5] = {
                { "HOST_CC",         toolchain->cc },
                { "HOST_CXX",        toolchain->cxx },
                { "HOST_AR",         toolchain->ar },
                { "HOST_CFLAGS",     CFLAGSForNative },
                { "HOST_CXXFLAGS",   CXXFLAGSForNative },
            };

            for (int i = 0; i < 5; i++) {
                if (setenv(envsForNativeBuild[i].name, envsForNativeBuild[i].value, 1) != 0) {
                    perror(envsForNativeBuild[i].name);
                    return XCPKG_ERROR;
                }
            }
        } else {
            KV envsForNativeBuild[5] = {
                { "HOST_CC",         getenv("TARGET_CC") },
                { "HOST_CXX",        getenv("TARGET_CXX") },
                { "HOST_AR",         getenv("TARGET_AR") },
                { "HOST_CFLAGS",     getenv("TARGET_CFLAGS") },
                { "HOST_CXXFLAGS",   getenv("TARGET_CXXFLAGS") },
            };

            for (int i = 0; i < 5; i++) {
                if (setenv(envsForNativeBuild[i].name, envsForNativeBuild[i].value, 1) != 0) {
                    perror(envsForNativeBuild[i].name);
                    return XCPKG_ERROR;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", packageWorkingLibDIR, 1) != 0) {
        perror("PKG_CONFIG_LIBDIR");
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (installOptions->dryrun) {
        const char * const SHELL = getenv("SHELL");

        if (SHELL == NULL) {
            fprintf(stderr, "SHELL environment variable is not set.\n");
            return XCPKG_ERROR;
        }

        if (SHELL[0] == '\0') {
            fprintf(stderr, "SHELL environment variable 's value should be a non-empty string.\n");
            return XCPKG_ERROR;
        }

        execl(SHELL, SHELL, NULL);

        perror(SHELL);

        return XCPKG_ERROR;
    }

    printenv();
    //////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_fork_exec2(3, "/bin/sh", shellScriptFilePath, "target");

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (packageInstalledDIR) != 0) {
        perror(packageInstalledDIR);

        if (errno == ENOENT) {
            fprintf(stderr, "nothing was installed.\n");
        }

        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* a[2] = { ".crates.toml", ".crates2.json" };

    struct stat st;

    for (int i = 0; i < 2; i++) {
        if (stat(a[i], &st) == 0) {
            if (unlink(a[i]) != 0) {
                perror(a[i]);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (mkdir(packageMetaInfoDIR, S_IRWXU) != 0) {
        if (errno != EEXIST) {
            perror(packageMetaInfoDIR);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install dependency graph files

    if (formula->dep_pkg != NULL) {
        const char* item[4] = { "dependencies.dot", "dependencies.box", "dependencies.svg", "dependencies.png" };

        for (int i = 0; i < 4; i++) {
            ret = install_files_to_metainfo_dir(st, packageWorkingTopDIR, packageWorkingTopDIRCapacity, packageMetaInfoDIR, packageMetaInfoDIRCapacity, item[i], strlen(item[i]));

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install config.log

    size_t packageBuildScriptDIRBufCapacity = packageWorkingSrcDIRCapacity + (formula->bscript == NULL ? 0U : strlen(formula->bscript)) + 2U;
    char   packageBuildScriptDIRBuf[packageBuildScriptDIRBufCapacity];

    const char * packageBuildScriptDIR;

    if (formula->bscript == NULL) {
        packageBuildScriptDIR = packageWorkingSrcDIR;
    } else {
        ret = snprintf(packageBuildScriptDIRBuf, packageBuildScriptDIRBufCapacity, "%s/%s", packageWorkingSrcDIR, formula->bscript);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        packageBuildScriptDIR = packageBuildScriptDIRBuf;
    }

    const char* b[2] = { packageWorkingTmpDIR, packageBuildScriptDIR };

    for (int i = 0; i < 2; i++) {
        size_t fromFilePathCapacity = strlen(b[i]) + 12U;
        char   fromFilePath[fromFilePathCapacity];

        ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/config.log", b[i]);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (stat(fromFilePath, &st) == 0) {
            size_t toFilePathCapacity = packageMetaInfoDIRCapacity + 12U;
            char   toFilePath[toFilePathCapacity];

            ret = snprintf(toFilePath, toFilePathCapacity, "%s/config.log", packageMetaInfoDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_rename_or_copy_file(fromFilePath, toFilePath);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install compile_commands.json

    if (installOptions->exportCompileCommandsJson) {
        for (int i = 0; i < 2; i++) {
            size_t fromFilePathCapacity = strlen(b[i]) + 23U;
            char   fromFilePath[fromFilePathCapacity];

            ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/compile_commands.json", b[i]);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (stat(fromFilePath, &st) == 0) {
                size_t toFilePathCapacity = packageMetaInfoDIRCapacity + 23U;
                char   toFilePath[toFilePathCapacity];

                ret = snprintf(toFilePath, toFilePathCapacity, "%s/compile_commands.json", packageMetaInfoDIR);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                ret = xcpkg_rename_or_copy_file(fromFilePath, toFilePath);

                if (ret != XCPKG_OK) {
                    return ret;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    
    const char* x[12] = { "AUTHORS", "LICENSE", "COPYING", "FAQ", "TODO", "NEWS", "THANKS", "CHANGELOG", "CHANGES", "README", "CONTRIBUTORS", "CONTRIBUTING" };

    const char* y[3] = { "", ".md", ".rst" };

    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 3; j++) {
            size_t itemCapacity = strlen(x[i]) + strlen(y[j]) + 1U;
            char   item[itemCapacity];

            ret = snprintf(item, itemCapacity, "%s%s", x[i], y[j]);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = install_files_to_metainfo_dir(st, packageWorkingSrcDIR, packageWorkingSrcDIRCapacity, packageMetaInfoDIR, packageMetaInfoDIRCapacity, item, itemCapacity);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    if (recursiveDependentPackageNamesString != NULL) {
        ret = backup_formulas(sessionDIR, packageMetaInfoDIR, packageMetaInfoDIRCapacity, recursiveDependentPackageNamesString, recursiveDependentPackageNamesStringSize);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    ret = generate_manifest(packageInstalledDIR);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ret = generate_receipt(packageName, formula, targetPlatformSpec, sysinfo, ts, packageMetaInfoDIR, packageMetaInfoDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t treeCmdCapacity = uppmPackageInstalledRootDIRCapacity + packageInstalledDIRCapacity + 30U;
    char   treeCmd[treeCmdCapacity];

    ret = snprintf(treeCmd, treeCmdCapacity, "%s/tree/bin/tree -a --dirsfirst %s", uppmPackageInstalledRootDIR, packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_fork_exec(treeCmd);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (chdir (packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = xcpkg_rm_rf(packageName, false, installOptions->logLevel >= XCPKGLogLevel_verbose);

                        if (ret != XCPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return XCPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return XCPKG_ERROR;
            }
        }
    }

    if (installOptions->keepSessionDIR) {
        return XCPKG_OK;
    } else {
        return xcpkg_rm_rf(packageWorkingTopDIR, false, installOptions->logLevel >= XCPKGLogLevel_verbose);
    }
}

typedef struct {
    char * packageName;
    XCPKGFormula * formula;
} XCPKGPackage;

static int getRecursiveDependentPackageNamesStringBuffer(char * packageName, XCPKGPackage ** packageSet, size_t packageSetSize, char ** precursiveDependentPackageNamesStringBuffer, size_t * precursiveDependentPackageNamesStringBufferSize, size_t * precursiveDependentPackageNamesStringBufferCapacity) {
    XCPKGPackage * package = NULL;

    for (size_t i = 0U; i < packageSetSize; i++) {
        if (strcmp(packageSet[i]->packageName, packageName) == 0) {
            package = packageSet[i];
            break;
        }
    }

    XCPKGFormula * formula = package->formula;

    if (formula->dep_pkg == NULL) {
        return XCPKG_OK;
    }

    size_t   packageNameStackCapacity = 8U;
    size_t   packageNameStackSize    = 0U;
    char * * packageNameStack = (char**)malloc(8 * sizeof(char*));

    if (packageNameStack == NULL) {
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    size_t  depPackageNamesLength = strlen(formula->dep_pkg);

    size_t  depPackageNamesCopyCapacity = depPackageNamesLength + 1U;
    char    depPackageNamesCopy[depPackageNamesCopyCapacity];
    strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyCapacity);

    char * depPackageName = strtok(depPackageNamesCopy, " ");

    while (depPackageName != NULL) {
        if (packageNameStackSize == packageNameStackCapacity) {
            char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 8U) * sizeof(char*));

            if (p == NULL) {
                free(packageNameStack);
                packageNameStack = NULL;

                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            packageNameStack = p;
            packageNameStackCapacity += 8U;
        }

        packageNameStack[packageNameStackSize] = depPackageName;
        packageNameStackSize++;
        depPackageName = strtok(NULL, " ");
    }

    ////////////////////////////////////////////////////////////////

    char * recursiveDependentPackageNamesStringBuffer        = (*precursiveDependentPackageNamesStringBuffer);
    size_t recursiveDependentPackageNamesStringBufferSize    = (*precursiveDependentPackageNamesStringBufferSize);
    size_t recursiveDependentPackageNamesStringBufferCapacity = (*precursiveDependentPackageNamesStringBufferCapacity);

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize != 0U) {
        char * packageName = packageNameStack[packageNameStackSize - 1U];
        packageNameStack[packageNameStackSize - 1U] = NULL;
        packageNameStackSize--;

        ////////////////////////////////////////////////////////////////

        if (recursiveDependentPackageNamesStringBuffer != NULL) {
            bool alreadyInRecursiveDependentPackageNamesStringBuffer = false;

            size_t  recursiveDependentPackageNamesStringBufferCopyCapacity = recursiveDependentPackageNamesStringBufferSize + 1U;
            char    recursiveDependentPackageNamesStringBufferCopy[recursiveDependentPackageNamesStringBufferCopyCapacity];
            strncpy(recursiveDependentPackageNamesStringBufferCopy, recursiveDependentPackageNamesStringBuffer, recursiveDependentPackageNamesStringBufferCopyCapacity);

            char * dependentPackageName = strtok(recursiveDependentPackageNamesStringBufferCopy, " ");

            while (dependentPackageName != NULL) {
                if (strcmp(dependentPackageName, packageName) == 0) {
                    alreadyInRecursiveDependentPackageNamesStringBuffer = true;
                    break;
                }
                dependentPackageName = strtok(NULL, " ");
            }

            if (alreadyInRecursiveDependentPackageNamesStringBuffer) {
                continue;
            }
        }

        ////////////////////////////////////////////////////////////////

        size_t packageNameLength = strlen(packageName);

        if ((recursiveDependentPackageNamesStringBufferSize + packageNameLength + 2U) > recursiveDependentPackageNamesStringBufferCapacity) {
            char * p = (char*)realloc(recursiveDependentPackageNamesStringBuffer, (recursiveDependentPackageNamesStringBufferCapacity + 256U) * sizeof(char*));

            if (p == NULL) {
                free(recursiveDependentPackageNamesStringBuffer);
                recursiveDependentPackageNamesStringBuffer = NULL;

                free(packageNameStack);
                packageNameStack = NULL;

                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            recursiveDependentPackageNamesStringBuffer = p;
            recursiveDependentPackageNamesStringBufferCapacity += 256U;
        }

        if (recursiveDependentPackageNamesStringBufferSize == 0U) {
            strncpy(recursiveDependentPackageNamesStringBuffer, packageName, packageNameLength);
            recursiveDependentPackageNamesStringBufferSize = packageNameLength;
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = '\0';
        } else {
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = ' ';
            strncpy(recursiveDependentPackageNamesStringBuffer + recursiveDependentPackageNamesStringBufferSize + 1U, packageName, packageNameLength);
            recursiveDependentPackageNamesStringBufferSize += packageNameLength + 1U;
            recursiveDependentPackageNamesStringBuffer[recursiveDependentPackageNamesStringBufferSize] = '\0';
        }

        ////////////////////////////////////////////////////////////////

        XCPKGPackage * package = NULL;

        for (size_t i = 0; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                package = packageSet[i];
                break;
            }
        }

        ////////////////////////////////////////////////////////////////

        XCPKGFormula * formula = package->formula;

        if (formula->dep_pkg != NULL) {
            size_t  depPackageNamesLength = strlen(formula->dep_pkg);

            size_t  depPackageNamesCopyLength = depPackageNamesLength + 1U;
            char    depPackageNamesCopy[depPackageNamesCopyLength];
            strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

            char * depPackageName = strtok(depPackageNamesCopy, " ");

            while (depPackageName != NULL) {
                XCPKGPackage * depPackage = NULL;

                for (size_t i = 0U; i < packageSetSize; i++) {
                    if (strcmp(packageSet[i]->packageName, depPackageName) == 0) {
                        depPackage = packageSet[i];
                        break;
                    }
                }

                ////////////////////////////////////////////////////////////////

                if (packageNameStackSize == packageNameStackCapacity) {
                    char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 8U) * sizeof(char*));

                    if (p == NULL) {
                        free(recursiveDependentPackageNamesStringBuffer);
                        recursiveDependentPackageNamesStringBuffer = NULL;

                        free(packageNameStack);
                        packageNameStack = NULL;

                        return XCPKG_ERROR_MEMORY_ALLOCATE;
                    }

                    packageNameStack = p;
                    packageNameStackCapacity += 8U;
                }

                packageNameStack[packageNameStackSize] = depPackage->packageName;
                packageNameStackSize++;

                ////////////////////////////////////////////////////////////////

                depPackageName = strtok(NULL, " ");
            }
        }
    }

    free(packageNameStack);

    (*precursiveDependentPackageNamesStringBuffer)        = recursiveDependentPackageNamesStringBuffer;
    (*precursiveDependentPackageNamesStringBufferSize)    = recursiveDependentPackageNamesStringBufferSize;
    (*precursiveDependentPackageNamesStringBufferCapacity) = recursiveDependentPackageNamesStringBufferCapacity;

    return XCPKG_OK;
}

static int check_and_read_formula_in_cache(const char * packageName, const char * targetPlatformName, const char * sessionDIR, XCPKGPackage *** ppackageSet, size_t * ppackageSetSize, size_t * ppackageSetCapacity) {
    size_t         packageSetCapacity = (*ppackageSetCapacity);
    size_t         packageSetSize     = (*ppackageSetSize);
    XCPKGPackage ** packageSet        = (*ppackageSet);

    size_t   packageNameStackCapacity = 10U;
    size_t   packageNameStackSize    = 0U;
    char * * packageNameStack = (char**)malloc(10 * sizeof(char*));

    if (packageNameStack == NULL) {
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStackSize = 1U;

    int ret = XCPKG_OK;

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0U) {
        size_t topIndex = packageNameStackSize - 1U;
        char * packageName = packageNameStack[topIndex];
        packageNameStack[topIndex] = NULL;
        packageNameStackSize--;

        XCPKGFormula * formula = NULL;

        for (size_t i = 0U; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                free(packageName);

                packageName = packageSet[i]->packageName;
                formula = packageSet[i]->formula;

                size_t lastIndex = packageSetSize - 1U;

                if (i != lastIndex) {
                    XCPKGPackage * package = packageSet[i];

                    for (size_t j = i + 1U; j < packageSetSize; j++) {
                        packageSet[j - 1] = packageSet[j];
                    }

                    packageSet[lastIndex] = package;
                }

                break;
            }
        }

        if (formula == NULL) {
            char formulaFilePath[PATH_MAX];

            ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

            if (ret != XCPKG_OK) {
                free(packageName);
                goto finalize;
            }

            size_t formulaFilePath2Length = strlen(sessionDIR) + strlen(packageName) + 6U;
            char   formulaFilePath2[formulaFilePath2Length];

            ret = snprintf(formulaFilePath2, formulaFilePath2Length, "%s/%s.yml", sessionDIR, packageName);

            if (ret < 0) {
                perror(NULL);
                free(packageName);
                return XCPKG_ERROR;
            }

            ret = xcpkg_copy_file(formulaFilePath, formulaFilePath2);

            if (ret != XCPKG_OK) {
                free(packageName);
                goto finalize;
            }

            ret = xcpkg_formula_load(packageName, targetPlatformName, formulaFilePath2, &formula);

            if (ret != XCPKG_OK) {
                free(packageName);
                goto finalize;
            }

            if (packageSetSize == packageSetCapacity) {
                XCPKGPackage ** p = (XCPKGPackage**)realloc(packageSet, (packageSetCapacity + 10U) * sizeof(XCPKGPackage*));

                if (p == NULL) {
                    free(packageName);
                    xcpkg_formula_free(formula);
                    ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageSet = p;
                packageSetCapacity += 10U;
            }

            XCPKGPackage * package = (XCPKGPackage*)malloc(sizeof(XCPKGPackage));

            if (package == NULL) {
                free(packageName);
                xcpkg_formula_free(formula);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            package->formula = formula;
            package->packageName = packageName;

            packageSet[packageSetSize] = package;
            packageSetSize++;
        }

        if (formula->dep_pkg == NULL) {
            continue;
        }

        size_t  depPackageNamesLength = strlen(formula->dep_pkg);

        size_t  depPackageNamesCopyLength = depPackageNamesLength + 1U;
        char    depPackageNamesCopy[depPackageNamesCopyLength];
        strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, packageName) == 0) {
                fprintf(stderr, "package '%s' depends itself.\n", packageName);
                ret = XCPKG_ERROR;
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 10U) * sizeof(char*));

                if (p == NULL) {
                    ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageNameStack = p;
                packageNameStackCapacity += 10U;
            }

            char * p = strdup(depPackageName);

            if (p == NULL) {
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            packageNameStack[packageNameStackSize] = p;
            packageNameStackSize++;

            depPackageName = strtok (NULL, " ");
        }
    }

finalize:
    for (size_t i = 0U; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    if (ret == XCPKG_OK) {
        (*ppackageSet) = packageSet;
        (*ppackageSetSize) = packageSetSize;
        (*ppackageSetCapacity) = packageSetCapacity;
    } else {
        for (size_t i = 0U; i < packageSetSize; i++) {
            free(packageSet[i]->packageName);
            xcpkg_formula_free(packageSet[i]->formula);

            packageSet[i]->formula = NULL;
            packageSet[i]->packageName = NULL;

            free(packageSet[i]);
            packageSet[i] = NULL;
        }
    }

    return ret;
}

static int check_if_compiler_support_Wno_error_unused_command_line_argument(const char * sessionDIR, const size_t sessionDIRLength, const char * compiler, const bool iscxx, const char * ccflags, const char * ldflags) {
    size_t testCFilePathCapacity = sessionDIRLength + 10U;
    char   testCFilePath[testCFilePathCapacity];

    int ret = snprintf(testCFilePath, testCFilePathCapacity, "%s/test.c",  sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (iscxx) {
        testCFilePath[ret]     = 'c';
        testCFilePath[ret + 1] = '\0';
    }

    int fd = open(testCFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(testCFilePath);
        return XCPKG_ERROR;
    }

    const char * testCCode = "int main() {\nreturn 0;\n}\n";

    size_t testCCodeLength = strlen(testCCode);

    ssize_t written = write(fd, testCCode, testCCodeLength);

    if (written == -1) {
        perror(testCFilePath);
        close(fd);
        return XCPKG_ERROR;
    }

    close(fd);

    if ((size_t)written != testCCodeLength) {
        fprintf(stderr, "file not fully written: %s\n", testCFilePath);
        return XCPKG_ERROR;
    }

    size_t cmdCapacity = strlen(compiler) + strlen(ccflags) + strlen(ldflags) + testCFilePathCapacity + 45U;
    char   cmd[cmdCapacity];

    ret = snprintf(cmd, cmdCapacity, "%s -Wno-error=unused-command-line-argument %s %s %s", compiler, ccflags, ldflags, testCFilePath);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    return xcpkg_fork_exec(cmd);
}

int xcpkg_install(const char * packageName, const char * targetPlatformSpec, const XCPKGInstallOptions * installOptions) {
    // redirect all stdout and stderr to /dev/null
    if (installOptions->logLevel == XCPKGLogLevel_silent) {
        int fd = open("/dev/null", O_CREAT | O_TRUNC | O_WRONLY, 0666);

        if (fd < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }

        if (dup2(fd, STDERR_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return XCPKG_ERROR_ENV_PATH_NOT_SET;
    }

    //////////////////////////////////////////////////////////////////////////////

    // https://perldoc.perl.org/cpan#PERL_MM_USE_DEFAULT
    // Would you like to configure as much as possible automatically?
    if (setenv("PERL_MM_USE_DEFAULT", "1", 1) != 0) {
        perror("PERL_MM_USE_DEFAULT");
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * unsetenvs[12] = { "TARGET_ARCH", "AUTOCONF", "AUTOHEADER", "AUTOM4TE", "AUTOMAKE", "AUTOPOINT", "ACLOCAL", "GTKDOCIZE", "INTLTOOLIZE", "LIBTOOLIZE", "M4", "MAKE" };

    for (int i = 0; i < 12; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t xcpkgCoreDIRCapacity = xcpkgHomeDIRLength + 6U;
    char   xcpkgCoreDIR[xcpkgCoreDIRCapacity];

    ret = snprintf(xcpkgCoreDIR, xcpkgCoreDIRCapacity, "%s/core", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t xcpkgDownloadsDIRCapacity = xcpkgHomeDIRLength + 11U;
    char   xcpkgDownloadsDIR[xcpkgDownloadsDIRCapacity];

    ret = snprintf(xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "%s/downloads", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(xcpkgDownloadsDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(xcpkgDownloadsDIR) != 0) {
                perror(xcpkgDownloadsDIR);
                return XCPKG_ERROR;
            }

            if (mkdir(xcpkgDownloadsDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(xcpkgDownloadsDIR);
                    return XCPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(xcpkgDownloadsDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(xcpkgDownloadsDIR);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t uppmPackageInstalledRootDIRCapacity = xcpkgHomeDIRLength + 16U;
    char   uppmPackageInstalledRootDIR[uppmPackageInstalledRootDIRCapacity];

    ret = snprintf(uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRCapacity, "%s/uppm/installed", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo sysinfo = {0};

    ret = sysinfo_make(&sysinfo);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////

    size_t capacity = xcpkgCoreDIRCapacity + 21U;

    char ccForNativeBuild[capacity];

    ret = snprintf(ccForNativeBuild, capacity, "%s/wrapper-native-cc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char cxxForNativeBuild[capacity];

    ret = snprintf(cxxForNativeBuild, capacity, "%s/wrapper-native-c++", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char objcForNativeBuild[capacity];

    ret = snprintf(objcForNativeBuild, capacity, "%s/wrapper-native-objc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    char ccForTargetBuild[capacity];

    ret = snprintf(ccForTargetBuild, capacity, "%s/wrapper-target-cc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char cxxForTargetBuild[capacity];

    ret = snprintf(cxxForTargetBuild, capacity, "%s/wrapper-target-c++", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char objcForTargetBuild[capacity];

    ret = snprintf(objcForTargetBuild, capacity, "%s/wrapper-target-objc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    XCPKGToolChain toolchain = {0};

    ret = xcpkg_toolchain_find(&toolchain);

    if (ret != XCPKG_OK) {
        return ret;
    }

    xcpkg_toolchain_dump(&toolchain);

    const KV proxiedTools[3] = {
        { "XCPKG_COMPILER_C",    toolchain.cc  },
        { "XCPKG_COMPILER_CXX",  toolchain.cxx },
        { "XCPKG_COMPILER_OBJC", toolchain.cc  }
    };

    for (int i = 0; i < 3; i++) {
        const char * name  = proxiedTools[i].name;
        const char * value = proxiedTools[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                xcpkg_toolchain_free(&toolchain);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                xcpkg_toolchain_free(&toolchain);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    size_t cppCapacity = strlen(toolchain.cc) + 4U;

    char cpp[cppCapacity];

    ret = snprintf(cpp, cppCapacity, "%s -E", toolchain.cc);

    if (ret < 0) {
        perror(NULL);
        xcpkg_toolchain_free(&toolchain);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    char sysrootForNativeBuild[PATH_MAX]; sysrootForNativeBuild[0] = '\0';

    ret = xcpkg_sdk_path("macosx", sysrootForNativeBuild);

    if (ret != XCPKG_OK) {
        xcpkg_toolchain_free(&toolchain);
        return ret;
    }

    if (sysrootForNativeBuild[0] == '\0') {
        xcpkg_toolchain_free(&toolchain);
        fprintf(stderr, "Can not locate MacOSX sdk path.\n");
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t commonFlagsForNativeBuildCapacity = strlen(sysrootForNativeBuild) + 80U;
    char   commonFlagsForNativeBuild[commonFlagsForNativeBuildCapacity];

    ret = snprintf(commonFlagsForNativeBuild, commonFlagsForNativeBuildCapacity, "-isysroot %s -mmacosx-version-min=%s -arch %s -Qunused-arguments", sysrootForNativeBuild, sysinfo.vers, sysinfo.arch);

    if (ret < 0) {
        perror(NULL);
        xcpkg_toolchain_free(&toolchain);
        return XCPKG_ERROR;
    }

    if (setenv("XCPKG_COMPILER_ARGS_FOR_BUILD", commonFlagsForNativeBuild, 1) != 0) {
        perror("XCPKG_COMPILER_ARGS_FOR_BUILD");
        xcpkg_toolchain_free(&toolchain);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t extraCCFlagsCapacity = 50U;
    char   extraCCFlags[extraCCFlagsCapacity];

    const char * s = "-fPIC -fno-common";

    char * p = extraCCFlags;

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (s[i] == '\0') {
            p = &p[i];
            break;
        }
    }

    if (installOptions->buildType == XCPKGBuildProfile_release) {
        s = " -Os";
    } else {
        s = " -g -O0";
    }

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (s[i] == '\0') {
            p = &p[i];
            break;
        }
    }

    if (installOptions->verbose_cc) {
        s = " -v";

        for (int i = 0; ; i++) {
            p[i] = s[i];

            if (s[i] == '\0') {
                p = &p[i];
                break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t extraLDFlagsCapacity = 50U;
    char   extraLDFlags[extraLDFlagsCapacity];

    s = "-Wl,-search_paths_first";

    p = extraLDFlags;

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (s[i] == '\0') {
            p = &p[i];
            break;
        }
    }

    if (installOptions->buildType == XCPKGBuildProfile_release) {
        s = " -Wl,-S";

        for (int i = 0; ; i++) {
            p[i] = s[i];

            if (s[i] == '\0') {
                p = &p[i];
                break;
            }
        }
    }

    if (installOptions->verbose_ld) {
        s = " -Wl,-v";

        for (int i = 0; ; i++) {
            p[i] = s[i];

            if (s[i] == '\0') {
                p = &p[i];
                break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char xcpkgExeFilePath[PATH_MAX];

    ret = selfpath(xcpkgExeFilePath);

    if (ret == -1) {
        perror(NULL);
        xcpkg_toolchain_free(&toolchain);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t          packageSetCapacity = 0U;
    size_t          packageSetSize     = 0U;
    XCPKGPackage ** packageSet         = NULL;

    ret = check_and_read_formula_in_cache(packageName, NULL, sessionDIR, &packageSet, &packageSetSize, &packageSetCapacity);

    if (ret != XCPKG_OK) {
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////

    printf("install packages in order: ");

    for (int i = packageSetSize - 1; i >= 0; i--) {
        XCPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;
        printf("%s ", packageName);
    }

    printf("\n");

    //////////////////////////////////////////////////////////////////////////////

    for (int i = packageSetSize - 1; i >= 0; i--) {
        XCPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;

        if (!installOptions->force) {
            ret = xcpkg_check_if_the_given_package_is_installed(packageName, targetPlatformSpec);

            if (ret == XCPKG_OK) {
                fprintf(stderr, "package already has been installed : %s\n", packageName);
                continue;
            }
        }

        if (setenv("PATH", PATH, 1) != 0) {
            perror("PATH");
            goto finalize;
        }

        char * recursiveDependentPackageNamesStringBuffer         = NULL;
        size_t recursiveDependentPackageNamesStringBufferSize     = 0U;
        size_t recursiveDependentPackageNamesStringBufferCapacity = 0U;

        ret = getRecursiveDependentPackageNamesStringBuffer(packageName, packageSet, packageSetSize, &recursiveDependentPackageNamesStringBuffer, &recursiveDependentPackageNamesStringBufferSize, &recursiveDependentPackageNamesStringBufferCapacity);

        if (ret != XCPKG_OK) {
            goto finalize;
        }

        //printf("%s:%zu:%s\n", packageName, recursiveDependentPackageNamesStringBufferSize, recursiveDependentPackageNamesStringBuffer);

        ret = xcpkg_install_package(packageName, targetPlatformSpec, package->formula, installOptions, &toolchain, &sysinfo, cpp, ccForNativeBuild, cxxForNativeBuild, objcForNativeBuild, ccForTargetBuild, cxxForTargetBuild, objcForTargetBuild, extraCCFlags, extraCCFlags, "", extraLDFlags, extraCCFlags, extraLDFlags, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRCapacity, xcpkgExeFilePath, xcpkgHomeDIR, xcpkgHomeDIRLength, xcpkgCoreDIR, xcpkgCoreDIRCapacity, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, sessionDIR, sessionDIRLength, (const char *)recursiveDependentPackageNamesStringBuffer, recursiveDependentPackageNamesStringBufferSize);

        free(recursiveDependentPackageNamesStringBuffer);
        recursiveDependentPackageNamesStringBuffer = NULL;

        if (ret != XCPKG_OK) {
            goto finalize;
        }
    }

finalize:
    xcpkg_toolchain_free(&toolchain);

    for (size_t i = 0; i < packageSetSize; i++) {
        free(packageSet[i]->packageName);
        xcpkg_formula_free(packageSet[i]->formula);

        packageSet[i]->formula = NULL;
        packageSet[i]->packageName = NULL;

        free(packageSet[i]);
        packageSet[i] = NULL;
    }

    free(packageSet);
    packageSet = NULL;

    if (ret == XCPKG_OK) {
        if (!installOptions->keepSessionDIR) {
            ret = xcpkg_rm_rf(sessionDIR, false, false);
        }
    }

    return ret;
}
