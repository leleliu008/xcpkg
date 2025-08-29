#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

int main(int argc, char * argv[]) {
    char * const compiler = getenv("XCPKG_COMPILER_CXX");

    if (compiler == NULL) {
        fprintf(stderr, "XCPKG_COMPILER_CXX environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "XCPKG_COMPILER_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }

    /////////////////////////////////////////////////////////////////

    char * const baseArgs = getenv("XCPKG_COMPILER_ARGS_FOR_BUILD");

    if (baseArgs == NULL) {
        fprintf(stderr, "XCPKG_COMPILER_ARGS_FOR_BUILD environment variable is not set.\n");
        return 5;
    }

    if (baseArgs[0] == '\0') {
        fprintf(stderr, "XCPKG_COMPILER_ARGS_FOR_BUILD environment variable value should be a non-empty string.\n");
        return 6;
    }

    /////////////////////////////////////////////////////////////////

    size_t baseArgc = 1U;

    for (size_t i = 0U; ; i++) {
        if (baseArgs[i] == '\0') {
            break;
        }

        if (baseArgs[i] == ' ') {
            baseArgc++;
        }
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

    char* args[argc + baseArgc + 2];

    args[0] = compiler;

    for (int i = 1; i < argc; i++) {
        args[i] = argv[i];
    }

    /////////////////////////////////////////////////////////////////

    char * p = baseArgs;

    for (size_t i = 0U; ; i++) {
        if (baseArgs[i] == '\0') {
            if (p[0] != '\0') {
                args[argc++] = p;
            }
            break;
        }

        if (baseArgs[i] == ' ') {
            baseArgs[i] = '\0';

            if (p[0] != '\0') {
                args[argc++] = p;
            }

            p = &baseArgs[i + 1];
        }
    }

    /////////////////////////////////////////////////////////////////

    if (createSharedLibrary == 1) {
        args[argc++] = (char*)"-fPIC";
    }

    args[argc++] = NULL;

    /////////////////////////////////////////////////////////////////

    const char * verbose = getenv("XCPKG_VERBOSE");

    if (verbose != NULL && strcmp(verbose, "1") == 0) {
        for (int i = 0; ;i++) {
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
