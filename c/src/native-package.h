#ifndef _NATIVE_PACKAGE
#define _NATIVE_PACKAGE

#include <stdlib.h>

#include "xcpkg.h"

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

typedef int (*NativePackageInstalledCallback)(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity);

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
        const NativePackageInstalledCallback callbak);

#endif
