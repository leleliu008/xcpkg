#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
#ifdef WRAPPER_CXX
    char * const compiler = getenv("PROXIED_CXX");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_CXX environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_CXX environment variable value should be a non-empty string.\n");
        return 2;
    }
#else
    char * const compiler = getenv("PROXIED_CC");

    if (compiler == NULL) {
        fprintf(stderr, "PROXIED_CC environment variable is not set.\n");
        return 1;
    }

    if (compiler[0] == '\0') {
        fprintf(stderr, "PROXIED_CC environment variable value should be a non-empty string.\n");
        return 2;
    }
#endif

    const char * const SYSROOT = getenv("SYSROOT");

    /////////////////////////////////////////////////////////////////

    char* argv2[argc + 3];

    argv2[0] = compiler;

    for (int i = 1; i < argc; i++) {
        argv2[i] = argv[i];
    }

    /////////////////////////////////////////////////////////////////

    size_t   sysrootArgLength = strlen(SYSROOT) + 11U;
    char     sysrootArg[sysrootArgLength];
    snprintf(sysrootArg, sysrootArgLength, "--sysroot=%s", SYSROOT);

    argv2[argc]     = sysrootArg;
    argv2[argc + 1] = NULL;

    execv (compiler, argv2);
    perror(compiler);
    return 255;
}
