#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "xcpkg.h"

static int read_from_fd(int inputFD, char buf[PATH_MAX]) {
    ssize_t readSize = read(inputFD, buf, PATH_MAX);

    if (readSize < 0) {
        return -1;
    }

    if (readSize == 0) {
        return 0;
    }

    if (buf[readSize - 1] == '\n') {
        readSize--;
    }

    if (readSize > 0) {
        buf[readSize] = '\0';
    }

    return 0;
}

// https://keith.github.io/xcode-man-pages/xcrun.1.html
int xcpkg_sdk_path(const char * sdk, char buf[PATH_MAX]) {
    int pipeFDs[2];

    if (pipe(pipeFDs) != 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (pid == 0) {
        close(pipeFDs[0]);

        if (dup2(pipeFDs[1], STDOUT_FILENO) < 0) {
            perror(NULL);
            exit(254);
        }

        execl("/usr/bin/xcrun", "/usr/bin/xcrun", "--sdk", sdk, "--show-sdk-path", NULL);

        perror("/usr/bin/xcrun");

        exit(255);
    } else {
        close(pipeFDs[1]);

        int ret = read_from_fd(pipeFDs[0], buf);

        int status;

        if (waitpid(pid, &status, 0) < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (status == 0) {
            if (ret == 0) {
                return XCPKG_OK;
            } else {
                perror(NULL);
                return XCPKG_ERROR;
            }
        }

        if (WIFEXITED(status)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk %s --show-sdk-path' exit with status code: %d\n", sdk, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk %s --show-sdk-path' killed by signal: %d\n", sdk, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk %s --show-sdk-path' stopped by signal: %d\n", sdk, WSTOPSIG(status));
        }

        return XCPKG_ERROR;
    }
}

// https://keith.github.io/xcode-man-pages/xcrun.1.html
static int xcrun_find(const char * what, char buf[PATH_MAX]) {
    int pipeFDs[2];

    if (pipe(pipeFDs) != 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (pid == 0) {
        close(pipeFDs[0]);

        if (dup2(pipeFDs[1], STDOUT_FILENO) < 0) {
            perror(NULL);
            exit(254);
        }

        execl("/usr/bin/xcrun", "/usr/bin/xcrun", "--sdk", "macosx", "--find", what, NULL);

        perror("/usr/bin/xcrun");

        exit(255);
    } else {
        close(pipeFDs[1]);

        int ret = read_from_fd(pipeFDs[0], buf);

        int status;

        if (waitpid(pid, &status, 0) < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (status == 0) {
            if (ret == 0) {
                return XCPKG_OK;
            } else {
                perror(NULL);
                return XCPKG_ERROR;
            }
        }

        if (WIFEXITED(status)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --find %s' exit with status code: %d\n", what, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --find %s' killed by signal: %d\n", what, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            fprintf(stderr, "running command '/usr/bin/xcrun --sdk macosx --find %s' stopped by signal: %d\n", what, WSTOPSIG(status));
        }

        return XCPKG_ERROR;
    }
}

int xcpkg_toolchain_find(XCPKGToolChain * toolchain) {
    const char * a[11] = { "clang", "clang++", "as", "ar", "ld", "nm", "size", "strip", "ranlib", "strings", "objdump" };

    char buf[PATH_MAX];

    char * p = NULL;

    int ret;

    for (int i = 0; i < 11; i++) {
        buf[0] = '\0';

        ret = xcrun_find(a[i], buf);

        if (ret == XCPKG_OK) {
            if (buf[0] == '\0') {
                fprintf(stderr, "command not found: %s\n", a[i]);
                xcpkg_toolchain_free(toolchain);
                return XCPKG_ERROR;
            } else {
                p = strdup(buf);

                if (p == NULL) {
                    xcpkg_toolchain_free(toolchain);
                    return XCPKG_ERROR_MEMORY_ALLOCATE;
                }

                switch (i) {
                    case 0: toolchain->cc = p; break;
                    case 1: toolchain->cxx = p; break;
                    case 2: toolchain->as = p; break;
                    case 3: toolchain->ar = p; break;
                    case 4: toolchain->ld = p; break;
                    case 5: toolchain->nm = p; break;
                    case 6: toolchain->size = p; break;
                    case 7: toolchain->strip = p; break;
                    case 8: toolchain->ranlib = p; break;
                    case 9: toolchain->strings = p; break;
                    case 10: toolchain->objdump = p; break;
                }
            }
        } else {
            xcpkg_toolchain_free(toolchain);
            return ret;
        }
    }

    return XCPKG_OK;
}

void xcpkg_toolchain_dump(XCPKGToolChain * toolchain) {
    if (toolchain == NULL) {
        return;
    }

    printf("cc:        %s\n", toolchain->cc);
    printf("cxx:       %s\n", toolchain->cxx);
    printf("swift:     %s\n", toolchain->swift);
    printf("as:        %s\n", toolchain->as);
    printf("ar:        %s\n", toolchain->ar);
    printf("ranlib:    %s\n", toolchain->ranlib);
    printf("ld:        %s\n", toolchain->ld);
    printf("nm:        %s\n", toolchain->nm);
    printf("size:      %s\n", toolchain->size);
    printf("strip:     %s\n", toolchain->strip);
    printf("strings:   %s\n", toolchain->strings);
    printf("objdump:   %s\n", toolchain->objdump);
}

void xcpkg_toolchain_free(XCPKGToolChain * toolchain) {
    if (toolchain == NULL) {
        return;
    }

    if (toolchain->cc != NULL) {
        free(toolchain->cc);
        toolchain->cc = NULL;
    }

    if (toolchain->cxx != NULL) {
        free(toolchain->cxx);
        toolchain->cxx = NULL;
    }

    if (toolchain->swift != NULL) {
        free(toolchain->swift);
        toolchain->swift = NULL;
    }

    if (toolchain->as != NULL) {
        free(toolchain->as);
        toolchain->as = NULL;
    }

    if (toolchain->ar != NULL) {
        free(toolchain->ar);
        toolchain->ar = NULL;
    }

    if (toolchain->ranlib != NULL) {
        free(toolchain->ranlib);
        toolchain->ranlib = NULL;
    }

    if (toolchain->ld != NULL) {
        free(toolchain->ld);
        toolchain->ld = NULL;
    }

    if (toolchain->nm != NULL) {
        free(toolchain->nm);
        toolchain->nm = NULL;
    }

    if (toolchain->size != NULL) {
        free(toolchain->size);
        toolchain->size = NULL;
    }

    if (toolchain->strip != NULL) {
        free(toolchain->strip);
        toolchain->strip = NULL;
    }

    if (toolchain->strings != NULL) {
        free(toolchain->strings);
        toolchain->strings = NULL;
    }

    if (toolchain->objdump != NULL) {
        free(toolchain->objdump);
        toolchain->objdump = NULL;
    }
}
