#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xcpkg.h"

int xcpkg_inspect_target_platform_spec(const char * targetPlatformSpec, int * hypenIndex1P, int * hypenIndex2P) {
    if (targetPlatformSpec == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (targetPlatformSpec[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    //////////////////////////////////////////////

    const char * p = targetPlatformSpec;

    int hypenIndex1 = -1;
    int hypenIndex2 = -1;

    //////////////////////////////////////////////

    for (int i = 0; ; i++) {
        if (p[i] == '\0') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return XCPKG_ERROR;
        }

        if (p[i] == '-') {
            hypenIndex1 = i;

            int support = 0;

            for (int j = 0; supportedTargetPlatformNames[j] != NULL; j++) {
                if (strncmp(p, supportedTargetPlatformNames[j], i) == 0) {
                    support = 1;
                    break;
                }
            }

            if (support == 0) {
                char buf[i];

                strncpy(buf, p, i);

                buf[i] = '\0';

                fprintf(stderr, "invalid target: %s, unsupported target platform name: %s\n", targetPlatformSpec, buf);
                return XCPKG_ERROR;
            } else {
                p += i + 1;
                break;
            }
        }
    }

    //////////////////////////////////////////////

    int k = -1;

    for (int i = 0; ; i++) {
        if (p[i] == '\0' || p[i] == '-') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return XCPKG_ERROR;
        }

        if (p[i] == '.') {
            k = i + 1;
            break;
        }

        if (p[i] < '0' || p[i] > '9') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return XCPKG_ERROR;
        }
    }

    for (int i = k; ; i++) {
        if (p[i] == '\0') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return XCPKG_ERROR;
        }

        if (p[i] == '-') {
            p += i + 1;
            hypenIndex2 = i;
            break;
        }

        if (p[i] < '0' || p[i] > '9') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////

    for (int i = 0; ; i++) {
        if (p[i] == '-') {
            fprintf(stderr, "invalid target: %s\n", targetPlatformSpec);
            return XCPKG_ERROR;
        }

        if (p[i] == '\0') {
            break;
        }
    }

    (*hypenIndex1P) = hypenIndex1;
    (*hypenIndex2P) = hypenIndex2;

    return XCPKG_OK;
}

int main2() {
    const char * p = "MacOSX-10.15-arm64";

    int x = 0;
    int y = 0;

    int ret = xcpkg_inspect_target_platform_spec(p, &x, &y);

    if (ret != XCPKG_OK) {
        return ret;
    }

    char targetPlatformName[x];

    strncpy(targetPlatformName, p, x);
    targetPlatformName[x] = '\0';

    p += x + 1;

    char targetPlatformVers[y];

    strncpy(targetPlatformVers, p, y);
    targetPlatformVers[y] = '\0';

    p += y + 1;

    int z = strlen(p) - x - y;

    char targetPlatformArch[z];

    strncpy(targetPlatformArch, p, z);
    targetPlatformArch[z] = '\0';

    printf("targetPlatformName=%s\n", targetPlatformName);
    printf("targetPlatformVers=%s\n", targetPlatformVers);
    printf("targetPlatformArch=%s\n", targetPlatformArch);

    return 0;
}
