#include <stdio.h>

#include <limits.h>
#include <sys/stat.h>

#include "xcpkg.h"

int xcpkg_setenv_SSL_CERT_FILE() {
    if (getenv("SSL_CERT_FILE") != NULL) {
        return XCPKG_OK;
    }

    int ret;

    char cacertFilePath[PATH_MAX];

    const char * const xcpkgHomeDIR = getenv("XCPKG_HOME");

    if (xcpkgHomeDIR == NULL || xcpkgHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
            return XCPKG_ERROR_ENV_HOME_NOT_SET;
        }

        ret = snprintf(cacertFilePath, PATH_MAX, "%s/.xcpkg/core/cacert.pem", userHomeDIR);
    } else {
        ret = snprintf(cacertFilePath, PATH_MAX, "%s/core/cacert.pem", xcpkgHomeDIR);
    }

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    char* a[2] = {cacertFilePath, "/etc/ssl/cert.pem"};

    for (int i = 0; i < 2; i++) {
        if (stat(a[i], &st) == 0 && S_ISREG(st.st_mode)) {
            // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
            if (setenv("SSL_CERT_FILE", a[i], 1) != 0) {
                perror("SSL_CERT_FILE");
                return XCPKG_ERROR;
            } else {
                return XCPKG_OK;
            }
        }
    }

    return XCPKG_OK;
}
