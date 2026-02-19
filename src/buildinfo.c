#include <stdio.h>

#include <zlib.h>
#include <git2.h>
#include <yaml.h>
#include <jansson.h>
#include <archive.h>
#include <curl/curlver.h>
#include <openssl/opensslv.h>

//#define PCRE2_CODE_UNIT_WIDTH 8
//#include <pcre2.h>

#include "xcpkg.h"

int xcpkg_buildinfo() {
    printf("xcpkg.build.utctime: %s\n", XCPKG_BUILD_TIMESTAMP);

    //printf("pcre2   : %d.%d\n", PCRE2_MAJOR, PCRE2_MINOR);
    printf("xcpkg.build.libyaml: %s\n", yaml_get_version_string());
    printf("xcpkg.build.libcurl: %s\n", LIBCURL_VERSION);
    printf("xcpkg.build.libgit2: %s\n", LIBGIT2_VERSION);

//https://www.openssl.org/docs/man3.0/man3/OPENSSL_VERSION_BUILD_METADATA.html
//https://www.openssl.org/docs/man1.1.1/man3/OPENSSL_VERSION_TEXT.html
#ifdef OPENSSL_VERSION_STR
    printf("xcpkg.build.openssl: %s\n", OPENSSL_VERSION_STR);
#else
    printf("xcpkg.build.openssl: %s\n", OPENSSL_VERSION_TEXT);
#endif

    printf("xcpkg.build.jansson: %s\n", JANSSON_VERSION);
    printf("xcpkg.build.archive: %s\n", ARCHIVE_VERSION_ONLY_STRING);
    printf("xcpkg.build.zlib:    %s\n", ZLIB_VERSION);

    return XCPKG_OK;
}
