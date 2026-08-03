#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "../core/base16.h"
#include "../xcpkg.h"
#include "../util.h"

/**
 *  xcpkg util base16-decode <BASE16-ENCODED-STR>
 *  xcpkg util base16-decode < input/file/path
 */
int xcpkg_util_base16_decode(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        fprintf(stderr, "Usage: %s %s %s <BASE16-ENCODED-STR> , <BASE16-ENCODED-STR> is unspecified.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "Usage: %s %s %s <BASE16-ENCODED-STR> , <BASE16-ENCODED-STR> should be non-empty.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    size_t inputBufSizeInBytes = strlen(argv[3]);

    if ((inputBufSizeInBytes & 1) != 0) {
        fprintf(stderr, "Usage: %s %s %s <BASE16-ENCODED-STR> , <BASE16-ENCODED-STR> length should be an even number.\n", argv[0], argv[1], argv[2]);
        return XCPKG_ERROR_ARG_IS_INVALID;
    }

    size_t        outputBufSizeInBytes = inputBufSizeInBytes >> 1;
    unsigned char outputBuf[outputBufSizeInBytes];

    if (base16_decode(outputBuf, argv[3], inputBufSizeInBytes) == 0) {
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
    } else {
        perror(NULL);

        if (errno == EINVAL) {
            return XCPKG_ERROR_ARG_IS_INVALID;
        } else {
            return XCPKG_ERROR;
        }
    }
}
