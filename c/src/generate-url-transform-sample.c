#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/log.h"

#include "xcpkg.h"

int xcpkg_generate_url_transform_sample() {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    const char * const p = ""
        "#!/bin/sh\n"
        "case $1 in\n"
        "    *githubusercontent.com/*)\n"
        "        printf 'https://ghfast.top/%s\\n' \"$1\"\n"
        "        ;;\n"
        "    https://github.com/*)\n"
        "        printf 'https://ghfast.top/%s\\n' \"$1\"\n"
        "        ;;\n"
        "    '') printf '%s\\n' \"$0 <URL>, <URL> is unspecified.\" >&2 ; exit 1 ;;\n"
        "    *)  printf '%s\\n' \"$1\"\n"
        "esac";

    size_t tmpFilePathCapacity = sessionDIRLength + 22U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/url-transform.sample", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_write_file(tmpFilePath, p, 0);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chmod(tmpFilePath, S_IRWXU) != 0) {
        perror(tmpFilePath);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t outFilePathCapacity = xcpkgHomeDIRLength + 22U;
    char   outFilePath[outFilePathCapacity];

    ret = snprintf(outFilePath, outFilePathCapacity, "%s/url-transform.sample", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (rename(tmpFilePath, outFilePath) != 0) {
        if (errno == EXDEV) {
            ret = xcpkg_copy_file(tmpFilePath, outFilePath);

            if (ret != XCPKG_OK) {
                return ret;
            }
        } else {
            perror(tmpFilePath);
            return XCPKG_ERROR;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "%surl-transform sample has been written into %s%s\n\n", COLOR_GREEN, outFilePath, COLOR_OFF);

    outFilePath[outFilePathCapacity - 9U] = '\0';

    fprintf(stderr, "%sYou can rename url-transform.sample to url-transform then edit it to meet your needs.\n\nTo apply this, you should run 'export XCPKG_URL_TRANSFORM=%s' in your terminal.\n%s", COLOR_GREEN, outFilePath, COLOR_OFF);

    return xcpkg_rm_rf(sessionDIR, false, false);
}
