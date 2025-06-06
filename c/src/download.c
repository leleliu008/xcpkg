#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "sha256sum.h"

#include "xcpkg.h"

int xcpkg_download(const char * url, const char * uri, const char * expectedSHA256SUM, const char * outputPath, const bool verbose) {
    if (verbose) {
        fprintf(stderr, "Downloading %s %s %s => %s\n", url, uri, expectedSHA256SUM, outputPath);
    }

    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (expectedSHA256SUM != NULL) {
        if (strlen(expectedSHA256SUM) != 64U) {
            return XCPKG_ERROR_ARG_IS_INVALID;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    if (outputPath == NULL || outputPath[0] == '\0' || strcmp(outputPath, "-") == 0 || strcmp(outputPath, "/dev/stdout") == 0) {
        int ret = xcpkg_http_fetch_to_stream(url, stdout, verbose, verbose);

        if (ret != XCPKG_OK) {
            if ((uri != NULL) && (uri[0] != '\0')) {
                ret = xcpkg_http_fetch_to_stream(uri, stdout, verbose, verbose);
            }
        }

        return ret;
    }

    if (strcmp(outputPath, "/dev/stderr") == 0) {
        int ret = xcpkg_http_fetch_to_stream(url, stderr, verbose, verbose);

        if (ret != XCPKG_OK) {
            if ((uri != NULL) && (uri[0] != '\0')) {
                ret = xcpkg_http_fetch_to_stream(uri, stderr, verbose, verbose);
            }
        }

        return ret;
    }

    char outputFilePath[PATH_MAX];

    if (strcmp(outputPath, ".") == 0 || strcmp(outputPath, "./") == 0) {
        int ret = xcpkg_extract_filename_from_url(url, outputFilePath, PATH_MAX);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (strcmp(outputPath, "..") == 0 || strcmp(outputPath, "../") == 0) {
        outputFilePath[0] = '.';
        outputFilePath[1] = '.';
        outputFilePath[2] = '/';

        int ret = xcpkg_extract_filename_from_url(url, outputFilePath + 3, PATH_MAX - 3);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    size_t outputPathLength = strlen(outputPath);

    if (outputPath[outputPathLength - 1U] == '/') {
        strncpy(outputFilePath, outputPath, outputPathLength);

        int ret = xcpkg_extract_filename_from_url(url, outputFilePath + outputPathLength, PATH_MAX - outputPathLength);

        if (ret != XCPKG_OK) {
            return ret;
        }
    } else {
        strncpy(outputFilePath, outputPath, outputPathLength);
        outputFilePath[outputPathLength] = '\0';
    }

    //////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(outputFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            if (expectedSHA256SUM != NULL) {
                char actualSHA256SUM[65] = {0};

                if (sha256sum_of_file(actualSHA256SUM, outputFilePath) != 0) {
                    return XCPKG_ERROR;
                }

                if (strcmp(actualSHA256SUM, expectedSHA256SUM) == 0) {
                    fprintf(stderr, "%s already downloaded into %s\n", url, outputFilePath);
                    return XCPKG_OK;
                }
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", outputFilePath);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    size_t tmpCapacity = strlen(url) + 30U;
    char   tmp[tmpCapacity];

    int ret = snprintf(tmp, tmpCapacity, "%s|%ld|%d", url, time(NULL), getpid());

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char sha256sum[65] = {0};

    ret = sha256sum_of_string(sha256sum, tmp);

    if (ret != 0) {
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    char   outputDIR[PATH_MAX];
    size_t outputDIRLength = 0U;

    int slashIndex = -1;

    for (int i = 0; ;i++) {
        char c = outputFilePath[i];

        if (c == '\0') {
            break;
        }

        if (c == '/') {
            slashIndex = i;
        }
    }

    if (slashIndex > 0) {
        outputDIRLength = slashIndex;

        strncpy(outputDIR, outputFilePath, outputDIRLength);

        outputDIR[outputDIRLength] = '\0';

        int ret = xcpkg_mkdir_p(outputDIR, verbose);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = outputDIRLength + 70U;
    char   tmpFilePath[tmpFilePathCapacity];

    if (outputDIRLength == 0) {
        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s.tmp", sha256sum);
    } else {
        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s.tmp", outputDIR, sha256sum);
    }

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    ret = xcpkg_http_fetch_to_file(url, tmpFilePath, verbose, verbose);

    if (ret != XCPKG_OK) {
        if ((uri != NULL) && (uri[0] != '\0')) {
            ret = xcpkg_http_fetch_to_file(uri, tmpFilePath, verbose, verbose);
        }
    }

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    if (expectedSHA256SUM != NULL) {
        char actualSHA256SUM[65] = {0};

        if (sha256sum_of_file(actualSHA256SUM, tmpFilePath) != 0) {
            return XCPKG_ERROR;
        }

        if (strcmp(actualSHA256SUM, expectedSHA256SUM) != 0) {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", expectedSHA256SUM, actualSHA256SUM);
            return XCPKG_ERROR_SHA256_MISMATCH;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    if (rename(tmpFilePath, outputFilePath) == 0) {
        if (verbose) {
            fprintf(stderr, "%s => %s\n", url, outputFilePath);
        }
        return XCPKG_OK;
    } else {
        perror(outputFilePath);
        return XCPKG_ERROR;
    }
}
