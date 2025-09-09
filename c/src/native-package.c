#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "sha256sum.h"

#include "native-package.h"

#define BUILD_SYSTEM_TYPE_CMAKE     1
#define BUILD_SYSTEM_TYPE_CONFIGURE 2

static int getNativePackageInfoByID(const int packageID, NativePackage * package) {
    switch (packageID) {
        case NATIVE_PACKAGE_ID_LIBZ:
            package->name = "libz";
            package->srcUrl = "https://zlib.net/zlib-1.3.1.tar.gz";
            package->srcSha = "9a93b2b7dfdac77ceba5a558a580e74667dd6fede4585b91eefb60f03b72df23";
            package->buildConfigureArgs = "-DZLIB_BUILD_EXAMPLES=OFF";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_OPENSSL:
            package->name = "openssl";
            package->srcUrl = "https://www.openssl.org/source/openssl-3.4.1.tar.gz";
            package->srcSha = "002a2d6b30b58bf4bea46c43bdd96365aaf8daa6c428782aa4feee06da197df3";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_LIBEXPAT:
            package->name = "libexpat";
            package->srcUrl = "https://github.com/libexpat/libexpat/releases/download/R_2_6_4/expat-2.6.4.tar.xz";
            package->srcSha = "a695629dae047055b37d50a0ff4776d1d45d0a4c842cf4ccee158441f55ff7ee";
            package->buildConfigureArgs = "-DEXPAT_BUILD_DOCS=OFF -DEXPAT_BUILD_TESTS=OFF -DEXPAT_BUILD_FUZZERS=OFF -DEXPAT_BUILD_EXAMPLES=OFF -DEXPAT_BUILD_TOOLS=OFF";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CMAKE;
            break;
        case NATIVE_PACKAGE_ID_LIBTOOL:
            package->name = "libtool";
            package->srcUrl = "https://ftp.gnu.org/gnu/libtool/libtool-2.5.4.tar.xz";
            package->srcSha = "f81f5860666b0bc7d84baddefa60d1cb9fa6fceb2398cc3baca6afaa60266675";
            package->buildConfigureArgs = "--enable-ltdl-install";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_TEXINFO:
            package->name = "texinfo";
            package->srcUrl = "https://ftp.gnu.org/gnu/texinfo/texinfo-7.2.tar.xz";
            package->srcSha = "0329d7788fbef113fa82cb80889ca197a344ce0df7646fe000974c5d714363a6";
            package->buildConfigureArgs = "--with-included-regex --enable-threads=posix --disable-nls";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOCONF:
            package->name = "autoconf";
            package->srcUrl = "https://ftp.gnu.org/gnu/autoconf/autoconf-2.72.tar.gz";
            package->srcSha = "afb181a76e1ee72832f6581c0eddf8df032b83e2e0239ef79ebedc4467d92d6e";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOMAKE:
            package->name = "automake";
            package->srcUrl = "https://ftp.gnu.org/gnu/automake/automake-1.17.tar.xz";
            package->srcSha = "8920c1fc411e13b90bf704ef9db6f29d540e76d232cb3b2c9f4dc4cc599bd990";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_AUTOCONF;
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_HELP2MAN:
            package->name = "help2man";
            package->srcUrl = "https://ftp.gnu.org/gnu/help2man/help2man-1.49.3.tar.xz";
            package->srcSha = "4d7e4fdef2eca6afe07a2682151cea78781e0a4e8f9622142d9f70c083a2fd4f";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_INTLTOOL:
            package->name = "intltool";
            package->srcUrl = "https://launchpad.net/intltool/trunk/0.51.0/+download/intltool-0.51.0.tar.gz";
            package->srcSha = "67c74d94196b153b774ab9f89b2fa6c6ba79352407037c8c14d5aeb334e959cd";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_PERL_XML_PARSER;
            package->buildConfigureArgs = "";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_PERL_XML_PARSER:
            package->name = "perl-XML-Parser";
            package->srcUrl = "https://cpan.metacpan.org/authors/id/T/TO/TODDR/XML-Parser-2.46.tar.gz";
            package->srcSha = "d331332491c51cccfb4cb94ffc44f9cd73378e618498d4a37df9e043661c515d";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_LIBEXPAT;
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_ITSTOOL:
            package->name = "itstool";
            package->srcUrl = "https://files.itstool.org/itstool/itstool-2.0.7.tar.bz2";
            package->srcSha = "6b9a7cd29a12bb95598f5750e8763cee78836a1a207f85b74d8b3275b27e87ca";
            package->depPackageIDArray[0] = NATIVE_PACKAGE_ID_LIBXML2;
            package->buildConfigureArgs = "";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
            break;
        case NATIVE_PACKAGE_ID_AUTOCONF_ARCHIVE:
            package->name = "autoconf-archive";
            package->srcUrl = "https://ftp.gnu.org/gnu/autoconf-archive/autoconf-archive-2024.10.16.tar.xz";
            package->srcSha = "7bcd5d001916f3a50ed7436f4f700e3d2b1bade3ed803219c592d62502a57363";
            package->buildConfigureArgs = "";
            package->buildSystemType = BUILD_SYSTEM_TYPE_CONFIGURE;
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
    const char * srcUrl = package.srcUrl;
    const char * srcUri = package.srcUri;
    const char * srcSha = package.srcSha;
    const char * buildConfigureArgs = package.buildConfigureArgs;
    int          buildSystemType = package.buildSystemType;

    if (buildConfigureArgs == NULL) {
        buildConfigureArgs = "";
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

    if (buildSystemType == BUILD_SYSTEM_TYPE_CMAKE) {
        size_t configurePhaseCmdLength = packageInstalledDIRCapacity + strlen(buildConfigureArgs) + 164U;
        char   configurePhaseCmd[configurePhaseCmdLength];

        ret = snprintf(configurePhaseCmd, configurePhaseCmdLength, "cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_LIBDIR=lib -DCMAKE_INSTALL_PREFIX=%s -DEXPAT_SHARED_LIBS=OFF -DCMAKE_VERBOSE_MAKEFILE=%s %s -G Ninja -S . -B build.d", packageInstalledDIR, (installOptions->logLevel >= XCPKGLogLevel_verbose) ? "ON" : "OFF", buildConfigureArgs);

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
        if (packageID == NATIVE_PACKAGE_ID_OPENSSL) {
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
    } else {
        if (packageID == NATIVE_PACKAGE_ID_NETSURF_BUILDSYSTEM) {
            size_t cmdCapacity = packageInstalledDIRCapacity + 22U;
            char   cmd[cmdCapacity];

            ret = snprintf(cmd, cmdCapacity, "gmake install PREFIX=%s", packageInstalledDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_fork_exec(cmd);

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
