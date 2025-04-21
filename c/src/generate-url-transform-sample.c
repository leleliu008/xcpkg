#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>

#include "core/log.h"
#include "core/base16.h"

#include "xcpkg.h"

int xcpkg_generate_url_transform_sample() {
    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t outFilePathCapacity = xcpkgHomeDIRLength + 22U;
    char   outFilePath[outFilePathCapacity];

    ret = snprintf(outFilePath, outFilePathCapacity, "%s/url-transform.sample", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    size_t iLength = strlen(XCPKG_URL_TRANSFORM_SAMPLE);
    size_t pLength = iLength >> 1;
    size_t pCapacity = pLength + 1U;
    unsigned char p[pCapacity]; p[pLength] = '\0';

    ret = base16_decode(p, XCPKG_URL_TRANSFORM_SAMPLE, iLength);

    if (ret == -1) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_write_file(outFilePath, (char*)p, pLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    if (chmod(outFilePath, S_IRWXU) != 0) {
        perror(outFilePath);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "%surl-transform sample has been written into %s%s\n\n", COLOR_GREEN, outFilePath, COLOR_OFF);

    outFilePath[outFilePathCapacity - 8U] = '\0';

    fprintf(stderr, "%sYou can rename url-transform.sample to url-transform then edit it to meet your needs.\n\nTo apply this, you should run 'export XCPKG_URL_TRANSFORM=%s' in your terminal.\n%s", COLOR_GREEN, outFilePath, COLOR_OFF);

    return XCPKG_OK;
}
