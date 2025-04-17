#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_logs(const char * packageName, const char * targetPlatformSpec) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (targetPlatformSpec == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t metaInfoDIRCapacity = xcpkgHomeDIRLength + strlen(targetPlatformSpec) + strlen(packageName) + sizeof(XCPKG_METADATA_DIR_PATH_RELATIVE_TO_INSTALLED_ROOT) + 15U;
    char   metaInfoDIR[metaInfoDIRCapacity];

    ret = snprintf(metaInfoDIR, metaInfoDIRCapacity, "%s/installed/%s/%s%s", xcpkgHomeDIR, targetPlatformSpec, packageName, XCPKG_METADATA_DIR_PATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (chdir(metaInfoDIR) == -1) {
        if (errno == ENOENT) {
            return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
        } else {
            perror(metaInfoDIR);
            return XCPKG_ERROR;
        }
    }

    struct stat st;

    if (stat("RECEIPT.yml", &st) != 0 || !S_ISREG(st.st_mode)) {
        return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * const str = "/uppm/installed/fzf/bin/fzf";

    size_t fzfCommandPathCapacity = xcpkgHomeDIRLength + strlen(str) + sizeof(char);
    char   fzfCommandPath[fzfCommandPathCapacity];

    ret = snprintf(fzfCommandPath, fzfCommandPathCapacity, "%s%s", xcpkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    return xcpkg_fork_exec2(3, fzfCommandPath, "--preview=bat --color=always --theme=Dracula {}", "--preview-window=right:75%");
}

static int xx(const char * metaInfoDIR, const size_t metaInfoDIRCapacity) {
    DIR * dir = opendir(metaInfoDIR);

    if (dir == NULL) {
        perror(metaInfoDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(metaInfoDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filepathCapacity = metaInfoDIRCapacity + strlen(dir_entry->d_name) + 2U;
        char   filepath[filepathCapacity];

        int ret = snprintf(filepath, filepathCapacity, "%s/%s", metaInfoDIR, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return XCPKG_ERROR;
        }

        struct stat st;

        if (stat(filepath, &st) == 0 && S_ISDIR(st.st_mode)) {
            continue;
        }

        ret = xcpkg_fork_exec2(2, "bat", filepath);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }
}
