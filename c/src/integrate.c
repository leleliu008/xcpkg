#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_integrate_zsh_completion(const char * outputDIR, const bool verbose) {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = xcpkg_home_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + 8U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/_xcpkg", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    const char * const url = "https://raw.githubusercontent.com/leleliu008/xcpkg/master/xcpkg-zsh-completion";

    ret = xcpkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t defaultOutputDIRCapacity = xcpkgHomeDIRLength + 26U;
    char   defaultOutputDIR[defaultOutputDIRCapacity];

    ret = snprintf(defaultOutputDIR, defaultOutputDIRCapacity, "%s/share/zsh/site-functions", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t outputDIRLength;

    if (outputDIR == NULL) {
        outputDIR       = defaultOutputDIR;
        outputDIRLength = ret;
    } else {
        outputDIRLength = strlen(outputDIR);
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_mkdir_p(outputDIR, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t outputFilePathCapacity = outputDIRLength + 8U;
    char   outputFilePath[outputFilePathCapacity];

    ret = snprintf(outputFilePath, outputFilePathCapacity, "%s/_xcpkg", outputDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (rename(tmpFilePath, outputFilePath) != 0) {
        if (errno == EXDEV) {
            ret = xcpkg_copy_file(tmpFilePath, outputFilePath);

            if (ret != XCPKG_OK) {
                return ret;
            }
        } else {
            perror(outputFilePath);
            return XCPKG_ERROR;
        }
    }

    printf("zsh completion script for xcpkg has been written to %s\n", outputFilePath);
    return XCPKG_OK;
}

int xcpkg_integrate_bash_completion(const char * outputDIR, const bool verbose) {
    (void)outputDIR;
    (void)verbose;
    return XCPKG_OK;
}

int xcpkg_integrate_fish_completion(const char * outputDIR, const bool verbose) {
    (void)outputDIR;
    (void)verbose;
    return XCPKG_OK;
}
