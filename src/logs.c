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

    char fzfCommandPath[PATH_MAX];

    ret = xcpkg_get_command_path_of_uppm_package("fzf", "fzf", fzfCommandPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char batCommandPath[PATH_MAX];

    ret = xcpkg_get_command_path_of_uppm_package("bat", "bat", batCommandPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir(metaInfoDIR) == -1) {
        if (errno == ENOENT) {
            return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
        } else {
            perror(metaInfoDIR);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t arg1Capacity = strlen(batCommandPath) + 45;
    char   arg1[arg1Capacity];

    ret = snprintf(arg1, arg1Capacity, "--preview=%s --color=always --theme=Dracula {}", batCommandPath);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    execl (fzfCommandPath, fzfCommandPath, arg1, "--preview-window=right:75%", NULL);
    perror(fzfCommandPath);

    return XCPKG_ERROR;
}
