#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include <curl/curl.h>
#include <curl/curlver.h>

#include "core/url-transform.h"

#include "sha256sum.h"

#include "xcpkg.h"

int xcpkg_http_fetch_to_stream(const char * url, FILE * outputFile, const bool verbose, const bool showProgress) {
    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    /////////////////////////////////////////

    if (outputFile == NULL) {
        int slashIndex = 0U;
        int nulIndex   = 0U;

        for (int i = 0; ; i++) {
            if (url[i] == '\0') {
                nulIndex = i;
                break;
            }

            if (url[i] == '/') {
                slashIndex = i;
            }
        }

        if (slashIndex == 0) {
            return XCPKG_ERROR_ARG_IS_INVALID;
        }

        size_t capacity = nulIndex - slashIndex;

        if (capacity < 2) {
            return XCPKG_ERROR_ARG_IS_INVALID;
        }

        char   filename[capacity];

        strncpy(filename, url + slashIndex + 1, capacity);

        outputFile = fopen(filename, "wb");

        if (outputFile == NULL) {
            perror(filename);
            return XCPKG_ERROR;
        }
    }

    ///////////////////////////////////////////////////////////

    char userAgent[50];

    const char * s = "User-Agent: curl-";

    size_t i;

    for (i = 0; s[i] != '\0'; i++) {
        userAgent[i] = s[i];
    }

    char * p = userAgent + i;

    size_t n = 50 - i - 1;

    for (i = 0; (i < n) && LIBCURL_VERSION[i] != '\0'; i++) {
        p[i] = LIBCURL_VERSION[i];
    }

    p[i] = '\0';

    ///////////////////////////////////////////////////////////

    char * transformedUrl = NULL;

    switch (transform_url(url, &transformedUrl)) {
        case -1: return XCPKG_ERROR_MEMORY_ALLOCATE;
        case  1: url = transformedUrl;
    }

    if (verbose) {
        fprintf(stderr, "Fetching: %s\n", url);
    }

    ///////////////////////////////////////////////////////////

    curl_global_init(CURL_GLOBAL_ALL);

    CURL * curl = curl_easy_init();

    // https://curl.se/libcurl/c/CURLOPT_URL.html
    curl_easy_setopt(curl, CURLOPT_URL, url);

    // https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
    //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

    // https://curl.se/libcurl/c/CURLOPT_WRITEDATA.html
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputFile);

    // https://curl.se/libcurl/c/CURLOPT_FOLLOWLOCATION.html
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // https://curl.se/libcurl/c/CURLOPT_FAILONERROR.html
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    // https://curl.se/libcurl/c/CURLOPT_TIMEOUT.html
    //curl_easy_setopt(curl, CURLOPT_TIMEOUT, 300L);

    // https://curl.se/libcurl/c/CURLOPT_VERBOSE.html
    curl_easy_setopt(curl, CURLOPT_VERBOSE, verbose ? 1 : 0);

    // https://curl.se/libcurl/c/CURLOPT_NOPROGRESS.html
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, showProgress ? 0: 1L);

    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);

    // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
    const char * const SSL_CERT_FILE = getenv("SSL_CERT_FILE");

    if ((SSL_CERT_FILE != NULL) && (SSL_CERT_FILE[0] != '\0')) {
        // https://curl.se/libcurl/c/CURLOPT_CAINFO.html
        curl_easy_setopt(curl, CURLOPT_CAINFO, SSL_CERT_FILE);
    }

    const char * const SSL_CERT_DIR = getenv("SSL_CERT_DIR");

    if ((SSL_CERT_DIR != NULL) && (SSL_CERT_DIR[0] != '\0')) {
        // https://curl.se/libcurl/c/CURLOPT_CAPATH.html
        curl_easy_setopt(curl, CURLOPT_CAPATH, SSL_CERT_DIR);
    }

    ///////////////////////////////////////////////////////////

    struct curl_slist *list = NULL;

    //list = curl_slist_append(list, "Accept: *");
    list = curl_slist_append(list, userAgent);

    // https://curl.se/libcurl/c/CURLOPT_HTTPHEADER.html
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

    ///////////////////////////////////////////////////////////

    CURLcode curlcode = curl_easy_perform(curl);

    // https://curl.se/libcurl/c/libcurl-errors.html
    if (curlcode != CURLE_OK) {
        fprintf(stderr, "SSL_CERT_FILE=%s\n", SSL_CERT_FILE);
        fprintf(stderr, "%s\n", curl_easy_strerror(curlcode));

        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
        // https://curl.se/libcurl/c/CURLINFO_RESPONSE_CODE.html
        long httpResponseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);
        fprintf(stderr, "%ld: %s\n", httpResponseCode, url);

    }

    curl_slist_free_all(list);

    curl_easy_cleanup(curl);

    curl_global_cleanup();

    if (transformedUrl != NULL) {
        free(transformedUrl);;
    }

    if (curlcode == 0) {
        return XCPKG_OK;
    } else {
        return abs((int)curlcode) + XCPKG_ERROR_NETWORK_BASE;
    }
}

