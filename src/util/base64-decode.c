#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <openssl/evp.h>

#include "../xcpkg.h"

#include "../util.h"

static inline int xcpkg_util_base64_decode_stdin() {
    unsigned char readBuf[1024];

    for (;;) {
        ssize_t readSizeInBytes = read(STDIN_FILENO, readBuf, 1024);

        if (readSizeInBytes == -1) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (readSizeInBytes == 0) {
            if (isatty(STDOUT_FILENO)) {
                printf("\n");
            }

            return XCPKG_OK;
        }

        unsigned int  outputBufSizeInBytes = (readSizeInBytes >> 2) * 3;
        unsigned char outputBuf[outputBufSizeInBytes];

        // EVP_DecodeBlock() returns the length of the data decoded or -1 on error.
        int n = EVP_DecodeBlock(outputBuf, readBuf, readSizeInBytes);

        if (n < 0) {
            return XCPKG_ERROR_ARG_IS_INVALID;
        }

        ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

        if (writeSizeInBytes == -1) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
            fprintf(stderr, "not fully written to stdout.\n");
            return XCPKG_ERROR;
        }
    }
}

static inline int xcpkg_util_base64_decode_string(const char * s) {
    unsigned char * inputBuf = (unsigned char *)s;
    unsigned int    inputBufSizeInBytes = strlen(s);

    unsigned int  outputBufSizeInBytes = (inputBufSizeInBytes >> 2) * 3;
    unsigned char outputBuf[outputBufSizeInBytes];

    // EVP_DecodeBlock() returns the length of the data decoded or -1 on error.
    int n = EVP_DecodeBlock(outputBuf, inputBuf, inputBufSizeInBytes);

    if (n < 0) {
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    ssize_t writeSizeInBytes = write(STDOUT_FILENO, outputBuf, outputBufSizeInBytes);

    if (writeSizeInBytes == -1) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if ((size_t)writeSizeInBytes != outputBufSizeInBytes) {
        fprintf(stderr, "not fully written to stdout.\n");
        return XCPKG_ERROR;
    }

    if (isatty(STDOUT_FILENO)) {
        printf("\n");
    }

    return XCPKG_OK;
}

/**
 *  xcpkg util base64-decode <BASE64-ENCODED-STR>
 *  xcpkg util base64-decode < input/file/path
 */
int xcpkg_util_base64_decode(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        return xcpkg_util_base64_decode_stdin();
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "Usage: %s %s %s <BASE64-ENCODED-STR> , <BASE64-ENCODED-STR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    return xcpkg_util_base64_decode_string(argv[3]);
}
