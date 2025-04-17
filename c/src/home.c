#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_home_dir(char buf[], size_t * len) {
    if (buf == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    const char * const xcpkgHomeDIR = getenv("XCPKG_HOME");

    if (xcpkgHomeDIR == NULL) {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL) {
            return XCPKG_ERROR_ENV_HOME_NOT_SET;
        }

        if (userHomeDIR[0] == '\0') {
            return XCPKG_ERROR_ENV_HOME_NOT_SET;
        }

        char   tmpBuf[PATH_MAX];
        size_t tmpBufLength;

        for (int i = 0; ; i++) {
            if (userHomeDIR[i] == '\0') {
                tmpBufLength = i;
                break;
            } else {
                tmpBuf[i] = userHomeDIR[i];
            }
        }

        char * p = tmpBuf + tmpBufLength;

        const char * const str = "/.xcpkg";

        for (int i = 0; ; i++) {
            p[i] = str[i];

            if (str[i] == '\0') {
                tmpBufLength += i;
                break;
            }
        }

        struct stat st;

        if (stat(tmpBuf, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", tmpBuf);
                return XCPKG_ERROR;
            }
        } else {
            if (mkdir(tmpBuf, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(tmpBuf);
                    return XCPKG_ERROR;
                }
            }
        }

        strncpy(buf, tmpBuf, tmpBufLength);

        buf[tmpBufLength] = '\0';

        if (len != NULL) {
            (*len) = tmpBufLength;
        }
    } else {
        if (xcpkgHomeDIR[0] == '\0') {
            fprintf(stderr, "'XCPKG_HOME' environment variable's value was expected to be a non-empty string, but it was not.\n");
            return XCPKG_ERROR;
        }

        struct stat st;

        if (stat(xcpkgHomeDIR, &st) == 0) {
            if (!S_ISDIR(st.st_mode)) {
                fprintf(stderr, "%s was expected to be a directory, but it was not.\n", xcpkgHomeDIR);
                return XCPKG_ERROR;
            }
        } else {
            if (mkdir(xcpkgHomeDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(xcpkgHomeDIR);
                    return XCPKG_ERROR;
                }
            }
        }

        size_t n = strlen(xcpkgHomeDIR);

        strncpy(buf, xcpkgHomeDIR, n);

        buf[n] = '\0';

        if (len != NULL) {
            (*len) = n;
        }
    }

    return XCPKG_OK;
}
