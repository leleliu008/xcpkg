#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include <openssl/evp.h>

#include "../xcpkg.h"

#include "../util.h"

static inline int xcpkg_util_base64_encode_stdin() {
    unsigned char inputBuf[1023];

    for (;;) {
        ssize_t readSizeInBytes = read(STDIN_FILENO, inputBuf, 1023);

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

        unsigned int  x = (readSizeInBytes % 3) == 0 ? 0 : 1;
        unsigned int  outputBufSizeInBytes = (readSizeInBytes / 3 + x) << 2;
        unsigned char outputBuf[outputBufSizeInBytes];

        int ret = EVP_EncodeBlock(outputBuf, inputBuf, readSizeInBytes);

        if (ret < 0) {
            return ret;
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

static inline int xcpkg_util_base64_encode_string(const char * s) {
    unsigned char * inputBuf = (unsigned char *)s;
    unsigned int    inputBufSizeInBytes = strlen(s);

    unsigned int  x = (inputBufSizeInBytes % 3) == 0 ? 0 : 1;
    unsigned int  outputBufSizeInBytes = (inputBufSizeInBytes / 3 + x) << 2;
    unsigned char outputBuf[outputBufSizeInBytes];

    int ret = EVP_EncodeBlock(outputBuf, inputBuf, inputBufSizeInBytes);

    if (ret < 0) {
        return ret;
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
 *  xcpkg util base64-encode <STR>
 *  xcpkg util base64-encode < input/file/path
 */
int xcpkg_util_base64_encode(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        return xcpkg_util_base64_encode_stdin();
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "Usage: %s %s %s <STR> , <STR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    return xcpkg_util_base64_encode_string(argv[3]);
}
