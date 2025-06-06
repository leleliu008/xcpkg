#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>

#include "xcpkg.h"

static int xcpkg_rm_rf_internal(const char * dirPath, const bool isRoot, const bool preserveRoot, const bool verbose) {
    if (dirPath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (dirPath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    size_t dirPathLength = strlen(dirPath);

    struct stat st;

    if (lstat(dirPath, &st) != 0) {
        // why does this happened?
        // Suppose you have following directory structure:
        // bin
        // ├── gsed
        // └── sed -> gsed
        // if bin/gsed was removed, then bin/sed points to a non-existent file. In this case, bin/sed is known as a dangling link.

        if (verbose) {
            printf("rm %s\n", dirPath);
        }

        if (unlink(dirPath) == 0) {
            return XCPKG_OK;
        } else {
            perror(dirPath);
            return XCPKG_ERROR;
        }
    }

    if (!S_ISDIR(st.st_mode)) {
        if (isRoot && preserveRoot) {
            return XCPKG_OK;
        } else {
            if (verbose) {
                printf("rm %s\n", dirPath);
            }

            if (unlink(dirPath) == 0) {
                return XCPKG_OK;
            } else {
                perror(dirPath);
                return XCPKG_ERROR;
            }
        }
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
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
                } else {
                    if (verbose) {
                        printf("rm %s\n", dirPath);
                    }

                    if (rmdir(dirPath) == 0) {
                        return XCPKG_OK;
                    } else {
                        perror(dirPath);
                        return XCPKG_ERROR;
                    }
                }
            } else {
                perror(dirPath);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathCapacity = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char   filePath[filePathCapacity];

        int ret = snprintf(filePath, filePathCapacity, "%s/%s", dirPath, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (lstat(filePath, &st) == 0) {
            if (S_ISDIR(st.st_mode)) {
                int ret = xcpkg_rm_rf_internal(filePath, false, false, verbose);

                if (ret != XCPKG_OK) {
                    return ret;
                }
            } else {
                if (verbose) {
                    printf("rm %s\n", filePath);
                }

                if (unlink(filePath) != 0) {
                    perror(filePath);
                    closedir(dir);
                    return XCPKG_ERROR;
                }
            }
        } else {
            // why does this happened?
            // Suppose you have following directory structure:
            // bin
            // ├── gsed
            // └── sed -> gsed
            // if bin/gsed was removed, then bin/sed points to a non-existent file. In this case, bin/sed is known as a dangling link.

            if (verbose) {
                printf("rm %s\n", filePath);
            }

            if (unlink(filePath) != 0) {
                perror(filePath);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }
    }
}

int xcpkg_rm_rf(const char * dirPath, const bool preserveRoot, const bool verbose) {
    return xcpkg_rm_rf_internal(dirPath, true, preserveRoot, verbose);
}
