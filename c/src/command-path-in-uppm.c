#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "uppm.h"

#include "xcpkg.h"

int xcpkg_get_command_path_of_uppm_package(const char * uppmPackageName, const char * cmdname, char buf[]) {
    char   uppmHomeDIR[PATH_MAX];
    size_t uppmHomeDIRLength;

    int ret = xcpkg_home_dir(uppmHomeDIR, &uppmHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    char * const p = uppmHomeDIR + uppmHomeDIRLength;

    const char * s = "/uppm";

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (p[i] == '\0') {
            uppmHomeDIRLength += i;
            break;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char commandPath[PATH_MAX];

    ret = snprintf(commandPath, PATH_MAX, "%s/installed/%s/bin/%s", uppmHomeDIR, uppmPackageName, cmdname);

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
        ret = uppm_formula_repo_sync_official_core(uppmHomeDIR, uppmHomeDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = uppm_install(uppmHomeDIR, uppmHomeDIRLength, uppmPackageName, true, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    for (int i = 0; ; i++) {
        buf[i] = commandPath[i];

        if (buf[i] == '\0') {
            break;
        }
    }

    return XCPKG_OK;
}
