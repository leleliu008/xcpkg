#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char * argv[]) {
    char * PROXIED_PROGRAM = NULL;

    if (argv[0][strlen(argv[0]) - 1] == '+') {
        PROXIED_PROGRAM = getenv("PROXIED_CXX");

        if (PROXIED_PROGRAM == NULL) {
            fprintf(stderr, "PROXIED_CXX environment variable is not set.\n");
            return 2;
        }
    } else {
        PROXIED_PROGRAM = getenv("PROXIED_CC");

        if (PROXIED_PROGRAM == NULL) {
            fprintf(stderr, "PROXIED_CC environment variable is not set.\n");
            return 2;
        }
    }

    char * SYSROOT = getenv("SYSROOT");

    if (SYSROOT == NULL) {
        fprintf(stderr, "SYSROOT environment variable is not set.\n");
        return 2;
    }

    size_t sysrootLength = strlen(SYSROOT) + 11;
    char   sysroot[sysrootLength];
    snprintf(sysroot, sysrootLength, "--sysroot=%s", SYSROOT);

    char * argv2[argc + 1];

    argv2[0] = PROXIED_PROGRAM;
    argv2[1] = sysroot;

    for (int i = 1; i < argc; i++) {
        argv2[i + 1] = argv[i];
    }

    if (PROXIED_PROGRAM[0] == '/') {
        execv (PROXIED_PROGRAM, argv);
        perror(PROXIED_PROGRAM);
        return -1;
    } else {
        execvp(PROXIED_PROGRAM, argv);
        perror(PROXIED_PROGRAM);
        return -1;
    }

    return 0;
}
