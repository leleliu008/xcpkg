#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "uppm.h"

#include "xcpkg.h"

int xcpkg_get_command_path_of_uppm_package(const char * uppmPackageName, const char * cmdname, char buf[]) {
    const char * const uppmHomeDIR = getenv("UPPM_HOME");

    char commandPath[PATH_MAX];

    int ret = snprintf(commandPath, PATH_MAX, "%s/installed/%s/bin/%s", uppmHomeDIR, uppmPackageName, cmdname);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(commandPath, &st) == 0) {
        if (!S_ISREG(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", commandPath);
            return XCPKG_ERROR;
        }
    } else {
        size_t uppmHomeDIRLength = strlen(uppmHomeDIR);

        ret = uppm_formula_repo_sync_official_core(uppmHomeDIR, uppmHomeDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = uppm_install(uppmHomeDIR, uppmHomeDIRLength, uppmPackageName, false, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    for (size_t i = 0U; ; i++) {
        buf[i] = commandPath[i];

        if (buf[i] == '\0') {
            break;
        }
    }

    return XCPKG_OK;
}
