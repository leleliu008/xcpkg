#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include <sys/stat.h>

#include "../base/sha256sum.h"

#include "native-package.h"

#define BUILD_SYSTEM_CONFIGURE 1
#define BUILD_SYSTEM_CMAKE     2
#define BUILD_SYSTEM_MESON     3

static int getNativePackageInfoByID(const int packageID, NativePackage * package) {
    switch (packageID) {
        case NATIVE_PACKAGE_ID_LIBZ:
            package->name = "libz";
            package->srcUrl = "https://distfiles.macports.org/zlib/zlib-1.3.2.tar.xz";
            package->srcSha = "d7a0654783a4da529d1bb793b7ad9c3318020af77667bcae35f95d0e42a792f3";
            package->buildArgs = "-DZLIB_BUILD_TESTING=OFF -DZLIB_BUILD_STATIC=ON -DZLIB_BUILD_SHARED=ON";
            package->buildSystem = BUILD_SYSTEM_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_OPENSSL:
            package->name = "openssl";
            package->srcUrl = "https://github.com/openssl/openssl/releases/download/openssl-3.6.3/openssl-3.6.3.tar.gz";
            package->srcSha = "243a86649cf6f23eeb6a2ff2456e09e5d77dd9018a54d3d96b0c6bdd6ba6c7f1";
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBEXPAT:
            package->name = "libexpat";
            package->srcUrl = "https://github.com/libexpat/libexpat/releases/download/R_2_7_4/expat-2.7.4.tar.lz";
            package->srcSha = "882bb3c124cdfd6d594818276f3ea851b780473a722385150a5793277635fcae";
            package->buildArgs = "-DEXPAT_BUILD_DOCS=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_FUZZERS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TOOLS=OFF";
            package->buildSystem = BUILD_SYSTEM_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_LIBXML2:
            package->name = "libxml2";
            package->srcUrl = "https://download.gnome.org/sources/libxml2/2.15/libxml2-2.15.3.tar.xz";
            package->srcSha = "78262a6e7ac170d6528ebfe2efccdf220191a5af6a6cd61ea4a9a9a5042c7a07";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_LIBZ;
            package->buildArgs = "--with-legacy --with-zlib --without-python --without-iconv --without-lzma --without-readline --without-coverage --without-debug --enable-ipv6 --enable-static --enable-shared";
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;

        case NATIVE_PACKAGE_ID_LIBTOOL:
            package->name = "libtool";
            package->srcUrl = "https://ftp.gnu.org/gnu/libtool/libtool-2.5.4.tar.xz";
            package->srcSha = "f81f5860666b0bc7d84baddefa60d1cb9fa6fceb2398cc3baca6afaa60266675";
            package->buildArgs = "--disable-ltdl-install";
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_TEXINFO:
            package->name = "texinfo";
            package->srcUrl = "https://ftp.gnu.org/gnu/texinfo/texinfo-7.3.tar.xz";
            package->srcSha = "51f74eb0f51cfa9873b85264dfdd5d46e8957ec95b88f0fb762f63d9e164c72e";
            package->buildArgs = "--with-included-regex --enable-threads=posix --disable-nls";
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOCONF:
            package->name = "autoconf";
            package->srcUrl = "https://ftp.gnu.org/gnu/autoconf/autoconf-2.73.tar.gz";
            package->srcSha = "259ddfa3bddc799cfb81489cc0f17dfdf1bd6d1505dda53c0f45ff60d6a4f9a7";
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOMAKE:
            package->name = "automake";
            package->srcUrl = "https://ftp.gnu.org/gnu/automake/automake-1.18.1.tar.xz";
            package->srcSha = "168aa363278351b89af56684448f525a5bce5079d0b6842bd910fdd3f1646887";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_AUTOCONF;
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_HELP2MAN:
            package->name = "help2man";
            package->srcUrl = "https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz";
            package->srcSha = "4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f";
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_INTLTOOL:
            package->name = "intltool";
            package->srcUrl = "https://distfiles.macports.org/intltool/intltool-0.51.0.tar.gz";
            package->srcSha = "67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL_XML_PARSER;
            package->buildArgs = NULL;
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_PERL_XML_PARSER:
            package->name = "perl-XML-Parser";
            package->srcUrl = "https://cpan.metacpan.org/authors/id/T/TO/TODDR/XML-Parser-2.47.tar.gz";
            package->srcSha = "ad4aae643ec784f489b956abe952432871a622d4e2b5c619e8855accbfc4d1d8";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_LIBEXPAT;
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_ITSTOOL:
            package->name = "itstool";
            package->srcUrl = "https://files.itstool.org/itstool/itstool-2.0.7.tar.bz2";
            package->srcSha = "6b9a7cd29a12bb95598f5750e8763cee78836a1a207f85b74d8b3275b27e87ca";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_LIBXML2;
            package->buildArgs = NULL;
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_GTK_DOC:
            package->name = "gtk-doc";
            package->srcUrl = "https://download.gnome.org/sources/gtk-doc/1.35/gtk-doc-1.35.1.tar.xz";
            package->srcSha = "611c9f24edd6d88a8ae9a79d73ab0dc63c89b81e90ecc31d6b9005c5f05b25e2";
            package->buildArgs = "-Dtests=false -Dyelp_manual=false";
            package->buildSystem = BUILD_SYSTEM_MESON;
            break;
        case NATIVE_PACKAGE_ID_AUTOCONF_ARCHIVE:
            package->name = "autoconf-archive";
            package->srcUrl = "https://ftp.gnu.org/gnu/autoconf-archive/autoconf-archive-2024.10.16.tar.xz";
            package->srcSha = "7bcd5d001916f3a50ed7436f4f700e3d2b1bade3ed803219c592d62502a57363";
            package->buildArgs = NULL;
            package->buildSystem = BUILD_SYSTEM_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_NETSURF_BUILDSYSTEM:
            package->name = "netsurf_buildsystem";
            package->srcUrl = "https://download.netsurf-browser.org/libs/releases/buildsystem-1.10.tar.gz";
            package->srcSha = "3d3e39d569e44677c4b179129bde614c65798e2b3e6253160239d1fd6eae4d79";
            break;
        default:
            fprintf(stderr, "unknown native package id: %d\n", packageID);
            return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

int install_native_package(
        const int packageID,
        const char * downloadsDIR,
        const size_t downloadsDIRLength,
        const char * sessionDIR,
        const size_t sessionDIRCapacity,
        const char * packageInstalledRootDIR,
        const size_t packageInstalledRootDIRCapacity,
        const size_t njobs,
        const XCPKGInstallOptions * installOptions,
        const NativePackageInstalledCallback callback) {
    NativePackage package = {0};

    int ret = getNativePackageInfoByID(packageID, &package);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (packageID == NATIVE_PACKAGE_ID_INTLTOOL) {
        size_t perl5LibDIRCapacity = packageInstalledRootDIRCapacity + 27U;
        char   perl5LibDIR[perl5LibDIRCapacity];

        ret = snprintf(perl5LibDIR, perl5LibDIRCapacity, "%s/perl-XML-Parser/lib/perl5", packageInstalledRootDIR);

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

    for (int i = 0; i < 10; i++) {
        if (package.depPackageIDArray[i] == 0) {
            break;
        }

        ret = install_native_package(package.depPackageIDArray[i], downloadsDIR, downloadsDIRLength, sessionDIR, sessionDIRCapacity, packageInstalledRootDIR, packageInstalledRootDIRCapacity, njobs, installOptions, callback);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * packageName = package.name;
    const char * srcUrl      = package.srcUrl;
    const char * srcUri      = package.srcUri;
    const char * srcSha      = package.srcSha;
    const char * buildArgs   = package.buildArgs;
    int          buildSystem = package.buildSystem;

    if (buildArgs == NULL) {
        buildArgs = "";
    }

    printf("native package '%s' is being installed.\n", packageName);

    //////////////////////////////////////////////////////////////////////////////

    size_t packageNameLength = strlen(packageName);

    size_t receiptFilePathLength = packageInstalledRootDIRCapacity + packageNameLength + 14U;
    char   receiptFilePath[receiptFilePathLength];

    ret = snprintf(receiptFilePath, receiptFilePathLength, "%s/%s/receipt.txt", packageInstalledRootDIR, packageName);

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

                size_t packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + packageNameLength + 2U;
                char   packageInstalledDIR[packageInstalledDIRCapacity];

                ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, packageName);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                return callback(packageInstalledDIR, packageInstalledDIRCapacity);
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", receiptFilePath);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingTopDIRLength = sessionDIRCapacity + packageNameLength + 14U;
    char   packageWorkingTopDIR[packageWorkingTopDIRLength];

    ret = snprintf(packageWorkingTopDIR, packageWorkingTopDIRLength, "%s/native-build-%s", sessionDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t packageWorkingSrcDIRLength = packageWorkingTopDIRLength + 5U;
    char   packageWorkingSrcDIR[packageWorkingSrcDIRLength];

    ret = snprintf(packageWorkingSrcDIR, packageWorkingSrcDIRLength, "%s/src", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_http_fetch_then_unpack(srcUrl, srcUri, srcSha, downloadsDIR, downloadsDIRLength, packageWorkingSrcDIR, packageWorkingSrcDIRLength, installOptions->verbose_net);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(packageWorkingSrcDIR) != 0) {
        perror(packageWorkingSrcDIR);
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

    size_t packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t packageWorkingLibDIRLength = packageWorkingTopDIRLength + 5U;
    char   packageWorkingLibDIR[packageWorkingLibDIRLength];

    ret = snprintf(packageWorkingLibDIR, packageWorkingLibDIRLength, "%s/lib", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t packageWorkingIncludeDIRLength = packageWorkingTopDIRLength + 9U;
    char   packageWorkingIncludeDIR[packageWorkingIncludeDIRLength];

    ret = snprintf(packageWorkingIncludeDIR, packageWorkingIncludeDIRLength, "%s/include", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t packageWorkingPkgconfigDIRLength = packageWorkingTopDIRLength + 15U;
    char   packageWorkingPkgconfigDIR[packageWorkingPkgconfigDIRLength];

    ret = snprintf(packageWorkingPkgconfigDIR, packageWorkingPkgconfigDIRLength, "%s/lib/pkgconfig", packageWorkingTopDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", packageWorkingLibDIR, 1) != 0) {
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

    if (packageID == NATIVE_PACKAGE_ID_TEXINFO) {
        if (setenv("PERL_EXT_CC", getenv("CC"), 1) != 0) {
            perror("PERL_EXT_CC");
            return XCPKG_ERROR;
        }

        if (setenv("PERL_EXT_CFLAGS", getenv("CFLAGS"), 1) != 0) {
            perror("PERL_EXT_CFLAGS");
            return XCPKG_ERROR;
        }

        if (setenv("PERL_EXT_CPPFLAGS", getenv("CPPFLAGS"), 1) != 0) {
            perror("PERL_EXT_CPPFLAGS");
            return XCPKG_ERROR;
        }

        if (setenv("PERL_EXT_LDFLAGS", getenv("LDFLAGS"), 1) != 0) {
            perror("PERL_EXT_LDFLAGS");
            return XCPKG_ERROR;
        }
    } else if (packageID == NATIVE_PACKAGE_ID_PERL_XML_PARSER) {
        int fd = open("configure", O_CREAT | O_TRUNC | O_WRONLY, 0755);

        if (fd == -1) {
            perror("configure");
            return XCPKG_ERROR;
        }

        int ret = dprintf(fd, "#!/bin/sh\nset -ex\n");

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "export EXPATLIBPATH='%s/libexpat/lib'\n", packageInstalledRootDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "export EXPATINCPATH='%s/libexpat/include'\n", packageInstalledRootDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "gsed -i '/check_lib/a not_execute,' Makefile.PL\n");

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "install -d %s\n", packageInstalledDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "perl Makefile.PL INSTALL_BASE=%s\n", packageInstalledDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        close(fd);
    }

    //////////////////////////////////////////////////////////////////////////////

    if (buildSystem == BUILD_SYSTEM_CMAKE) {
        size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildArgs) + 164U;
        char   configurePhaseCmd[configurePhaseCmdLength];

        ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=%s -DEXPAT_SHARED_LIBS=OFF -DCMAKE_VERBOSE_MAKEFILE=%s %s -G Ninja -S . -B build.d", packageInstalledDIR, (installOptions->logLevel >= XCPKGLogLevel_verbose) ? "ON" : "OFF", buildArgs);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_posix_spawn(configurePhaseCmd);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_posix_spawn2(3, "cmake", "--build", "build.d");

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_posix_spawn2(3, "cmake", "--install", "build.d");

        if (ret != XCPKG_OK) {
            return ret;
        }
    } else if (buildSystem == BUILD_SYSTEM_MESON) {
        if (packageID == NATIVE_PACKAGE_ID_GTK_DOC) {
            ret = xcpkg_posix_spawn2(9, "python3", "-m", "pip", "install", "--upgrade", "pip", "meson", "pygments", "lxml");

            if (ret != XCPKG_OK) {
                return ret;
            }
        }

        size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildArgs) + 110U;
        char   configurePhaseCmd[configurePhaseCmdLength];

        ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "meson setup --buildtype=release --backend=ninja -Ddefault_library=both -Dlibdir=lib --prefix=%s %s %s build.d .", packageInstalledDIR, (installOptions->logLevel >= XCPKGLogLevel_verbose) ? "-v" : "", buildArgs);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_posix_spawn(configurePhaseCmd);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_posix_spawn2(4, "meson", "compile", "-C", "build.d");

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_posix_spawn2(4, "meson", "install", "-C", "build.d");

        if (ret != XCPKG_OK) {
            return ret;
        }
    } else if (buildSystem == BUILD_SYSTEM_CONFIGURE) {
        if (packageID == NATIVE_PACKAGE_ID_OPENSSL) {
            size_t configurePhaseCmdLength = (packageInstalledDIRCapacity * 3) + 100U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./config no-tests no-ssl3 no-ssl3-method no-zlib --prefix=%s --libdir=%s/lib --openssldir=%s/etc/ssl", packageInstalledDIR, packageInstalledDIR, packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_posix_spawn(configurePhaseCmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        } else {
            size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildArgs) + 32U;
            char   configurePhaseCmd[configurePhaseCmdLength];

            ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "./configure --prefix=%s %s %s", packageInstalledDIR, (installOptions->logLevel == XCPKGLogLevel_silent) ? "--silent" : "", buildArgs);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_posix_spawn(configurePhaseCmd);

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

        ret = xcpkg_posix_spawn(buildPhaseCmd);

        if (ret != XCPKG_OK) {
            return ret;
        }

        //////////////////////////////////////////////////////////////////////////////

        ret = xcpkg_posix_spawn2(2, "gmake", "install");

        if (ret != XCPKG_OK) {
            return ret;
        }
    } else {
        if (packageID == NATIVE_PACKAGE_ID_NETSURF_BUILDSYSTEM) {
            size_t cmdCapacity = packageInstalledDIRCapacity + 22U;
            char   cmd[cmdCapacity];

            ret = snprintf(cmd, cmdCapacity, "gmake install PREFIX=%s", packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_posix_spawn(cmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(packageInstalledDIR) != 0) {
        perror(packageInstalledDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    switch (packageID) {
        case NATIVE_PACKAGE_ID_LIBTOOL:
            if (symlink("libtool", "bin/glibtool") == -1) {
                perror("libtool");
                return XCPKG_ERROR;
            }
            if (symlink("libtoolize", "bin/glibtoolize") == -1) {
                perror("libtoolize");
                return XCPKG_ERROR;
            }
            break;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_write_file("receipt.txt", srcSha, 64U);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
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

    return callback(packageInstalledDIR, packageInstalledDIRCapacity);
}
