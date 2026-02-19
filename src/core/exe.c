#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "exe.h"

int exe_search(const char * commandName, char *** listP, const bool findAll) {
    if (commandName == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (commandName[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (listP == NULL) {
        errno = EINVAL;
        return -1;
    }

    //////////////////////////////////

    const char * p = getenv("PATH");

    if (p == NULL) {
        return -2;
    }

    if (p[0] == '\0') {
        return -3;
    }

    //////////////////////////////////

    char ** stringArrayList = NULL;
    size_t  stringArrayListSize    = 0U;
    size_t  stringArrayListCapacity = 0U;

    struct stat st;

    char pathBuf[PATH_MAX];

    char * q;

    size_t i;
    size_t n;

loop:
    if (p[0] == '\0') {
        return 0;
    }

    if (p[0] == ' ' || p[0] == ':') {
        p++;
        goto loop;
    }

    //////////////////////////////////

    for (i = 0U; ; i++) {
        pathBuf[i] = p[i];

        if (p[i] == '\0') {
            break;
        }

        if (p[i] == ':') {
            pathBuf[i] = '\0';
            break;
        }
    }

    //////////////////////////////////

    if (stat(pathBuf, &st) != 0) goto next;

    if (!S_ISDIR(st.st_mode)) goto next;

    //////////////////////////////////

    q = pathBuf + i;

    q[0] = '/';

    q++;

    for (size_t j = 0U; ; j++) {
        q[j] = commandName[j];

        if (q[j] == '\0') {
            n = i + j + 1U;
            break;
        }
    }

    //////////////////////////////////

    if (access(pathBuf, X_OK) != 0) goto next;

    //////////////////////////////////

    char * s = malloc(n + 1);

    if (s == NULL) {
        if (stringArrayList != NULL) {
            for (size_t i = 0; i < stringArrayListSize; i++) {
                free(stringArrayList[i]);
                stringArrayList[i] = NULL;
            }
            free(stringArrayList);
        }
        errno = ENOMEM;
        return -1;
    }

    //////////////////////////////////

    if (stringArrayListCapacity == stringArrayListSize) {
        stringArrayListCapacity += 2U;

        char** paths = (char**)realloc(stringArrayList, stringArrayListCapacity * sizeof(char*));

        if (paths == NULL) {
            if (stringArrayList != NULL) {
                for (size_t i = 0; i < stringArrayListSize; i++) {
                    free(stringArrayList[i]);
                    stringArrayList[i] = NULL;
                }
                free(stringArrayList);
            }
            errno = ENOMEM;
            return -1;
        } else {
            stringArrayList = paths;
        }
    }

    stringArrayList[stringArrayListSize] = s;
    stringArrayListSize += 1U;

    //////////////////////////////////

    for (size_t j = 0U; j < n; j++) {
        s[j] = pathBuf[j];
    }

    s[n] = '\0';

    //////////////////////////////////

    if (!findAll) {
        goto finally;
    }

next:
    p += i;

    if (p[0] == ':') {
        p++;
        goto loop;
    }

finally:
    (*listP) = stringArrayList;
    return stringArrayListSize;
}

int exe_lookup(const char * commandName, char ** pathP) {
    if (commandName == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (commandName[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (pathP == NULL) {
        errno = EINVAL;
        return -1;
    }

    //////////////////////////////////

    const char * p = getenv("PATH");

    if (p == NULL) {
        return -2;
    }

    if (p[0] == '\0') {
        return -3;
    }

    //////////////////////////////////

    struct stat st;

    char pathBuf[PATH_MAX];

    char * q;

    size_t i;
    size_t n;

loop:
    if (p[0] == '\0') {
        return 0;
    }

    if (p[0] == ' ' || p[0] == ':') {
        p++;
        goto loop;
    }

    //////////////////////////////////

    for (i = 0U; ; i++) {
        pathBuf[i] = p[i];

        if (p[i] == '\0') {
            break;
        }

        if (p[i] == ':') {
            pathBuf[i] = '\0';
            break;
        }
    }

    //////////////////////////////////

    if ((stat(pathBuf, &st) == 0) && S_ISDIR(st.st_mode)) {
        q = &pathBuf[i];

        q[0] = '/';

        q++;

        for (size_t j = 0U; ; j++) {
            q[j] = commandName[j];

            if (q[j] == '\0') {
                n = i + j + 1U;
                break;
            }
        }

        if (access(pathBuf, X_OK) == 0) {
            char * s = malloc(n + 1);

            if (s == NULL) {
                errno = ENOMEM;
                return -1;
            }

            for (size_t j = 0U; j < n; j++) {
                s[j] = pathBuf[j];
            }

            s[n] = '\0';

            (*pathP) = s;

            return n;
        }
    }

    //////////////////////////////////

    p += i;

    if (p[0] == ':') {
        p++;
        goto loop;
    }

    return 0;
}

int exe_where(const char * commandName, char buf[]) {
    if (commandName == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (commandName[0] == '\0') {
        errno = EINVAL;
        return -1;
    }

    if (buf == NULL) {
        errno = EINVAL;
        return -1;
    }

    //////////////////////////////////

    const char * p = getenv("PATH");

    if (p == NULL) {
        return -2;
    }

    if (p[0] == '\0') {
        return -3;
    }

    //////////////////////////////////

    struct stat st;

    char pathBuf[PATH_MAX];

    char * q;

    size_t i;
    size_t n;

loop:
    if (p[0] == '\0') {
        return 0;
    }

    if (p[0] == ' ' || p[0] == ':') {
        p++;
        goto loop;
    }

    //////////////////////////////////

    for (i = 0U; ; i++) {
        pathBuf[i] = p[i];

        if (p[i] == '\0') {
            break;
        }

        if (p[i] == ':') {
            pathBuf[i] = '\0';
            break;
        }
    }

    //////////////////////////////////

    if ((stat(pathBuf, &st) == 0) && S_ISDIR(st.st_mode)) {
        q = &pathBuf[i];

        q[0] = '/';

        q++;

        for (size_t j = 0U; ; j++) {
            q[j] = commandName[j];

            if (q[j] == '\0') {
                n = i + j + 1U;
                break;
            }
        }

        if (access(pathBuf, X_OK) == 0) {
            for (size_t j = 0U; j < n; j++) {
                buf[j] = pathBuf[j];
            }

            buf[n] = '\0';

            return n;
        }
    }

    //////////////////////////////////

    p += i;

    if (p[0] == ':') {
        p++;
        goto loop;
    }

    return 0;
}
