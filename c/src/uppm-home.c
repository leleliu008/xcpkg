#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "uppm.h"
#include "xcpkg.h"

int uppm_home_dir(char buf[], size_t * len) {
    if (buf == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    char   tmpBuf[PATH_MAX];
    size_t tmpBufLength;

    int ret = xcpkg_home_dir(tmpBuf, &tmpBufLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    const char * const str = "/uppm";
    size_t strLength = strlen(str);

    strncpy(tmpBuf + tmpBufLength, str, strLength + sizeof(char));

    struct stat st;

    if (stat(tmpBuf, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            fprintf(stderr, "%s was expected to be a directory, but it was not.\n", buf);
            return XCPKG_ERROR;
        }
    } else {
        if (mkdir(tmpBuf, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(tmpBuf);
                return XCPKG_ERROR;
            }
        }
    }

    tmpBufLength += strLength;

    if (len != NULL) {
        (*len) = tmpBufLength;
    }

    strncpy(buf, tmpBuf, tmpBufLength);
    buf[tmpBufLength] = '\0';

    return XCPKG_OK;
}
