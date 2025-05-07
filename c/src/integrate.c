#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>

#include "xcpkg.h"

int xcpkg_integrate_zsh_completion(const char * outputDIR, const bool verbose) {
    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t outputDIRLength;

    if (outputDIR == NULL) {
        outputDIR       = xcpkgHomeDIR;
        outputDIRLength = xcpkgHomeDIRLength;
    } else {
        outputDIRLength = strlen(outputDIR);

        ret = xcpkg_mkdir_p(outputDIR, verbose);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    size_t outputFilePathCapacity = outputDIRLength + 8U;
    char   outputFilePath[outputFilePathCapacity];

    ret = snprintf(outputFilePath, outputFilePathCapacity, "%s/_xcpkg", outputDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_write_file(outputFilePath, XCPKG_ZSH_COMPLETION_SCRIPT_STRING, XCPKG_ZSH_COMPLETION_SCRIPT_STRING_LENGTH);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

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
