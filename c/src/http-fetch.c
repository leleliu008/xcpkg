#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>

#include <curl/curl.h>
#include <curl/curlver.h>

#include "core/url-transform.h"

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

    fprintf(stderr, "Fetching: %s\n", url);

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
        fprintf(stderr, "%s\n", curl_easy_strerror(curlcode));
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
        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Status
        // https://curl.se/libcurl/c/CURLINFO_RESPONSE_CODE.html
        long httpResponseCode;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpResponseCode);
        fprintf(stderr, "%ld: %s\n", httpResponseCode, url);

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
