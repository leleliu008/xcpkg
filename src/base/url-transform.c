#include <stdlib.h>
#include <string.h>

int transform_url(const char * url, char ** outP) {
    const char * const githubProxyUrl = getenv("XCPKG_GITHUB_PROXY");

    if (githubProxyUrl == NULL || githubProxyUrl[0] == '\0') {
        return 0;
    }

    if (strncmp(url, "https://github.com/", 19) != 0) {
        return 0;
    }

    char * buf = malloc(strlen(url) + strlen(githubProxyUrl) + 1);

    if (buf == NULL) {
        return -1;
    }

    int i;

    for (i = 0; ; i++) {
        buf[i] = githubProxyUrl[i];

        if (githubProxyUrl[i] == '\0') {
            if (githubProxyUrl[i - 1] != '/') {
                buf[i] = '/';
                i++;
            }

            break;
        }
    }

    char * p = buf + i;

    for (i = 0; ; i++) {
        p[i] = url[i];

        if (url[i] == '\0') {
            break;
        }
    }

    *outP = buf;

    return 1;
}
