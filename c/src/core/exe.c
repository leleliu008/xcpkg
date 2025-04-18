#include <errno.h>
#include <stdio.h>
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

    const char * const PATH = getenv("PATH");

    if (PATH == NULL) {
        return -2;
    }

    if (PATH[0] == '\0') {
        return -3;
    }

    size_t  PATH2Capacity = strlen(PATH) + 1U;
    char    PATH2[PATH2Capacity];
    strncpy(PATH2, PATH, PATH2Capacity);

    struct stat st;

    size_t commandNameLength = strlen(commandName);

    char ** stringArrayList = NULL;
    size_t  stringArrayListSize    = 0U;
    size_t  stringArrayListCapacity = 0U;

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t fullPathCapacity = strlen(PATHItem) + commandNameLength + 2U;
            char   fullPath[fullPathCapacity];

            int ret = snprintf(fullPath, fullPathCapacity, "%s/%s", PATHItem, commandName);

            if (ret < 0) {
                return -1;
            }

            if (access(fullPath, X_OK) == 0) {
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

                char * fullPathDup = strdup(fullPath);

                if (fullPathDup == NULL) {
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

                stringArrayList[stringArrayListSize] = fullPathDup;
                stringArrayListSize += 1U;

                if (!findAll) {
                    break;
                }
            }
        }

        PATHItem = strtok(NULL, ":");
    }

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

    const char * const PATH = getenv("PATH");

    if (PATH == NULL) {
        return -2;
    }

    if (PATH[0] == '\0') {
        return -3;
    }

    size_t  PATH2Capacity = strlen(PATH) + 1U;
    char    PATH2[PATH2Capacity];
    strncpy(PATH2, PATH, PATH2Capacity);

    struct stat st;

    size_t commandNameLength = strlen(commandName);

    char * PATHItem = strtok(PATH2, ":");

    while (PATHItem != NULL) {
        if ((stat(PATHItem, &st) == 0) && S_ISDIR(st.st_mode)) {
            size_t fullPathCapacity = strlen(PATHItem) + commandNameLength + 2U;
            char   fullPath[fullPathCapacity];

            int ret = snprintf(fullPath, fullPathCapacity, "%s/%s", PATHItem, commandName);

            if (ret < 0) {
                return -1;
            }

            if (access(fullPath, X_OK) == 0) {
                char * p = strdup(fullPath);

                if (p == NULL) {
                    errno = ENOMEM;
                    return -1;
                }

                (*pathP) = p;

                return ret;
            }
        }

        PATHItem = strtok(NULL, ":");
    }

    (*pathP) = NULL;
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

    const char * p = getenv("PATH");

    if (p == NULL) {
        return -2;
    }

    if (p[0] == '\0') {
        return -3;
    }

    struct stat st;

    char tmpBuf[PATH_MAX];
    char outBuf[PATH_MAX];

    while (p != NULL) {
        for (int i = 0;; i++) {
            if (p[i] == '\0') {
                p = NULL;
                tmpBuf[i] = '\0';

                if (i != 0) {
                    if ((stat(tmpBuf, &st) == 0) && S_ISDIR(st.st_mode)) {
                        int n = snprintf(outBuf, PATH_MAX, "%s/%s", tmpBuf, commandName);

                        if (n < 0) {
                            return -1;
                        }

                        if (access(outBuf, X_OK) == 0) {
                            strncpy(buf, outBuf, n);

                            buf[n] = '\0';

                            return n;
                        }
                    }
                }

                break;
            }

            if (p[i] == ':') {
                p += i + 1;
                tmpBuf[i] = '\0';

                if (i != 0) {
                    if ((stat(tmpBuf, &st) == 0) && S_ISDIR(st.st_mode)) {
                        int n = snprintf(outBuf, PATH_MAX, "%s/%s", tmpBuf, commandName);

                        if (n < 0) {
                            return -1;
                        }

                        if (access(outBuf, X_OK) == 0) {
                            strncpy(buf, outBuf, n);

                            buf[n] = '\0';

                            return n;
                        }
                    }
                }

                break;
            }

            tmpBuf[i] = p[i];
        }
    }

    return 0;
}
