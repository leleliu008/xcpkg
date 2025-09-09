#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#define ACTION_E      1
#define ACTION_S      2
#define ACTION_c      3
#define ACTION_shared 4

int main(int argc, char * argv[]) {
    char * const compiler = getenv("XCPKG_OBJC");

    if (compiler == NULL) {
        fprintf(stderr, "XCPKG_OBJC environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "XCPKG_OBJC environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    char * const baseArgs = getenv("XCPKG_NATIVE_FLAGS");

    if (baseArgs == NULL) {
        fprintf(stderr, "XCPKG_NATIVE_FLAGS environment variable is not set.\n");
        return 5;
    }

    if (baseArgs[0] == '\0') {
        fprintf(stderr, "XCPKG_NATIVE_FLAGS environment variable value should be a non-empty string.\n");
        return 6;
    }

    /////////////////////////////////////////////////////////////////

    int action = 0;

    int i;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-E") == 0) {
            action = ACTION_E;
            break;
        }

        if (strcmp(argv[i], "-S") == 0) {
            action = ACTION_S;
            break;
        }

        if (strcmp(argv[i], "-c") == 0) {
            action = ACTION_c;
            break;
        }

        if (strcmp(argv[i], "-dynamiclib") == 0) {
            action = ACTION_shared;
            break;
        }

        if (strcmp(argv[i], "-shared") == 0) {
            action = ACTION_shared;
            break;
        }
    }

    /////////////////////////////////////////////////////////////////

    size_t n1 = 0U;

    char * p = baseArgs;

    while (p[0] != '\0') {
        if (p[0] == ' ') {
            p++;
            continue;
        }

        n1++;

        for (;;) {
            p++;

            if (p[0] == '\0') {
                break;
            }

            if (p[0] == ' ') {
                p++;
                break;
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    size_t n2 = 0U;

    char * ccflags = NULL;

    if (action == 0 || action == ACTION_c) {
        ccflags = getenv("XCPKG_NATIVE_OBJCFLAGS");

        if (ccflags != NULL) {
            p = ccflags;

            while (p[0] != '\0') {
                if (p[0] == ' ') {
                    p++;
                    continue;
                }

                n2++;

                for (;;) {
                    p++;

                    if (p[0] == '\0') {
                        break;
                    }

                    if (p[0] == ' ') {
                        p++;
                        break;
                    }
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    size_t n3 = 0U;

    char * ldflags = NULL;

    if (action == 0 || action == ACTION_shared) {
        ldflags = getenv("XCPKG_NATIVE_LDFLAGS");

        //fprintf(stderr, "ldflags=%s\n", ldflags);

        if (ldflags != NULL) {
            p = ldflags;

            while (p[0] != '\0') {
                if (p[0] == ' ') {
                    p++;
                    continue;
                }

                n3++;

                for (;;) {
                    p++;

                    if (p[0] == '\0') {
                        break;
                    }

                    if (p[0] == ' ') {
                        p++;
                        break;
                    }
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    char* args[argc + n1 + n2 + n3 + 5];

    for (i = 1; i < argc; i++) {
        args[i] = argv[i];
    }

    /////////////////////////////////////////////////////////////////

    if (n1 != 0U) {
        char * p = baseArgs;

        while (p[0] != '\0') {
            if (p[0] == ' ') {
                p++;
                continue;
            }

            args[argc++] = p;

            for (;;) {
                p++;

                if (p[0] == '\0') {
                    break;
                }

                if (p[0] == ' ') {
                    p[0] = '\0';
                    p++;
                    break;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (n2 != 0U) {
        char * p = ccflags;

        while (p[0] != '\0') {
            if (p[0] == ' ') {
                p++;
                continue;
            }

            args[argc++] = p;

            for (;;) {
                p++;

                if (p[0] == '\0') {
                    break;
                }

                if (p[0] == ' ') {
                    p[0] = '\0';
                    p++;
                    break;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (n3 != 0U) {
        char * p = ldflags;

        while (p[0] != '\0') {
            if (p[0] == ' ') {
                p++;
                continue;
            }

            args[argc++] = p;

            for (;;) {
                p++;

                if (p[0] == '\0') {
                    break;
                }

                if (p[0] == ' ') {
                    p[0] = '\0';
                    p++;
                    break;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    if (action == ACTION_c || action == ACTION_shared) {
        args[argc++] = (char*)"-fPIC";
    }

    args[argc++] = NULL;
    args[0] = compiler;

    /////////////////////////////////////////////////////////////////

    const char * verbose = getenv("XCPKG_VERBOSE");

    if (verbose != NULL && strcmp(verbose, "1") == 0) {
        for (i = 0; ; i++) {
            if (args[i] == NULL) {
                break;
            } else {
                fprintf(stderr, "%s\n", args[i]);
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    execv (compiler, args);
    perror(compiler);
    return 255;
}
