#include <stddef.h>
#include <stdarg.h>

#include <spawn.h>
#include <crt_externs.h>
#include <stdio.h>

#include "core/log.h"

#include "xcpkg.h"

static inline __attribute__((always_inline)) int xcpkg_posix_spawn_internal(const char * cmd, char* argv[]) {
    fprintf(stderr, "%s==>%s %s%s%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, cmd, COLOR_OFF);

    //////////////////////////////////

    pid_t pid;

    if (posix_spawnp(&pid, argv[0], NULL, NULL, argv, *_NSGetEnviron()) != 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int status;

    if (waitpid(pid, &status, 0) == -1) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (status == 0) {
        return XCPKG_OK;
    }

    //////////////////////////////////

    if (WIFEXITED(status)) {
        fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(status));
    } else if (WIFSTOPPED(status)) {
        fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(status));
    }

    return XCPKG_ERROR;
}

int xcpkg_posix_spawn(const char * cmd) {
    size_t i;

    for (i = 0U; cmd[i] != '\0'; i++);

    char s[i + 1];

    for (i = 0U; ; i++) {
        s[i] = cmd[i];

        if (s[i] == '\0') break;
    }

    size_t argc = 0U;
    char*  argv[30];

    char * p = s;

    while (p[0] != '\0') {
        if (p[0] == ' ') {
            p++;
            continue;
        }

        argv[argc++] = p;

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

    argv[argc] = NULL;

    //for (i = 0U; i < argc; i++) {
    //    fprintf(stderr, "argv[%lu]=%s\n", i, argv[i]);
    //}

    return xcpkg_posix_spawn_internal(cmd, argv);
}

int xcpkg_posix_spawn2(const size_t n, ...) {
    char* argv[n + 1];

    va_list args;
    va_start(args, n);

    for (size_t i = 0; i < n; i++) {
        argv[i] = va_arg(args, char*);
    }

    va_end(args);

    argv[n] = NULL;

    //////////////////////////////////

    char cmd[1024];
    char * p = cmd;

    for (size_t i = 0U; i < n; i++) {
        if (i != 0U) {
            p[0] = ' ';
            p++;
        }

        for (size_t j = 0U; ; j++) {
            p[j] = argv[i][j];

            if (p[j] == '\0') {
                p += j;
                break;
            }
        }
    }

    return xcpkg_posix_spawn_internal(cmd, argv);
}
