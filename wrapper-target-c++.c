#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char * argv[]) {
    char * const compiler = getenv("PROXIED_CXX");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_CXX environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    const char * const SYSROOT = getenv("SYSROOT");

    if (SYSROOT == NULL) {
        fprintf(stderr, "SYSROOT environment variable is not set.\n");
        return 5;
    }

    if (SYSROOT[0] == '\0') {
        fprintf(stderr, "SYSROOT environment variable value should be a non-empty string.\n");
        return 6;
    }

    /////////////////////////////////////////////////////////////////

    size_t sysrootArgLength = strlen(SYSROOT) + 11U;
    char   sysrootArg[sysrootArgLength];

    int ret = snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

    if (ret < 0) {
        perror(NULL);
        return 8;
    }

    /////////////////////////////////////////////////////////////////

    int createSharedLibrary = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-dynamiclib") == 0 || strcmp(argv[i], "-shared") == 0) {
            createSharedLibrary = 1;
            break;
        }
    }

    /////////////////////////////////////////////////////////////////

    char* argv2[argc + 3];

    if (createSharedLibrary == 0) {
        const char * msle = getenv("PACKAGE_CREATE_MOSTLY_STATICALLY_LINKED_EXECUTABLE");

        if (msle != NULL && strcmp(msle, "1") == 0) {
            for (int i = 1; i < argc; i++) {
                if (argv[i][0] == '/') {
                    int len = 0;
                    int dotIndex = -1;

                    for (int j = 0; ; j++) {
                        if (argv[i][j] == '\0') {
                            len = j;
                            break;
                        }

                        if (argv[i][j] == '.') {
                            dotIndex = j;
                        }
                    }

                    if (dotIndex > 0) {
                        if (len - dotIndex == 6) {
                            if (strcmp(&argv[i][dotIndex], ".dylib") == 0) {
                                argv[i][dotIndex + 1] = 'a' ;
                                argv[i][dotIndex + 2] = '\0';

                                struct stat st;

                                if (stat(argv[i], &st) != 0 || !S_ISREG(st.st_mode)) {
                                    argv[i][dotIndex + 1] = 'd';
                                    argv[i][dotIndex + 2] = 'y';
                                }
                            }
                        }
                    }
                }

                argv2[i] = argv[i];
            }
        } else {
            for (int i = 1; i < argc; i++) {
                argv2[i] = argv[i];
            }
        }
    } else {
        for (int i = 1; i < argc; i++) {
            argv2[i] = argv[i];
        }
    }

    /////////////////////////////////////////////////////////////////

    argv2[0]    = compiler;
    argv2[argc] = sysrootArg;

    if (createSharedLibrary == 1) {
        argv2[argc + 1] = (char*)"-fPIC";
        argv2[argc + 2] = NULL;
    } else {
        argv2[argc + 1] = NULL;
    }

    /////////////////////////////////////////////////////////////////

    const char * verbose = getenv("XCPKG_VERBOSE");

    if (verbose != NULL && strcmp(verbose, "1") == 0) {
        for (int i = 0; ;i++) {
            if (argv2[i] == NULL) {
                break;
            } else {
                fprintf(stderr, "%s\n", argv2[i]);
            }
        }
    }

    /////////////////////////////////////////////////////////////////

    execv (compiler, argv2);
    perror(compiler);
    return 255;
}
