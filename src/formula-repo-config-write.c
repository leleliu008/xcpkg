#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>

#include "xcpkg.h"

int xcpkg_formula_repo_config_write(const char * formulaRepoDIRPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * createdAt, const char * updatedAt) {
    if (formulaRepoDIRPath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (branchName == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (createdAt == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (updatedAt == NULL) {
        updatedAt = "";
    }

    size_t strCapacity = strlen(formulaRepoUrl) + strlen(branchName) + strlen(createdAt) + strlen(updatedAt) + 78U;
    char   str[strCapacity];

    int ret = snprintf(str, strCapacity, "url: %s\nbranch: %s\npinned: %1d\nenabled: %1d\ncreated: %s\nupdated: %s\n", formulaRepoUrl, branchName, pinned, enabled, createdAt, updatedAt);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int strLength = ret;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoConfigFilePathCapacity = strlen(formulaRepoDIRPath) + strlen(XCPKG_FORMULA_REPO_CONFIG_FILENAME) + 2U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/%s", formulaRepoDIRPath, XCPKG_FORMULA_REPO_CONFIG_FILENAME);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int fd = open(formulaRepoConfigFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(formulaRepoConfigFilePath);
        return XCPKG_ERROR;
    }

    ssize_t writeSize = write(fd, str, strLength);

    if (writeSize == -1) {
        perror(formulaRepoConfigFilePath);
        close(fd);
        return XCPKG_ERROR;
    }

    close(fd);

    if (writeSize == strLength) {
        return XCPKG_OK;
    } else {
        fprintf(stderr, "not fully written to %s\n", formulaRepoConfigFilePath);
        return XCPKG_ERROR;
    }
}
