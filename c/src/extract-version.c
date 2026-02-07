#include <stddef.h>

#include "xcpkg.h"

/**
 * v2_0_2.tar.gz
 * v2023.2.tar.gz
 * V0.3.0.tar.gz
 * r2.5.tar.gz
 * R70.11.tar.gz
 *
 * gping-v1.16.1.tar.gz
 * oxlint_v1.41.0.tar.gz
 *
 * expat-2.6.4.tar.lz
 *
 * m4-1.4.20.tar.xz
 * lcms2-2.16.tar.gz
 * aria2-1.37.0.tar.xz
 * libidn2-2.3.8.tar.gz
 *
 * tectonic@0.15.0.tar.gz
 * mediainfo_24.05.tar.bz2
 * x265_4.1.tar.gz
 *
 * v0.4.4-alpha.tar.gz
 * v0.15.0-beta.tar.gz
 * v5.7.0-stable.tar.gz
 * upx-4.2.4-src.tar.xz
 * helix-25.01.1-source.tar.xz
 * llvm-project-18.1.8.src.tar.xz
 * libiberty_20230104.orig.tar.xz
 *
 * icu4c-75_1-src.tgz
 *
 * rtmpdump_2.4+20151223.gitfa8646d.1.orig.tar.gz
 *
 * quickjs-2025-09-13-2.tar.xz
 *
 * mksh-R59c.tgz
 * jpegsrc.v9f.tar.gz
 * tmux-3.6a.tar.gz
 * autossh-1.4g.tgz
 * openssl-1.1.1w.tar.gz
 *
 *
 * 3.1.2
 * v3.1.2
 * refs/tags/v3.1.2
 */
int xcpkg_extract_version(const char * s, char versionBuf[], size_t versionBufCapacity) {
    if (s == NULL) {
        return XCPKG_OK;
    }

    if (s[0] == '\0') {
        return XCPKG_OK;
    }

    const char * p = s;

    for (int i = 0; ; i++) {
        if (s[i] == '\0') {
            break;
        }

        if (s[i] == '/') {
            p = s + i + 1;
        }
    }

    s = p;

    size_t len;

    const char * q = s;

loop:
    if (p[0] == '\0') {
        goto finally;
    }

    if (s == p) {
        if (p[0] == 'v' || p[0] == 'V' || p[0] == 'r' || p[0] == 'R') {
            p++;
            s = p;
            q = p;
        }
    }

    int has = 0;

    for (int i = 0; ; i++) {
        if (p[i] == '\0') {
            if (has) {
                if (s == q) {
                    p += i;
                    s = p;
                    q = p;
                }
                goto finally;
            } else {
                q = &p[i];
                p = q + 1;
                goto loop;
            }
        }

        if (p[i] == '.' || p[i] == '-' || p[i] == '_' || p[i] == '@' || p[i] == '+') {
            if (has) {
                if (s == q) {
                    p += i + 1;
                    s = p;
                    q = p;
                    goto loop;
                } else {
                    goto finally;
                }
            } else {
                q = &p[i];
                p = q + 1;
                goto loop;
            }
        } else {
            if (p[i] < '0' || p[i] > '9') {
                has = 1;
            }
        }
    }

finally:
    len = q - s;

    if (len > versionBufCapacity) {
        len = versionBufCapacity;
    }

    for (size_t i = 0U; i < len; i++) {
        if (s[i] == '\0') {
            break;
        }

        if (s[i] == '-' || s[i] == '_' || s[i] == '@' || s[i] == '+') {
            versionBuf[i] = '.';
        } else {
            versionBuf[i] = s[i];
        }
    }
    
    versionBuf[len] = '\0';

    return XCPKG_OK;
}
