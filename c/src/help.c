#include <stdio.h>
#include <string.h>

#include "core/base16.h"

#include "xcpkg.h"

int xcpkg_help() {
    size_t iLength = strlen(XCPKG_HELP);
    size_t pLength = iLength >> 1;
    size_t pCapacity = pLength + 1U;
    unsigned char   p[pCapacity];
    p[pLength] = '\0';

    int ret = base16_decode(p, XCPKG_HELP, iLength);

    if (ret == -1) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    printf("%s\n", p);

    return 0;
}
