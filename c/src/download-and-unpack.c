#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "sha256sum.h"

#include "xcpkg.h"

int xcpkg_download_via_http_then_unpack(const char * url, const char * uri, const char * expectedSHA256SUM, const char * downloadDIR, size_t downloadDIRLength, const char * unpackDIR, size_t unpackDIRLength, const bool verbose) {
    //if (verbose) {
        fprintf(stderr, "url=%s\nuri=%s\nsha=%s\n", url, uri, expectedSHA256SUM);
    //}

    char fileType[21] = {0};

    int ret = xcpkg_extract_filetype_from_url(url, fileType, 20);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t fileNameCapacity = strlen(expectedSHA256SUM) + strlen(fileType) + 1U;
    char   fileName[fileNameCapacity];

    ret = snprintf(fileName, fileNameCapacity, "%s%s", expectedSHA256SUM, fileType);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t filePathCapacity = downloadDIRLength + fileNameCapacity + 1U;
    char   filePath[filePathCapacity];

    ret = snprintf(filePath, filePathCapacity, "%s/%s", downloadDIR, fileName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    bool needFetch = true;

    struct stat st;

    if (stat(filePath, &st) == 0 && S_ISREG(st.st_mode)) {
        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, filePath);

        if (ret != 0) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            needFetch = false;

            if (verbose) {
                fprintf(stderr, "%s already have been fetched.\n", filePath);
            }
        }
    }

    if (needFetch) {
        size_t tmpStrCapacity = strlen(url) + 30U;
        char   tmpStr[tmpStrCapacity];

        ret = snprintf(tmpStr, tmpStrCapacity, "%s|%ld|%d", url, time(NULL), getpid());

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        char tmpFileName[65] = {0};

        ret = sha256sum_of_string(tmpFileName, tmpStr);

        if (ret != 0) {
            return XCPKG_ERROR;
        }

        size_t tmpFilePathCapacity = downloadDIRLength + 65U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

        if (ret != XCPKG_OK) {
            if (uri != NULL) {
                ret = xcpkg_http_fetch_to_file(uri, tmpFilePath, verbose, verbose);
            }
        }

        if (ret != XCPKG_OK) {
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        ret = sha256sum_of_file(actualSHA256SUM, tmpFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
            if (rename(tmpFilePath, filePath) == 0) {
                if (verbose) {
                    printf("%s\n", filePath);
                }
            } else {
                perror(filePath);
                return XCPKG_ERROR;
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
            return XCPKG_ERROR_SHA256_MISMATCH;
        }
    }

    if (strcmp(fileType, ".zip") == 0 ||
        strcmp(fileType, ".tgz") == 0 ||
        strcmp(fileType, ".txz") == 0 ||
        strcmp(fileType, ".tlz") == 0 ||
        strcmp(fileType, ".tbz2") == 0 ||
        strcmp(fileType, ".crate") == 0) {

        ret = tar_extract(unpackDIR, filePath, ARCHIVE_EXTRACT_TIME, verbose, 1);

        if (ret != 0) {
            return abs(ret) + XCPKG_ERROR_ARCHIVE_BASE;
        }
    } else {
        size_t toFilePathCapacity = unpackDIRLength + fileNameCapacity + 1U;
        char   toFilePath[toFilePathCapacity];

        ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", unpackDIR, fileName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_copy_file(filePath, toFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}