int xcpkg_http_fetch_to_file(const char * url, const char * outputFilePath, const bool verbose, const bool showProgress) {
    FILE * file = fopen(outputFilePath, "wb");

    if (file == NULL) {
        perror(outputFilePath);
        return XCPKG_ERROR;
    }

    int ret = xcpkg_http_fetch_to_stream(url, file, verbose, showProgress);

    fclose(file);

    return ret;
}

typedef int (*xcpkg_http_fetch_to_fn)(const char * url, const void * to, const bool verbose, const bool showProgress);

static inline int startswith(const char * s1, const char * s2) {
    if (s1 == NULL || s2 == NULL) {
        return 1;
    }

    while (s2[0] != '\0') {
        if (s1[0] != s2[0]) {
            return 1;
        }

        s1++;
        s2++;
    }

    return 0;
}

static inline int xcpkg_http_fetch_internal(const char * url, const char * uri, const void * to, xcpkg_http_fetch_to_fn xcpkg_http_fetch_to, const bool verbose) {
    int ret = xcpkg_http_fetch_to(url, to, verbose, verbose);

    if (ret == XCPKG_OK) {
        return XCPKG_OK;
    }

    if (uri != NULL && uri[0] != '\0') {
        ret = xcpkg_http_fetch_to(uri, to, verbose, verbose);

        if (ret == XCPKG_OK) {
            return XCPKG_OK;
        }
    }

    size_t slashIndex = 0U;

    size_t n = 0U;

    for (; ; n++) {
        if (url[n] == '\0') break;
        if (url[n] == '?')  break;
        if (url[n] == '/')  slashIndex = n;
    }

    if (slashIndex == 0U) {
        return XCPKG_ERROR_INVALID_URL;
    }

    if (startswith(url, "https://ftp.gnu.org/gnu/") == 0) {
        const char * s = "https://ftpmirror.gnu.org/gnu/";

        char buf[n + 7];

        int j;

        for (j = 0; s[j] != '\0'; j++) {
            buf[j] = s[j];
        }

        char * p = buf + j;

        s = url + j - 6;

        while (s[0] != '\0' && s[0] != '?') {
            p[0] = s[0];
            p++;
            s++;
        }

        p[0] = '\0';

        ret = xcpkg_http_fetch_to(buf, to, verbose, verbose);

        if (ret == XCPKG_OK) {
            return XCPKG_OK;
        }
    }

    ///////////////////////////////////////////////////

    const char * s = "https://fossies.org/linux/misc/";

    size_t capacity = strlen(s) + n - slashIndex;

    char buf[capacity];

    char * p = buf;

    while (s[0] != '\0') {
        p[0] = s[0];
        p++;
        s++;
    }

    s = url + slashIndex + 1;

    while (s[0] != '\0' && s[0] != '?') {
        p[0] = s[0];
        p++;
        s++;
    }

    p[0] = '\0';

    return xcpkg_http_fetch_to(buf, to, verbose, verbose);
}

int xcpkg_http_fetch(const char * url, const char * uri, const char * expectedSHA256SUM, const char * outputPath, const bool verbose) {
    if (verbose) {
        fprintf(stderr, "Fetching: %s %s %s => %s\n", url, uri, expectedSHA256SUM, outputPath);
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

    if (outputPath == NULL || outputPath[0] == '\0' || (outputPath[0] == '-' && outputPath[1] == '\0') || strcmp(outputPath, "/dev/stdout") == 0) {
        return xcpkg_http_fetch_internal(url, uri, stdout, (xcpkg_http_fetch_to_fn)xcpkg_http_fetch_to_stream, verbose);
    }

    if (strcmp(outputPath, "/dev/stderr") == 0) {
        return xcpkg_http_fetch_internal(url, uri, stderr, (xcpkg_http_fetch_to_fn)xcpkg_http_fetch_to_stream, verbose);
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

    ret = xcpkg_http_fetch_internal(url, uri, tmpFilePath, (xcpkg_http_fetch_to_fn)xcpkg_http_fetch_to_file, verbose);

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

int xcpkg_http_fetch_then_unpack(const char * url, const char * uri, const char * expectedSHA256SUM, const char * downloadDIR, size_t downloadDIRLength, const char * unpackDIR, size_t unpackDIRLength, const bool verbose) {
    char fileType[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

    int ret = xcpkg_extract_filetype_from_url(url, fileType, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t fileNameCapacity = strlen(fileType) + 65U;
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

    ret = xcpkg_http_fetch_internal(url, uri, filePath, (xcpkg_http_fetch_to_fn)xcpkg_http_fetch_to_file, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (unpackDIR == NULL) {
        return XCPKG_OK;
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
