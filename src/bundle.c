#include <string.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "core/log.h"
#include "core/tar.h"

#include "xcpkg.h"

int xcpkg_bundle(const char * packageName, const char * targetPlatformSpec, ArchiveType outputType, const char * outputPath, const bool verbose) {
    XCPKGReceipt * receipt = NULL;

    int ret = xcpkg_receipt_parse(packageName, targetPlatformSpec, &receipt);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    size_t len = 0U;

    for (size_t i = 0U; ; i++) {
        if (packageName[i] == '\0') {
            len = (len == 0U) ? i : len;
            break;
        }

        if (packageName[i] == '@') {
            len = i;
        }
    }

    size_t packingDIRNameCapacity = len + strlen(receipt->version) + strlen(receipt->builtFor) + 3U;
    char   packingDIRName[packingDIRNameCapacity];

    for (size_t i = 0U; i < len; i++) {
        packingDIRName[i] = packageName[i];
    }

    ret = snprintf(&packingDIRName[len], packingDIRNameCapacity - len, "-%s-%s", receipt->version, receipt->builtFor);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    xcpkg_receipt_free(receipt);

    /////////////////////////////////////////////////////////////////////////////////

    const char * outputFileExt;

    switch (outputType) {
        case ArchiveType_tar_gz:  outputFileExt = ".tar.gz";  break;
        case ArchiveType_tar_xz:  outputFileExt = ".tar.xz";  break;
        case ArchiveType_tar_lz:  outputFileExt = ".tar.lz";  break;
        case ArchiveType_tar_bz2: outputFileExt = ".tar.bz2"; break;
        case ArchiveType_zip:     outputFileExt = ".zip";     break;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char buf[PATH_MAX];

    char * cwd = getcwd(buf, PATH_MAX);

    if (cwd == NULL) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    char outputFilePath[PATH_MAX];

    if (outputPath == NULL) {
        ret = snprintf(outputFilePath, PATH_MAX, "%s/%s%s", cwd, packingDIRName, outputFileExt);
    } else {
        size_t outputPathLength = strlen(outputPath);

        if (outputPath[outputPathLength - 1U] == '/') {
            if (outputPath[0] == '/') {
                ret = snprintf(outputFilePath, PATH_MAX, "%s%s%s", outputPath, packingDIRName, outputFileExt);
            } else {
                ret = snprintf(outputFilePath, PATH_MAX, "%s/%s%s%s", cwd, outputPath, packingDIRName, outputFileExt);
            }
        } else {
            if (outputPath[0] == '/') {
                strncpy(outputFilePath, outputPath, outputPathLength);
                outputFilePath[outputPathLength] = '\0';
            } else {
                ret = snprintf(outputFilePath, PATH_MAX, "%s/%s", cwd, outputPath);
            }
        }
    }

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ///////////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////

    if (chdir(sessionDIR) != 0) {
        perror(sessionDIR);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledDIRCapacity = xcpkgHomeDIRLength + strlen(targetPlatformSpec) + strlen(packageName) + 15U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/installed/%s/%s", xcpkgHomeDIR, targetPlatformSpec, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (symlink(packageInstalledDIR, packingDIRName) != 0) {
        perror(packageInstalledDIR);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + packingDIRNameCapacity + 10U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s%s", sessionDIR, packingDIRName, outputFileExt);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = tar_create(packingDIRName, tmpFilePath, outputType, verbose);

    if (ret != 0) {
        return ret;
    }

    ret = xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(outputFilePath, &st) != 0) {
        perror(outputFilePath);
        return XCPKG_ERROR;
    }

    off_t nBytes = st.st_size;

    if (nBytes < 1024) {
        if (sizeof(off_t) == sizeof(long long)) {
            printf("%s %lld Byte\n", outputFilePath, (long long)nBytes);
        } else {
            printf("%s %ld Byte\n", outputFilePath, (long int)nBytes);
        }
    } else if (nBytes < 1024 * 1024) {
        printf("%s %.2f KB\n", outputFilePath, nBytes / 1024.0);
    } else if (nBytes < 1024 * 1024 * 1024) {
        printf("%s %.2f MB\n", outputFilePath, nBytes / 1024.0 / 1024.0);
    } else {
        LOG_ERROR2("file is too large: ", tmpFilePath);
        return XCPKG_ERROR;
    }

    return xcpkg_rm_rf(sessionDIR, false, verbose);
}
