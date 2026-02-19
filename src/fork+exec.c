#include <string.h>
#include <stdarg.h>

#include <unistd.h>

#include "core/log.h"

#include "xcpkg.h"

int xcpkg_fork_exec(char * cmd) {
    fprintf(stderr, "%s==>%s %s%s%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, cmd, COLOR_OFF);

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (pid == 0) {
        size_t argc = 0U;
        char*  argv[30] = {0};

        char * arg = strtok(cmd, " ");

        while (arg != NULL) {
            argv[argc] = arg;
            argc++;
            arg = strtok(NULL, " ");
        }

        ////////////////////////////////////////

        bool isPath = false;

        const char * p = argv[0];

        for (;;) {
            if (p[0] == '\0') {
                break;
            }

            if (p[0] == '/') {
                isPath = true;
                break;
            }

            p++;
        }

        ////////////////////////////////////////

        if (isPath) {
            execv (argv[0], argv);
        } else {
            execvp(argv[0], argv);
        }

        perror(argv[0]);
        exit(255);
    } else {
        int status;

        if (waitpid(pid, &status, 0) < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (status == 0) {
            return XCPKG_OK;
        }

        if (WIFEXITED(status)) {
            fprintf(stderr, "running command '%s' exit with status code: %d\n", cmd, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "running command '%s' killed by signal: %d\n", cmd, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            fprintf(stderr, "running command '%s' stopped by signal: %d\n", cmd, WSTOPSIG(status));
        }

        return XCPKG_ERROR;
    }
}

int xcpkg_fork_exec2(const size_t n, ...) {
    char* argv[n + 1];

    va_list args;
    va_start(args, n);

    for (size_t i = 0; i < n; i++) {
        argv[i] = va_arg(args, char*);
    }

    va_end(args);

    argv[n] = NULL;

    //////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (pid == 0) {
        bool isPath = false;

        const char * p = argv[0];

        for (;;) {
            if (p[0] == '\0') {
                break;
            }

            if (p[0] == '/') {
                isPath = true;
                break;
            }

            p++;
        }

        ////////////////////////////////////////

        if (isPath) {
            execv (argv[0], argv);
        } else {
            execvp(argv[0], argv);
        }

        perror(argv[0]);
        exit(255);
    } else {
        int status;

        if (waitpid(pid, &status, 0) < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (status == 0) {
            return XCPKG_OK;
        }

        char s[1024];
        char * p = s;

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

        if (WIFEXITED(status)) {
            fprintf(stderr, "running command '%s' exit with status code: %d\n", s, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "running command '%s' killed by signal: %d\n", s, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            fprintf(stderr, "running command '%s' stopped by signal: %d\n", s, WSTOPSIG(status));
        }

        return XCPKG_ERROR;
    }
}
