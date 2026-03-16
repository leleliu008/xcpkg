#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "xcpkg.h"

static int xcpkg_rm_rf_internal(const char * path, const bool isRoot, const bool preserveRoot, const bool verbose) {
    struct stat st;

    if (lstat(path, &st) != 0) {
        // path may be a non-existent directory.
        // path may be a non-existent file.
        // path may be a dangling link.

        if (verbose) {
            printf("rm %s\n", path);
        }

        if (unlink(path) == 0) {
            return XCPKG_OK;
        } else {
            perror(path);
            return XCPKG_ERROR;
        }
    }

    if (!S_ISDIR(st.st_mode)) {
        if (verbose) {
            printf("rm %s\n", path);
        }

        if (unlink(path) == 0) {
            return XCPKG_OK;
        } else {
            perror(path);
            return XCPKG_ERROR;
        }
    }

    DIR * dir = opendir(path);

    if (dir == NULL) {
        perror(path);
        return XCPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);

                if (isRoot && preserveRoot) {
                    return XCPKG_OK;
                }

                if (verbose) {
                    printf("rm %s\n", path);
                }

                if (rmdir(path) == 0) {
                    return XCPKG_OK;
                } else {
                    perror(path);
                    return XCPKG_ERROR;
                }
            } else {
                perror(path);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t path2Capacity = strlen(path) + strlen(dir_entry->d_name) + 2U;
        char   path2[path2Capacity];

        int ret = snprintf(path2, path2Capacity, "%s/%s", path, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_rm_rf_internal(path2, false, false, verbose);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }
}

int xcpkg_rm_rf(const char * path, const bool preserveRoot, const bool verbose) {
    if (path == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (path[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    return xcpkg_rm_rf_internal(path, true, preserveRoot, verbose);
}
