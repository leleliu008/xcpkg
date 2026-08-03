#include <stdio.h>

#include <limits.h>

#include "../xcpkg.h"

typedef struct {
    const char * key;
    const char * val;
} Map;

int xcpkg_setenv() {
    char p[PATH_MAX];

    Map maps[3] = {
        {"XCPKG_HOME", ".xcpkg"},
        {"UPPM_HOME", ".uppm"},
        {"XCPKG_DOWNLOADS_DIR", "downloads"}
    };

    for (int i = 0; i < 3; i++) {
        const char * const key = maps[i].key;

        const char * const env = getenv(key);

        if (env != NULL && env[0] != '\0') {
            continue;
        }

        const char * s;

        if (i == 2) {
            s = getenv("XCPKG_HOME");
        } else {
            s = getenv("HOME");
 
            if (s == NULL || s[0] == '\0') {
                return XCPKG_ERROR_ENV_HOME_NOT_SET;
            }
        }

        int ret = snprintf(p, PATH_MAX, "%s/%s", s, maps[i].val);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv(key, p, 1) != 0) {
            perror(key);
            return XCPKG_ERROR;
        }
    }

    return XCPKG_OK;
}
