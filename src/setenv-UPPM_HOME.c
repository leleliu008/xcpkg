#include <stdio.h>

#include <limits.h>

#include "xcpkg.h"

int xcpkg_setenv_UPPM_HOME() {
    if (getenv("UPPM_HOME") != NULL) {
        return XCPKG_OK;
    }

    const char * const userHomeDIR = getenv("HOME");

    if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
        return XCPKG_ERROR_ENV_HOME_NOT_SET;
    }

    char uppmHomeDIR[PATH_MAX];

    int ret = snprintf(uppmHomeDIR, PATH_MAX, "%s/.uppm", userHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("UPPM_HOME", uppmHomeDIR, 1) != 0) {
        perror("UPPM_HOME");
        return XCPKG_ERROR;
    } else {
        return XCPKG_OK;
    }
}
