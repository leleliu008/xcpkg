#include <string.h>

#include <unistd.h>

#include "../core/base16.h"

#include "../xcpkg.h"

#include "../util.h"

static inline int xcpkg_util_base16_encode_stdin() {
    unsigned char inputBuf[1024];

    for (;;) {
        ssize_t readSizeInBytes = read(STDIN_FILENO, inputBuf, 1024);

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

        size_t outputBufSizeInBytes = readSizeInBytes << 1;
        char   outputBuf[outputBufSizeInBytes];

        if (base16_encode(outputBuf, inputBuf, readSizeInBytes, true) != 0) {
            perror(NULL);
            return XCPKG_ERROR;
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

static inline int xcpkg_util_base16_encode_string(const char * s) {
    unsigned char * inputBuf = (unsigned char *)s;
    size_t          inputBufSizeInBytes = strlen(s);

    size_t outputBufSizeInBytes = inputBufSizeInBytes << 1;
    char   outputBuf[outputBufSizeInBytes];

    if (base16_encode(outputBuf, inputBuf, inputBufSizeInBytes, true) != 0) {
        return XCPKG_ERROR;
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
 *  xcpkg util base16-encode <STR>
 *  xcpkg util base16-encode < input/file/path
 */
int xcpkg_util_base16_encode(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        return xcpkg_util_base16_encode_stdin();
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "Usage: %s %s %s <STR> , <STR> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    return xcpkg_util_base16_encode_string(argv[3]);
}
