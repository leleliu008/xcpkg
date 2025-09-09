#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/printenv.h"
#include "core/sysinfo.h"
#include "core/self.h"
#include "core/exe.h"
#include "core/log.h"

#include "sha256sum.h"
#include "native-package.h"
#include "xcpkg.h"
#include "uppm.h"

typedef struct {
    const char * name;
    const char * value;
} KV;

typedef struct {
    const char * name;
    bool         value;
} KB;

// https://www.gnu.org/software/gettext/manual/html_node/config_002eguess.html
// https://git.savannah.gnu.org/cgit/config.git/tree/
static int fetch_gnu_config(const char * sessionDIR, const size_t sessionDIRCapacity, bool verbose) {
    size_t gnuconfigDIRCapacity = sessionDIRCapacity + 7U;
    char   gnuconfigDIR[gnuconfigDIRCapacity];

    int ret = snprintf(gnuconfigDIR, gnuconfigDIRCapacity, "%s/config", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (mkdir(gnuconfigDIR, S_IRWXU) != 0) {
        if (errno == EEXIST) {
            return XCPKG_OK;
        } else {
            perror(gnuconfigDIR);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_git_sync(gnuconfigDIR, "https://github.com/leleliu008/gnu-config", "HEAD", "refs/remotes/origin/master", "master", 1);

    if (ret == XCPKG_OK) {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t configSubFilePathCapacity = gnuconfigDIRCapacity + 12U;
    char   configSubFilePath[configSubFilePathCapacity];

    ret = snprintf(configSubFilePath, configSubFilePathCapacity, "%s/config.sub", gnuconfigDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t configGuessFilePathCapacity = gnuconfigDIRCapacity + 14U;
    char   configGuessFilePath[configGuessFilePathCapacity];

    ret = snprintf(configGuessFilePath, configGuessFilePathCapacity, "%s/config.guess", gnuconfigDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_http_fetch_to_file("https://git.savannah.gnu.org/cgit/config.git/plain/config.sub", configSubFilePath, verbose, verbose);

    if (ret != XCPKG_OK) {
        goto alternative;
    }

    ret = xcpkg_http_fetch_to_file("https://git.savannah.gnu.org/cgit/config.git/plain/config.guess", configGuessFilePath, verbose,verbose);

    if (ret == XCPKG_OK) {
        goto finally;
    } else {
        goto alternative;
    }

    //////////////////////////////////////////////////////////////////////////////

alternative:
    ret = xcpkg_http_fetch_to_file("https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.sub;hb=HEAD", configSubFilePath, verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ret = xcpkg_http_fetch_to_file("https://git.savannah.gnu.org/gitweb/?p=config.git;a=blob_plain;f=config.guess;hb=HEAD", configGuessFilePath, verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

finally:
    if (chmod(configSubFilePath, S_IRWXU) == -1) {
        perror(configSubFilePath);
        return XCPKG_ERROR;
    }

    if (chmod(configGuessFilePath, S_IRWXU) == -1) {
        perror(configGuessFilePath);
        return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

static int fetch_fixlist(const char * fixlist, const char * xcpkgDownloadsDIR, const size_t xcpkgDownloadsDIRCapacity, bool verbose) {
    size_t  bufCapacity = strlen(fixlist) + 1U;
    char    buf[bufCapacity];
    strncpy(buf, fixlist, bufCapacity);

    char * p = buf;

    int ret;

    int fd = -1;

    while (p[0] != '\0') {
        char * sha = NULL;
        char * url = NULL;
        char * uri = NULL;
        char * opt = NULL;

        ////////////////// sha //////////////////

        for (int i = 0; ; i++) {
            if (p[i] == '\0') {
                fprintf(stderr, "fixlist mapping is invalid.\n");
                return XCPKG_ERROR_FORMULA_SCHEME;
            }

            if (p[i] == '|') {
                if (i == 64) {
                    sha = p;
                    p[i] = '\0';
                    p += i + 1;
                    break;
                } else {
                    fprintf(stderr, "fixlist mapping is invalid. sha256sum length shall be 64.\n");
                    return XCPKG_ERROR_FORMULA_SCHEME;
                }
            }
        }

        ////////////////// url //////////////////

        for (int i = 0; ; i++) {
            if (p[i] == '\n') {
                p[i] = '\0';
                url = p;
                p += i + 1;
                goto action;
            }

            if (p[i] == '\0') {
                p[i] = '\0';
                url = p;
                p = &p[i];
                goto action;
            }

            if (p[i] == '|') {
                p[i] = '\0';
                url = p;
                p += i + 1;
                break;
            }
        }

        ////////////////// uri //////////////////

        for (int i = 0; ; i++) {
            if (p[i] == '\n') {
                p[i] = '\0';
                uri = p;
                p += i + 1;
                goto action;
            }

            if (p[i] == '\0') {
                p[i] = '\0';
                uri = p;
                p = &p[i];
                goto action;
            }

            if (p[i] == '|') {
                p[i] = '\0';
                uri = p;
                p += i + 1;
                break;
            }
        }

        ////////////////// opt //////////////////

        for (int i = 0; ; i++) {
            if (p[i] == '\n') {
                p[i] = '\0';
                opt = p;
                p += i + 1;
                goto action;
            }

            if (p[i] == '\0') {
                p[i] = '\0';
                opt = p;
                p = &p[i];
                goto action;
            }

            if (p[i] == '|') {
                fprintf(stderr, "fixlist mapping is invalid.\n");
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
        }

        action:
        ret = xcpkg_http_fetch_then_unpack(url, uri, sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "fix", 4U, verbose);

        if (ret != XCPKG_OK) {
            if (fd != -1) {
                close(fd);
            }
            return ret;
        }

        char ft[XCPKG_FILE_EXTENSION_MAX_CAPACITY]; ft[0] = '\0';

        ret = xcpkg_extract_filetype_from_url(url, ft, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            if (fd != -1) {
                close(fd);
            }
            return ret;
        }

        if (fd == -1) {
            const char * fp = "fix/index";

            fd = open(fp, O_CREAT | O_TRUNC | O_WRONLY, 0666);

            if (fd == -1) {
                perror(fp);
                return XCPKG_ERROR;
            }
        }

        ret = dprintf(fd, "%s%s|%s\n", sha, ft, (opt == NULL) ? "" : opt);

        if (ret < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }
    }

    if (fd != -1) {
        close(fd);
    }

    return XCPKG_OK;
}

static int fetch_reslist(const char * reslist, const char * xcpkgDownloadsDIR, const size_t xcpkgDownloadsDIRCapacity, bool verbose) {
    size_t  bufCapacity = strlen(reslist) + 1U;
    char    buf[bufCapacity];
    strncpy(buf, reslist, bufCapacity);

    char * p = buf;

    int ret;

    while (p[0] != '\0') {
        char * sha = NULL;
        char * url = NULL;
        char * uri = NULL;

        for (int i = 0; ; i++) {
            if (p[i] == '\0') {
                fprintf(stderr, "reslist mapping is invalid.\n");
                return XCPKG_ERROR_FORMULA_SCHEME;
            }

            if (p[i] == '|') {
                if (i == 64) {
                    sha = p;
                    p[i] = '\0';
                    p += i + 1;
                    break;
                } else {
                    fprintf(stderr, "reslist mapping is invalid. sha256sum length shall be 64.\n");
                    return XCPKG_ERROR_FORMULA_SCHEME;
                }
            }
        }

        ////////////////////////////////////

        for (int i = 0; ; i++) {
            if (p[i] == '\n') {
                p[i] = '\0';
                url = p;
                p += i + 1;
                goto action;
            }

            if (p[i] == '\0') {
                p[i] = '\0';
                url = p;
                p = &p[i];
                goto action;
            }

            if (p[i] == '|') {
                p[i] = '\0';
                url = p;
                p += i + 1;
                break;
            }
        }

        ////////////////////////////////////

        for (int i = 0; ; i++) {
            if (p[i] == '\n') {
                p[i] = '\0';
                uri = p;
                p += i + 1;
                goto action;
            }

            if (p[i] == '\0') {
                p[i] = '\0';
                uri = p;
                p = &p[i];
                goto action;
            }

            if (p[i] == '|') {
                fprintf(stderr, "reslist mapping is invalid.\n");
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
        }

        action:
        ret = xcpkg_http_fetch_then_unpack(url, uri, sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "res", 4U, verbose);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

static int setup_rust_toolchain(const XCPKGInstallOptions * installOptions, const char * sessionDIR, const size_t sessionDIRLength) {
    const char * cargoHomeDIR = getenv("CARGO_HOME");

    if (cargoHomeDIR == NULL || cargoHomeDIR[0] == '\0') {
        const char * const userHomeDIR = getenv("HOME");

        if (userHomeDIR == NULL || userHomeDIR[0] == '\0') {
            return XCPKG_ERROR_ENV_HOME_NOT_SET;
        }

        size_t defaultCargoHomeDIRCapacity = strlen(userHomeDIR) + 8U;
        char   defaultCargoHomeDIR[defaultCargoHomeDIRCapacity];

        int ret = snprintf(defaultCargoHomeDIR, defaultCargoHomeDIRCapacity, "%s/.cargo", userHomeDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CARGO_HOME", defaultCargoHomeDIR, 1) != 0) {
            perror("CARGO_HOME");
            return XCPKG_ERROR;
        }

        cargoHomeDIR = getenv("CARGO_HOME");
    }

    size_t cargoHomeDIRLength = strlen(cargoHomeDIR);

    size_t rustupCommandPathCapacity = cargoHomeDIRLength + 12U;
    char   rustupCommandPath[rustupCommandPathCapacity];

    int ret = snprintf(rustupCommandPath, rustupCommandPathCapacity, "%s/bin/rustup", cargoHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t cargoCommandPathCapacity = cargoHomeDIRLength + 11U;
    char   cargoCommandPath[cargoCommandPathCapacity];

    ret = snprintf(cargoCommandPath, cargoCommandPathCapacity, "%s/bin/cargo", cargoHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    const bool  cargoExist = (stat( cargoCommandPath, &st) == 0) && S_ISREG(st.st_mode);
    const bool rustupExist = (stat(rustupCommandPath, &st) == 0) && S_ISREG(st.st_mode);

    if (!(cargoExist && rustupExist)) {
        LOG_INFO("rustup and cargo commands are required, but they are not found on this machine, xcpkg will install them via running shell script.");

        size_t rustupInitScriptFilePathCapacity = sessionDIRLength + 16U;
        char   rustupInitScriptFilePath[rustupInitScriptFilePathCapacity];

        ret = snprintf(rustupInitScriptFilePath, rustupInitScriptFilePathCapacity, "%s/rustup-init.sh", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_http_fetch_to_file("https://sh.rustup.rs", rustupInitScriptFilePath, installOptions->verbose_net, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_fork_exec2(3, "bash", rustupInitScriptFilePath, "-y");

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return XCPKG_ERROR_ENV_PATH_NOT_SET;
    }

    size_t newPATHCapacity = cargoHomeDIRLength + strlen(PATH) + 6U;
    char   newPATH[newPATHCapacity];

    ret = snprintf(newPATH, newPATHCapacity, "%s/bin:%s", cargoHomeDIR, PATH);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("PATH", newPATH, 1) != 0) {
        perror("PATH");
        return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

typedef int (*setenv_fn)(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity);

static int setenv_CPPFLAGS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t includeDIRCapacity = packageInstalledDIRCapacity + 9U;
    char   includeDIR[includeDIRCapacity];

    int ret = snprintf(includeDIR, includeDIRCapacity, "%s/include", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(includeDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const CPPFLAGS = getenv("CPPFLAGS");

        if (CPPFLAGS == NULL || CPPFLAGS[0] == '\0') {
            size_t newCPPFLAGSCapacity = includeDIRCapacity + 3U;
            char   newCPPFLAGS[newCPPFLAGSCapacity];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSCapacity, "-I%s", includeDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return XCPKG_ERROR;
            }
        } else {
            size_t newCPPFLAGSCapacity = includeDIRCapacity + strlen(CPPFLAGS) + 4U;
            char   newCPPFLAGS[newCPPFLAGSCapacity];

            ret = snprintf(newCPPFLAGS, newCPPFLAGSCapacity, "-I%s %s", includeDIR, CPPFLAGS);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("CPPFLAGS", newCPPFLAGS, 1) != 0) {
                perror("CPPFLAGS");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_LDFLAGS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t libDIRCapacity = packageInstalledDIRCapacity + 5U;
    char   libDIR[libDIRCapacity];

    int ret = snprintf(libDIR, libDIRCapacity, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(libDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const LDFLAGS = getenv("LDFLAGS");

        if (LDFLAGS == NULL || LDFLAGS[0] == '\0') {
            size_t newLDFLAGSCapacity = libDIRCapacity + 3U;
            char   newLDFLAGS[newLDFLAGSCapacity];

            ret = snprintf(newLDFLAGS, newLDFLAGSCapacity, "-L%s", libDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return XCPKG_ERROR;
            }
        } else {
            size_t newLDFLAGSCapacity = (libDIRCapacity << 1U) + strlen(LDFLAGS) + 15U;
            char   newLDFLAGS[newLDFLAGSCapacity];

            ret = snprintf(newLDFLAGS, newLDFLAGSCapacity, "%s -L%s -Wl,-rpath,%s", LDFLAGS, libDIR, libDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("LDFLAGS", newLDFLAGS, 1) != 0) {
                perror("LDFLAGS");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_PKG_CONFIG_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    const char * a[2] = { "lib", "share" };

    for (int i = 0; i < 2; i++) {
        size_t pkgconfigDIRCapacity = packageInstalledDIRCapacity + 20U;
        char   pkgconfigDIR[pkgconfigDIRCapacity];

        int ret = snprintf(pkgconfigDIR, pkgconfigDIRCapacity, "%s/%s/pkgconfig", packageInstalledDIR, a[i]);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (stat(pkgconfigDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
            const char * const PKG_CONFIG_PATH = getenv("PKG_CONFIG_PATH");

            if (PKG_CONFIG_PATH == NULL || PKG_CONFIG_PATH[0] == '\0') {
                if (setenv("PKG_CONFIG_PATH", pkgconfigDIR, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return XCPKG_ERROR;
                }
            } else {
                size_t newPKG_CONFIG_PATHCapacity = pkgconfigDIRCapacity + strlen(PKG_CONFIG_PATH) + 2U;
                char   newPKG_CONFIG_PATH[newPKG_CONFIG_PATHCapacity];

                ret = snprintf(newPKG_CONFIG_PATH, newPKG_CONFIG_PATHCapacity, "%s:%s", PKG_CONFIG_PATH, pkgconfigDIR);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                if (setenv("PKG_CONFIG_PATH", newPKG_CONFIG_PATH, 1) != 0) {
                    perror("PKG_CONFIG_PATH");
                    return XCPKG_ERROR;
                }
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    struct stat st;

    size_t binDIRCapacity = packageInstalledDIRCapacity + 5U;
    char   binDIR[binDIRCapacity];

    int ret = snprintf(binDIR, binDIRCapacity, "%s/bin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t sbinDIRCapacity = packageInstalledDIRCapacity + 6U;
    char   sbinDIR[sbinDIRCapacity];

    ret = snprintf(sbinDIR, sbinDIRCapacity, "%s/sbin", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    const bool  binDIRExists = stat( binDIR, &st) == 0 && S_ISDIR(st.st_mode);
    const bool sbinDIRExists = stat(sbinDIR, &st) == 0 && S_ISDIR(st.st_mode);

    if (binDIRExists || sbinDIRExists) {
        const char * const PATH = getenv("PATH");

        if (PATH == NULL || PATH[0] == '\0') {
            return XCPKG_ERROR_ENV_PATH_NOT_SET;
        }

        if (binDIRExists && sbinDIRExists) {
            size_t newPATHLength = binDIRCapacity + sbinDIRCapacity + strlen(PATH) + 3U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s:%s", binDIR, sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return XCPKG_ERROR;
            }
        } else if (binDIRExists) {
            size_t newPATHLength = binDIRCapacity + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", binDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return XCPKG_ERROR;
            }
        } else if (sbinDIRExists) {
            size_t newPATHLength = sbinDIRCapacity + strlen(PATH) + 2U;
            char   newPATH[newPATHLength];

            ret = snprintf(newPATH, newPATHLength, "%s:%s", sbinDIR, PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("PATH", newPATH, 1) != 0) {
                perror("PATH");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_ACLOCAL_PATH(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t shareDIRCapacity = packageInstalledDIRCapacity + 7U;
    char   shareDIR[shareDIRCapacity];

    int ret = snprintf(shareDIR, shareDIRCapacity, "%s/share", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html

    size_t aclocalDIRCapacity = shareDIRCapacity + 9U;
    char   aclocalDIR[aclocalDIRCapacity];

    ret = snprintf(aclocalDIR, aclocalDIRCapacity, "%s/aclocal", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(aclocalDIR, &st) == 0 && S_ISDIR(st.st_mode)) {
        const char * const ACLOCAL_PATH = getenv("ACLOCAL_PATH");

        if (ACLOCAL_PATH == NULL || ACLOCAL_PATH[0] == '\0') {
            if (setenv("ACLOCAL_PATH", aclocalDIR, 1) != 0) {
                perror("ACLOCAL_PATH");
                return XCPKG_ERROR;
            }
        } else {
            size_t newACLOCAL_PATHLength = aclocalDIRCapacity + strlen(ACLOCAL_PATH) + 2U;
            char   newACLOCAL_PATH[newACLOCAL_PATHLength];

            ret = snprintf(newACLOCAL_PATH, newACLOCAL_PATHLength, "%s:%s", aclocalDIR, ACLOCAL_PATH);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("ACLOCAL_PATH", newACLOCAL_PATH, 1) != 0) {
                perror("ACLOCAL_PATH");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int setenv_XDG_DATA_DIRS(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t shareDIRCapacity = packageInstalledDIRCapacity + 7U;
    char   shareDIR[shareDIRCapacity];

    int ret = snprintf(shareDIR, shareDIRCapacity, "%s/share", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////
    // https://specifications.freedesktop.org/basedir-spec/basedir-spec-latest.html
    // https://gi.readthedocs.io/en/latest/tools/g-ir-scanner.html#environment-variables
    // https://help.gnome.org/admin//system-admin-guide/2.32/mimetypes-database.html.en

    size_t girSearchDIRCapacity = shareDIRCapacity + 9U;
    char   girSearchDIR[girSearchDIRCapacity];

    ret = snprintf(girSearchDIR, girSearchDIRCapacity, "%s/gir-1.0", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t mimeSearchDIRLength = shareDIRCapacity + 6U;
    char   mimeSearchDIR[mimeSearchDIRLength];

    ret = snprintf(mimeSearchDIR, mimeSearchDIRLength, "%s/mime", shareDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if ((stat(girSearchDIR, &st) == 0 && S_ISDIR(st.st_mode)) || (stat(mimeSearchDIR, &st) == 0 && S_ISDIR(st.st_mode))) {
        const char * const XDG_DATA_DIRS = getenv("XDG_DATA_DIRS");

        if (XDG_DATA_DIRS == NULL || XDG_DATA_DIRS[0] == '\0') {
            if (setenv("XDG_DATA_DIRS", shareDIR, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return XCPKG_ERROR;
            }
        } else {
            size_t newXDG_DATA_DIRSLength = shareDIRCapacity + strlen(XDG_DATA_DIRS) + 2U;
            char   newXDG_DATA_DIRS[newXDG_DATA_DIRSLength];

            ret = snprintf(newXDG_DATA_DIRS, newXDG_DATA_DIRSLength, "%s:%s", shareDIR, XDG_DATA_DIRS);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv("XDG_DATA_DIRS", newXDG_DATA_DIRS, 1) != 0) {
                perror("XDG_DATA_DIRS");
                return XCPKG_ERROR;
            }
        }
    }

    return XCPKG_OK;
}

static int native_package_installed_callback(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    setenv_fn funs[6] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

    for (int i = 0; i < 6; i++) {
        int ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

static int install_native_packages_via_uppm(
        const char * uppmPackageNames,
        const char * xcpkgHomeDIR,
        const size_t xcpkgHomeDIRLength,
        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRCapacity,
        const bool   verbose) {
    size_t uppmHomeDIRCapacity = xcpkgHomeDIRLength + 6U;
    char   uppmHomeDIR[uppmHomeDIRCapacity];

    int ret = snprintf(uppmHomeDIR, uppmHomeDIRCapacity, "%s/uppm", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t uppmHomeDIRLength = ret;

    ret = uppm_formula_repo_sync_official_core(uppmHomeDIR, uppmHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "uppm packages to be installed in order: %s\n", uppmPackageNames);

    const char * p = uppmPackageNames;

    char uppmPackageName[51];

    size_t i;

loop:
    if (p[0] == '\0') {
        return XCPKG_OK;
    }

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    //////////////////////////////////////////////////////////////////////////////

    for(i = 0U; p[i] != ' ' && p[i] != '\0'; i++) {
        if (i == 50U) {
            fprintf(stderr, "uppm package name must be no more than 50 characters\n");
            return XCPKG_ERROR;
        }

        uppmPackageName[i] = p[i];
    }

    uppmPackageName[i] = '\0';

    //////////////////////////////////////////////////////////////////////////////

    ret = uppm_install(uppmHomeDIR, uppmHomeDIRLength, uppmPackageName, verbose, false);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t uppmPackageInstalledDIRCapacity = uppmPackageInstalledRootDIRCapacity + strlen(uppmPackageName) + 2U;
    char   uppmPackageInstalledDIR[uppmPackageInstalledDIRCapacity];

    ret = snprintf(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity, "%s/%s", uppmPackageInstalledRootDIR, uppmPackageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = setenv_PATH(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ret = setenv_ACLOCAL_PATH(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ret = setenv_XDG_DATA_DIRS(uppmPackageInstalledDIR, uppmPackageInstalledDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (strcmp(uppmPackageName, "git") == 0) {
        // https://git-scm.com/book/en/v2/Git-Internals-Environment-Variables

        size_t gitCoreDIRCapacity = uppmPackageInstalledDIRCapacity + 18U;
        char   gitCoreDIR[gitCoreDIRCapacity];

        ret = snprintf(gitCoreDIR, gitCoreDIRCapacity, "%s/libexec/git-core", uppmPackageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("GIT_EXEC_PATH" , gitCoreDIR, 1) != 0) {
            perror("GIT_EXEC_PATH");
            return XCPKG_ERROR;
        }

        size_t gitTemplateDIRCapacity = uppmPackageInstalledDIRCapacity + 26U;
        char   gitTemplateDIR[gitTemplateDIRCapacity];

        ret = snprintf(gitTemplateDIR, gitTemplateDIRCapacity, "%s/share/git-core/templates", uppmPackageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("GIT_TEMPLATE_DIR" , gitTemplateDIR, 1) != 0) {
            perror("GIT_TEMPLATE_DIR");
            return XCPKG_ERROR;
        }
    } else if (strcmp(uppmPackageName, "docbook-xsl") == 0) {
        // http://xmlsoft.org/xslt/xsltproc.html

        size_t xmlCatalogFilePathCapacity = uppmPackageInstalledDIRCapacity + 13U;
        char   xmlCatalogFilePath[xmlCatalogFilePathCapacity];

        ret = snprintf(xmlCatalogFilePath, xmlCatalogFilePathCapacity, "%s/catalog.xml", uppmPackageInstalledDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("XML_CATALOG_FILES" , xmlCatalogFilePath, 1) != 0) {
            perror("XML_CATALOG_FILES");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    p = &p[i];

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    return XCPKG_OK;
}

/**
 * a is \0 or \n or space terminated
 * b is \0 terminated
 */
static inline __attribute__((always_inline)) bool _str_equal(const char * a, const char * b) {
    for (;;) {
        if (a[0] == ' ' || a[0] == '\n' || a[0] == '\0') {
            return (b[0] == '\0');
        }

        if (b[0] == '\0') {
            return (a[0] == ' ' || a[0] == '\n' || a[0] == '\0');
        }

        if (a[0] == b[0]) {
            a++;
            b++;
        } else {
            return false;
        }
    }
}

static int install_native_packages_via_uppm_or_build(
        const char * depPackageNames,
        const char * xcpkgHomeDIR,
        const size_t xcpkgHomeDIRLength,
        const char * xcpkgDownloadsDIR,
        const size_t xcpkgDownloadsDIRCapacity,
        const char * sessionDIR,
        const size_t sessionDIRLength,
        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRCapacity,
        const char * nativePackageInstalledRootDIR,
        const size_t nativePackageInstalledRootDIRCapacity,
        const XCPKGInstallOptions * installOptions,
        const unsigned int njobs,
        const KV flagsForNativeBuild[]) {

    // these packages are not relocatable, we need to build them from source locally.
    bool needToBuildLibOpenssl = false;
    bool needToBuildLibtool  = false;
    bool needToBuildTexinfo  = false;
    bool needToBuildAutomake = false;
    bool needToBuildAutoconf = false;
    bool needToBuildHelp2man = false;
    bool needToBuildIntltool = false;
    bool needToBuildItstool  = false;
    bool needToBuildPerlXMLParser = false;
    bool needToBuildAutoconfArchive = false;
    bool needToBuildNetsurf  = false;

    bool needToInstallGmake  = false;
    bool needToInstallGm4    = false;
    bool needToInstallPerl   = false;
    bool needToInstallPython3 = false;

    size_t uppmPackageNamesCapacity = 100U;

    size_t i;

    if (depPackageNames == NULL) {
        depPackageNames = "";
    } else {
        for (i = 0U; ; i++) {
            if (depPackageNames[i] == '\0') {
                uppmPackageNamesCapacity += i;
                break;
            }
        }
    }

    char uppmPackageNames[uppmPackageNamesCapacity];

    const char * s = "bash coreutils findutils gsed gawk grep tree pkg-config";

    char * p = uppmPackageNames;

    for (;;) {
        p[0] = s[0];

        if (s[0] == '\0') break;

        p++;
        s++;
    }

    if (installOptions->enableCcache) {
        s = " ccache";

        for (;;) {
            p[0] = s[0];

            if (s[0] == '\0') break;

            p++;
            s++;
        }
    }

    const char * q = depPackageNames;

    while (q[0] != '\0') {
        if (q[0] == ' ' || q[0] == '\n') {
            q++;
            continue;
        }

        for (i = 0U; ; i++) {
            if (q[i] == ' ' || q[i] == '\n' || q[i] == '\0') {
                break;
            }
        }

               if (_str_equal(q, "texinfo")) {
            needToBuildTexinfo = true;
            needToInstallGmake = true;
            needToInstallPerl  = true;
        } else if (_str_equal(q, "libtool")) {
            needToBuildLibtool = true;
            needToInstallGmake = true;
            needToInstallGm4   = true;
        } else if (_str_equal(q, "autoconf")) {
            needToBuildAutoconf = true;
            needToInstallGmake = true;
            needToInstallGm4   = true;
            needToInstallPerl  = true;
        } else if (_str_equal(q, "automake")) {
            needToBuildAutomake = true;
            needToInstallGmake = true;
            needToInstallGm4   = true;
            needToInstallPerl  = true;
        } else if (_str_equal(q, "help2man")) {
            needToBuildHelp2man = true;
            needToInstallGmake  = true;
            needToInstallPerl   = true;
        } else if (_str_equal(q, "intltool")) {
            needToBuildIntltool = true;
            needToInstallGmake = true;
            needToInstallPerl  = true;
        } else if (_str_equal(q, "itstool")) {
            needToBuildItstool = true;
            needToInstallGmake = true;
            needToInstallPython3 = true;
        } else if (_str_equal(q, "perl-XML-Parser")) {
            needToBuildPerlXMLParser = true;
            needToInstallGmake = true;
            needToInstallPerl  = true;
        } else if (_str_equal(q, "libopenssl")) {
            needToBuildLibOpenssl = true;
            needToInstallGmake = true;
            needToInstallPerl  = true;
        } else if (_str_equal(q, "autoconf-archive")) {
            needToBuildAutoconfArchive = true;
            needToInstallGmake = true;
        } else if (_str_equal(q, "netsurf_buildsystem")) {
            needToBuildNetsurf = true;
            needToInstallGmake = true;
        } else {
            p[0] = ' ';

            p++;

            for (size_t j = 0U; j < i; j++) {
                p[j] = q[j];
            }

            p += i;
            p[0] = '\0';
        }

        q += i;

        if (q[0] == '\0') {
            break;
        }

        q++;
    }

    bool bs[4] = {needToInstallGmake, needToInstallGm4, needToInstallPerl, needToInstallPython3};

    for (i = 0U; i < 4U; i++) {
        switch (i) {
            case 0: s = " gmake"  ; break;
            case 1: s = " gm4"    ; break;
            case 2: s = " perl"   ; break;
            case 3: s = " python3"; break;
        }

        if (bs[i]) {
            for (;;) {
                p[0] = s[0];

                if (s[0] == '\0') break;

                p++;
                s++;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int ret = install_native_packages_via_uppm(uppmPackageNames, xcpkgHomeDIR, xcpkgHomeDIRLength, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRCapacity, installOptions->verbose_net);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    char m4Path[PATH_MAX];

    ret = exe_where("m4", m4Path);

    switch (ret) {
        case -3:
            return XCPKG_ERROR_ENV_PATH_NOT_SET;
        case -2:
            return XCPKG_ERROR_ENV_PATH_NOT_SET;
        case -1:
            perror(NULL);
            return XCPKG_ERROR;
    }

    if (ret > 0) {
        if (setenv("M4", m4Path, 1) != 0) {
            perror("M4");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    int    nativePackageIDArray[15] = {0};
    size_t nativePackageIDArraySize = 0U;

    if (needToBuildLibtool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_LIBTOOL;
        nativePackageIDArraySize++;
    }

    if (needToBuildAutoconf) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOCONF;
        nativePackageIDArraySize++;
    }

    if (needToBuildAutomake) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOMAKE;
        nativePackageIDArraySize++;
    }

    if (needToBuildTexinfo) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_TEXINFO;
        nativePackageIDArraySize++;
    }

    if (needToBuildHelp2man) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_HELP2MAN;
        nativePackageIDArraySize++;
    }

    if (needToBuildIntltool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_INTLTOOL;
        nativePackageIDArraySize++;
    }

    if (needToBuildItstool) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_ITSTOOL;
        nativePackageIDArraySize++;
    }

    if (needToBuildPerlXMLParser) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_PERL_XML_PARSER;
        nativePackageIDArraySize++;
    }

    if (needToBuildAutoconfArchive) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_AUTOCONF_ARCHIVE;
        nativePackageIDArraySize++;
    }

    if (needToBuildNetsurf) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_NETSURF_BUILDSYSTEM;
        nativePackageIDArraySize++;
    }

    if (needToBuildLibOpenssl) {
        nativePackageIDArray[nativePackageIDArraySize] = NATIVE_PACKAGE_ID_OPENSSL;
        nativePackageIDArraySize++;
    }

    //////////////////////////////////////////////////////////////////////////////

    char key[20];

    for (i = 0U; i < nativePackageIDArraySize; i++) {
        for (int j = 0U; ; j++) {
            const char * name  = flagsForNativeBuild[j].name;
            const char * value = flagsForNativeBuild[j].value;

            if (name == NULL) {
                break;
            }

            if (value == NULL) {
                if (unsetenv(name) != 0) {
                    perror(name);
                    return XCPKG_ERROR;
                }

                ret = snprintf(key, 20, "%s_FOR_BUILD", name);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                if (unsetenv(key) != 0) {
                    perror(key);
                    return XCPKG_ERROR;
                }
            } else {
                if (setenv(name, value, 1) != 0) {
                    perror(name);
                    return XCPKG_ERROR;
                }

                ret = snprintf(key, 20, "%s_FOR_BUILD", name);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                if (setenv(key, value, 1) != 0) {
                    perror(key);
                    return XCPKG_ERROR;
                }
            }
        }

        ret = install_native_package(nativePackageIDArray[i], xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, sessionDIR, sessionDIRLength + 1, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, njobs, installOptions, native_package_installed_callback);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

static int setenv_rustflags(const char * rustTarget, const size_t rustTargetLength, const char * cc, const char * ldflags, const char * libDIR, const size_t libDIRCapacity) {
    size_t envNameCapacity = rustTargetLength + 25U;
    char   envName[envNameCapacity];

    // https://doc.rust-lang.org/cargo/reference/environment-variables.html
    int ret = snprintf(envName, envNameCapacity, "CARGO_TARGET_%s_RUSTFLAGS", rustTarget);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    const char * const s = " -C link-arg=";

    size_t k;

    for(k = 0U; s[k] != '\0'; k++);

    /////////////////////////////////////////

    size_t i = 1U;
    size_t j = 1U;

    for(i = 0U; ldflags[i] != '\0'; i++) {
        if (ldflags[i] == ' ') j++;
    }

    size_t rustFlagsCapacity = i + j * k + strlen(cc) + libDIRCapacity + 21U;

    char rustFlags[rustFlagsCapacity];

    /////////////////////////////////////////

    // https://doc.rust-lang.org/rustc/command-line-arguments.html
    ret = snprintf(rustFlags, rustFlagsCapacity, "-C linker=%s -L native=%s", cc, libDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    char * p = rustFlags + ret;

loop:
    if (ldflags[0] == '\0') {
        goto finally;
    }

    if (ldflags[0] == ' ') {
        ldflags++;
        goto loop;
    }

    //////////////////////////////////////

    for(i = 0U; i < k; i++) {
        p[i] = s[i];
    }

    p += k;

    //////////////////////////////////////

    for(i = 0U; ; i++) {
        p[i] = ldflags[i];

        if (ldflags[i] == '\0') {
            break;
        }

        if (ldflags[i] == ' ') {
            ldflags += i + 1;
            p[i] = '\0';
            p += i;
            goto loop;
        }
    }

finally:
    if (setenv(envName, rustFlags, 1) == -1) {
        perror(envName);
        return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

static int setup_rust_env2(const bool isForTarget, const char * rustTarget, const char * libDIR, const size_t libDIRCapacity) {
    const char * ar;
    const char * cc;
    const char * cxx;
    const char * cflags;
    const char * cxxflags;
    const char * cppflags;
    const char * ldflags;

    if (isForTarget) {
        ar       = getenv("AR");
        cc       = getenv("CC");
        cxx      = getenv("CXX");
        cflags   = getenv("CFLAGS");
        cxxflags = getenv("CXXFLAGS");
        cppflags = getenv("CPPFLAGS");
        ldflags  = getenv("LDFLAGS");
    } else {
        ar       = getenv("AR_FOR_BUILD");
        cc       = getenv("CC_FOR_BUILD");
        cxx      = getenv("CXX_FOR_BUILD");
        cflags   = getenv("CFLAGS_FOR_BUILD");
        cxxflags = getenv("CXXFLAGS_FOR_BUILD");
        cppflags = getenv("CPPFLAGS_FOR_BUILD");
        ldflags  = getenv("LDFLAGS_FOR_BUILD");
    }

    /////////////////////////////////////////

    char uu[31]; uu[30] = '\0';

    size_t i;

    for (i = 0U; i < 30U; i++) {
        if (rustTarget[i] == '-') {
            uu[i] = '_';
        } else if (rustTarget[i] >= 'a' && rustTarget[i] <= 'z') {
            uu[i] = rustTarget[i] - 32;
        } else {
            uu[i] = rustTarget[i];

            if (uu[i] == '\0') {
                break;
            }
        }
    }

    /////////////////////////////////////////

    size_t envNameLinkerCapacity = i + 21U;
    char   envNameLinker[envNameLinkerCapacity];

    // https://doc.rust-lang.org/cargo/reference/environment-variables.html
    int ret = snprintf(envNameLinker, envNameLinkerCapacity, "CARGO_TARGET_%s_LINKER", uu);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv(envNameLinker, cc, 1) != 0) {
        perror(envNameLinker);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    ret = setenv_rustflags(uu, i, cc, ldflags, libDIR, libDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////

    if (cppflags == NULL) {
        cppflags = "";
    }

    const size_t cppflagsLength = strlen(cppflags);

    /////////////////////////////////////////

    size_t CFLAGSCapacity = strlen(cflags) + cppflagsLength + 2U;
    char   CFLAGS[CFLAGSCapacity];

    ret = snprintf(CFLAGS, CFLAGSCapacity, "%s %s", cflags, cppflags);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    size_t CXXFLAGSCapacity = strlen(cxxflags) + cppflagsLength + 2U;
    char   CXXFLAGS[CXXFLAGSCapacity];

    ret = snprintf(CXXFLAGS, CXXFLAGSCapacity, "%s %s", cxxflags, cppflags);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    KV envs[6] = {
        { "AR",       ar },
        { "CC",       cc },
        { "CXX",      cxx },
        { "CFLAGS",   CFLAGS },
        { "CXXFLAGS", CXXFLAGS },
        { "PKG_CONFIG_PATH", isForTarget ? getenv("PKG_CONFIG_PATH") : getenv("PKG_CONFIG_PATH_FOR_BUILD") }
    };

    char key[100];

    for (size_t i = 0; i < 6; i++) {
        ret = snprintf(key, 100, "%s_%s", envs[i].name, rustTarget);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (envs[i].value == NULL) continue;

        if (setenv(key, envs[i].value, 1) != 0) {
            perror(key);
            return XCPKG_ERROR;
        }
    }

    return XCPKG_OK;
}

static int setup_rust_env(const char * targetPlatformArch, const char * packageWorkingTopDIR, const size_t packageWorkingTopDIRCapacity, const bool isCrossBuild, const size_t njobs) {
    // https://doc.rust-lang.org/cargo/reference/config.html#buildrustflags
    // https://doc.rust-lang.org/cargo/reference/environment-variables.html

    // https://libraries.io/cargo/cc
    // https://crates.io/crates/cc
    // https://docs.rs/cc/latest/cc/
    // https://github.com/alexcrichton/cc-rs

    // https://docs.rs/pkg-config/latest/pkg_config/
    const char * unsetenvs[] = {
        "CARGO_ENCODED_RUSTFLAGS",

        "HOST_CC",
        "HOST_CXX",
        "HOST_AR",
        "HOST_CFLAGS",
        "HOST_CXXFLAGS",

        "TARGET_CC",
        "TARGET_CXX",
        "TARGET_AR",
        "TARGET_CFLAGS",
        "TARGET_CXXFLAGS",

        NULL
    };

    for (int i = 0; unsetenvs[i] != NULL; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return XCPKG_ERROR;
        }
    }

    /////////////////////////////////////////

    // https://docs.rs/backtrace/latest/backtrace/
    if (setenv("RUST_BACKTRACE", "1", 1) != 0) {
        perror("RUST_BACKTRACE");
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    char ns[4];

    int ret = snprintf(ns, 4, "%zu", njobs);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("CARGO_BUILD_JOBS", ns, 1) != 0) {
        perror("CARGO_BUILD_JOBS");
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    const char * rustTarget;

    if (strcmp(targetPlatformArch, "arm64") == 0) {
        rustTarget = "aarch64-apple-darwin";
    } else {
        rustTarget = "x86_64-apple-darwin";
    }

    /////////////////////////////////////////

    // RUST_TARGET environment variable is not defined by Rust, but it is widely used by lots of third-party projects.
    if (setenv("RUST_TARGET", rustTarget, 1) != 0) {
        perror("RUST_TARGET");
        return XCPKG_ERROR;
    }

    /////////////////////////////////////////

    ret = setup_rust_env2(true, rustTarget, packageWorkingTopDIR, packageWorkingTopDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    /////////////////////////////////////////

    if (isCrossBuild) {
#if defined(__x86_64__)
        rustTarget = "x86_64-apple-darwin";
#else
        rustTarget = "aarch64-apple-darwin";
#endif

        ret = setup_rust_env2(false, rustTarget, packageWorkingTopDIR, packageWorkingTopDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

static int copy_dependent_libraries(
        const char * depPackageInstalledDIR,
        const size_t depPackageInstalledDIRLength,
        const char * packageWorkingTopDIR,
        const size_t packageWorkingTopDIRCapacity) {

    size_t fromDIRCapacity = depPackageInstalledDIRLength + 5U;
    char   fromDIR[fromDIRCapacity];

    int ret = snprintf(fromDIR, fromDIRCapacity, "%s/lib", depPackageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(fromDIR, &st) != 0) {
        return XCPKG_OK;
    }

    DIR * dir = opendir(fromDIR);

    if (dir == NULL) {
        perror(fromDIR);
        return XCPKG_ERROR;
    }

    char * fileName;
    size_t fileNameLength;

    struct dirent * dir_entry;

    for (;;) {
        errno = 0;

        dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(fromDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        //puts(dir_entry->d_name);

        fileName = dir_entry->d_name;

        if (strncmp(fileName, "lib", 3) == 0) {
            fileNameLength = strlen(fileName);

            if (fileNameLength > 5 && strcmp(fileName + fileNameLength - 2U, ".a") == 0) {
                size_t fromFilePathCapacity = fromDIRCapacity + fileNameLength + 2U;
                char   fromFilePath[fromFilePathCapacity];

                ret = snprintf(fromFilePath, fromFilePathCapacity, "%s/%s", fromDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return XCPKG_ERROR;
                }

                size_t toFilePathCapacity = packageWorkingTopDIRCapacity + fileNameLength + 5U;
                char   toFilePath[toFilePathCapacity];

                ret = snprintf(toFilePath, toFilePathCapacity, "%s/lib/%s", packageWorkingTopDIR, fileName);

                if (ret < 0) {
                    perror(NULL);
                    closedir(dir);
                    return XCPKG_ERROR;
                }

                ret = xcpkg_copy_file(fromFilePath, toFilePath);

                if (ret != XCPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static inline int config_envs_for_target(const char * recursiveDependentPackageNames, const char * packageInstalledRootDIR, const size_t packageInstalledRootDIRCapacity, const char * nativePackageInstalledRootDIR, const size_t nativePackageInstalledRootDIRCapacity, const char * packageWorkingTopDIR, const size_t packageWorkingTopDIRCapacity, const bool needToCopyStaticLibs, const bool isCrossBuild) {
    size_t packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + 52U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    size_t i;

    char * m;

    for (i = 0U; ; i++) {
        packageInstalledDIR[i] = packageInstalledRootDIR[i];

        if (packageInstalledDIR[i] == '\0') {
            packageInstalledDIR[i] = '/';
            m = packageInstalledDIR + i + 1;
            break;
        }
    }

    //////////////////////////////////////

    size_t nativePkgInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + 52U;
    char   nativePkgInstalledDIR[nativePkgInstalledDIRCapacity];

    char * n;

    for (i = 0U; ; i++) {
        nativePkgInstalledDIR[i] = nativePackageInstalledRootDIR[i];

        if (nativePkgInstalledDIR[i] == '\0') {
            nativePkgInstalledDIR[i] = '/';
            n = nativePkgInstalledDIR + i + 1;
            break;
        }
    }

    //////////////////////////////////////

    const char * p = recursiveDependentPackageNames;

    setenv_fn fns[5] = { setenv_CPPFLAGS, setenv_LDFLAGS, setenv_PKG_CONFIG_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

    int ret;

loop:
    if (p[0] == '\0') {
        return XCPKG_OK;
    }

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    //////////////////////////////////////

    for (i = 0U; ; i++) {
        m[i] = p[i];
        n[i] = p[i];

        if (p[i] == '\0') {
            break;
        }

        if (p[i] == ' ') {
            m[i] = '\0';
            n[i] = '\0';
            break;
        }
    }

    fprintf(stderr, "packageInstalledDIR=%s\n", packageInstalledDIR);
    fprintf(stderr, "nativePkgInstalledDIR=%s\n", nativePkgInstalledDIR);

    //////////////////////////////////////

    for (size_t j = 0U; j < 5U; j++) {
        ret = fns[j](packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (needToCopyStaticLibs) {
        ret = copy_dependent_libraries(packageInstalledDIR, packageInstalledDIRCapacity, packageWorkingTopDIR, packageWorkingTopDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    ret = setenv_PATH(nativePkgInstalledDIR, nativePkgInstalledDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (!isCrossBuild) {
        ret = setenv_PATH(packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////

    p += i;

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    return XCPKG_OK;
}

static inline int write_shell_script_file(const int fd, const char * recursiveDependentPackageNames, const char * packageInstalledRootDIR, const size_t packageInstalledRootDIRCapacity) {
    size_t packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + 52U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    size_t i;

    char * q;

    for (i = 0U; ; i++) {
        packageInstalledDIR[i] = packageInstalledRootDIR[i];

        if (packageInstalledDIR[i] == '\0') {
            packageInstalledDIR[i] = '/';
            q = packageInstalledDIR + i + 1;
            break;
        }
    }

    //////////////////////////////////////

    const char * p = recursiveDependentPackageNames;

    char packageName[51];

loop:
    if (p[0] == '\0') {
        return XCPKG_OK;
    }

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    //////////////////////////////////////

    for (i = 0U; ; i++) {
        q[i] = p[i];

        if (p[i] == '@' || p[i] == '+' || p[i] == '-' || p[i] == '.') {
            packageName[i] = '_';
        } else {
            packageName[i] = p[i];
        }

        if (p[i] == '\0') {
            break;
        }

        if (p[i] == ' ') {
            q[i] = '\0';
            packageName[i] = '\0';
            break;
        }
    }

    //////////////////////////////////////

    int ret = dprintf(fd, "\n%s_INSTALL_DIR='%s'\n", packageName, packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "%s_INCLUDE_DIR='%s/include'\n", packageName, packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "%s_LIBRARY_DIR='%s/lib'\n", packageName, packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////

    p += i;

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    return XCPKG_OK;
}

static int generate_shell_script_file(
        const char * shellScriptFileName,
        const char * packageName,
        const XCPKGFormula * formula,
        const XCPKGInstallOptions * installOptions,
        const SysInfo * sysinfo,
        const char * uppmPackageInstalledRootDIR,
        const char * nativePackageInstalledRootDIR,
        const char * xcpkgExeFilePath,
        const time_t ts,
        const size_t njobs,
        const bool isCrossBuild,
        const char * targetPlatformSpec,
        const char * targetPlatformName,
        const char * targetPlatformVers,
        const char * targetPlatformArch,
        const char * xcpkgHomeDIR,
        const char * xcpkgCoreDIR,
        const char * xcpkgDownloadsDIR,
        const char * sessionDIR,
        const char * packageWorkingTopDIR,
        const char * packageInstalledRootDIR,
        const size_t packageInstalledRootDIRCapacity,
        const char * packageInstalledDIR,

        const char * recursiveDependentPackageNames) {
    int fd = open(shellScriptFileName, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(shellScriptFileName);
        return XCPKG_ERROR;
    }

    int ret;

    if (installOptions->xtrace) {
        ret = dprintf(fd, "set -x\n\n");

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    ret = dprintf(fd, "set -e\n\nunset IFS\n\n");

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs1[] = {
        {"NATIVE_PLATFORM_ARCH", sysinfo->arch },
        {"NATIVE_PLATFORM_VERS", sysinfo->vers },
        {NULL, NULL }
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs1[i].name;
        const char * value = kvs1[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "NATIVE_PLATFORM_NCPU=%u\n", sysinfo->ncpu);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_PLATFORM_EUID=%u\n", geteuid());

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "NATIVE_PLATFORM_EGID=%u\n\n", getegid());

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV targets[] = {
        {"TARGET_PLATFORM_SPEC", targetPlatformSpec },
        {"TARGET_PLATFORM_NAME", targetPlatformName },
        {"TARGET_PLATFORM_VERS", targetPlatformVers },
        {"TARGET_PLATFORM_ARCH", targetPlatformArch },
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = targets[i].name;
        const char * value = targets[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\n");

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs2[] = {
        {"XCPKG_VERSION", XCPKG_VERSION_STRING},
        {"XCPKG_PATH", xcpkgExeFilePath},
        {"XCPKG_HOME", xcpkgHomeDIR},
        {"XCPKG_CORE_DIR", xcpkgCoreDIR},
        {"XCPKG_DOWNLOADS_DIR", xcpkgDownloadsDIR},
        {"XCPKG_PACKAGE_INSTALLED_ROOT", packageInstalledRootDIR},
        {" UPPM_PACKAGE_INSTALLED_ROOT", uppmPackageInstalledRootDIR},
        {"NATIVE_PACKAGE_INSTALLED_ROOT", nativePackageInstalledRootDIR},
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs2[i].name;
        const char * value = kvs2[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\nCROSS_COMPILING=%d\n\n", isCrossBuild);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KB options[] = {
        {"KEEP_SESSION_DIR", installOptions->keepSessionDIR},
        {"BEAR_ENABLED", installOptions->enableBear},
        {"CCACHE_ENABLED", installOptions->enableCcache},
        {"EXPORT_COMPILE_COMMANDS_JSON", installOptions->exportCompileCommandsJson},
        {NULL,false}
    };

    for (int i = 0; ; i++) {
        const char * name  = options[i].name;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s=%d\n", name, options[i].value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "LOG_LEVEL=%d\n", installOptions->logLevel);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "BUILD_NJOBS=%zu\n", njobs);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "PROFILE=%s\n\n", installOptions->buildType == XCPKGBuildProfile_release ? "release" : "debug");

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\nTIMESTAMP_UNIX=%ld\n\n", ts);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KB kbs[] = {
        {"DUMP_ENV",      installOptions->verbose_env},

        {"VERBOSE_MESON", installOptions->verbose_bs},
        {"VERBOSE_NINJA", installOptions->verbose_bs},
        {"VERBOSE_GMAKE", installOptions->verbose_bs},
        {"VERBOSE_CMAKE", installOptions->verbose_bs},
        {"VERBOSE_XMAKE", installOptions->verbose_bs},
        {"VERBOSE_CARGO", installOptions->verbose_bs},
        {"VERBOSE_GO",    installOptions->verbose_bs},

        {"DEBUG_MESON", installOptions->debug_bs},
        {"DEBUG_NINJA", installOptions->debug_bs},
        {"DEBUG_GMAKE", installOptions->debug_bs},
        {"DEBUG_CMAKE", installOptions->debug_bs},
        {"DEBUG_XMAKE", installOptions->debug_bs},
        {"DEBUG_CARGO", installOptions->debug_bs},
        {"DEBUG_GO",    installOptions->debug_bs},

        {"PACKAGE_BINBSTD", formula->binbstd},
        {"PACKAGE_SYMLINK", formula->symlink},
        {"PACKAGE_BUILD_IN_PARALLEL", formula->support_build_in_parallel},
        {"PACKAGE_BUILD_IN_BSCRIPT_DIR", formula->binbstd},
        {"PACKAGE_USE_BSYSTEM_AUTOGENSH", formula->useBuildSystemAutogen},
        {"PACKAGE_USE_BSYSTEM_AUTOTOOLS", formula->useBuildSystemAutotools},
        {"PACKAGE_USE_BSYSTEM_CONFIGURE", formula->useBuildSystemConfigure},
        {"PACKAGE_USE_BSYSTEM_CMAKE", formula->useBuildSystemCmake},
        {"PACKAGE_USE_BSYSTEM_XMAKE", formula->useBuildSystemXmake},
        {"PACKAGE_USE_BSYSTEM_GMAKE", formula->useBuildSystemGmake},
        {"PACKAGE_USE_BSYSTEM_NINJA", formula->useBuildSystemNinja},
        {"PACKAGE_USE_BSYSTEM_MESON", formula->useBuildSystemMeson},
        {"PACKAGE_USE_BSYSTEM_CARGO", formula->useBuildSystemCargo},
        {"PACKAGE_USE_BSYSTEM_GO", formula->useBuildSystemGolang},
        {"PACKAGE_USE_BSYSTEM_GN", formula->useBuildSystemGN},
        {"PACKAGE_USE_BSYSTEM_ZIG", formula->useBuildSystemZIG},
        {NULL,false}
    };

    for (int i = 0; ; i++) {
        const char * name  = kbs[i].name;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s=%d\n", name, kbs[i].value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }
    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "PACKAGE_GIT_NTH=%zu\n", formula->git_nth);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    char srcFileType[XCPKG_FILE_EXTENSION_MAX_CAPACITY]; srcFileType[0] = '\0';

    if (formula->src_url != NULL) {
        ret = xcpkg_extract_filetype_from_url(formula->src_url, srcFileType, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    char fixFileType[XCPKG_FILE_EXTENSION_MAX_CAPACITY]; fixFileType[0] = '\0';

    if (formula->fix_url != NULL) {
        ret = xcpkg_extract_filetype_from_url(formula->fix_url, fixFileType, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    char resFileType[XCPKG_FILE_EXTENSION_MAX_CAPACITY]; resFileType[0] = '\0';

    if (formula->res_url != NULL) {
        ret = xcpkg_extract_filetype_from_url(formula->res_url, resFileType, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs[] = {
        {"PACKAGE_FORMULA_FILEPATH", formula->path},
        {"PACKAGE_NAME", packageName},
        {"PACKAGE_SUMMARY", formula->summary},
        {"PACKAGE_VERSION", formula->version},
        {"PACKAGE_LICENSE", formula->license},
        {"PACKAGE_WEB_URL", formula->web_url},

        {"PACKAGE_GIT_URL", formula->git_url},
        {"PACKAGE_GIT_SHA", formula->git_sha},
        {"PACKAGE_GIT_REF", formula->git_ref},

        {"PACKAGE_SRC_URL", formula->src_url},
        {"PACKAGE_SRC_URI", formula->src_uri},
        {"PACKAGE_SRC_SHA", formula->src_sha},
        {"PACKAGE_SRC_EXT", srcFileType},

        {"PACKAGE_FIX_URL", formula->fix_url},
        {"PACKAGE_FIX_URI", formula->fix_uri},
        {"PACKAGE_FIX_SHA", formula->fix_sha},
        {"PACKAGE_FIX_OPT", formula->fix_opt},
        {"PACKAGE_FIX_EXT", fixFileType},

        {"PACKAGE_RES_URL", formula->res_url},
        {"PACKAGE_RES_URI", formula->res_uri},
        {"PACKAGE_RES_SHA", formula->res_sha},
        {"PACKAGE_RES_EXT", resFileType},

        {"PACKAGE_RESLIST", formula->reslist},
        {"PACKAGE_FIXLIST", formula->patches},

        {"PACKAGE_DEP_LIB", formula->dep_lib},
        {"PACKAGE_DEP_PKG", formula->dep_pkg},
        {"PACKAGE_DEP_PKG_R", recursiveDependentPackageNames},
        {"PACKAGE_DEP_UPP", formula->dep_upp},
        {"PACKAGE_DEP_PYM", formula->dep_pip},
        {"PACKAGE_DEP_PLM", formula->dep_plm},

        {"PACKAGE_BSYSTEM", formula->bsystem},
        {"PACKAGE_BSCRIPT", formula->bscript},

        {"PACKAGE_PPFLAGS", formula->ppflags},
        {"PACKAGE_CCFLAGS", formula->ccflags},
        {"PACKAGE_XXFLAGS", formula->xxflags},
        {"PACKAGE_LDFLAGS", formula->ldflags},

        {"PACKAGE_WORKING_DIR", packageWorkingTopDIR},
        {"PACKAGE_INSTALL_DIR", packageInstalledDIR},

        {NULL, NULL},
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs[i].name;
        const char * value = kvs[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "%s='%s'\n", name, (value == NULL) ? "" : value);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s/src'\n", packageWorkingTopDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/src/_'\n", packageWorkingTopDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    } else {
        ret = dprintf(fd, "PACKAGE_BSCRIPT_DIR='%s/src/%s'\n", packageWorkingTopDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }

        ret = dprintf(fd, "PACKAGE_BCACHED_DIR='%s/src/%s/_'\n", packageWorkingTopDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->bscript == NULL) {
        ret = dprintf(fd, "\n NATIVE_BCACHED_DIR='%s/src/-'\n", packageWorkingTopDIR);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    } else {
        ret = dprintf(fd, "\n NATIVE_BCACHED_DIR='%s/src/%s/-'\n", packageWorkingTopDIR, formula->bscript);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    ret = dprintf(fd, " NATIVE_INSTALL_DIR='%s/%s'\n\n", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    ret = dprintf(fd, "        SESSION_DIR='%s'\n\n", sessionDIR);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    KV kvs5[] = {
        {"DOFETCH", formula->dofetch },
        {"DO12345", formula->do12345 },
        {"DOPATCH", formula->dopatch },
        {"PREPARE", formula->prepare },
        {"DOBUILD", formula->install },
        {"DOTWEAK", formula->dotweak },
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = kvs5[i].name;
        const char * value = kvs5[i].value;

        if (name == NULL) {
            break;
        }

        ret = dprintf(fd, "PACKAGE_%s=%d\n", name, (value == NULL) ? 0 : 1);

        if (ret < 0) {
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    KV actions[] = {
        {"dofetch", formula->dofetch },
        {"do12345", formula->do12345 },
        {"dopatch", formula->dopatch },
        {"prepare", formula->prepare },
        {"dobuild", formula->install },
        {"dotweak", formula->dotweak },
        {"isCrossBuild", isCrossBuild ? "true" : "false" },
        {NULL, NULL}
    };

    for (int i = 0; ; i++) {
        const char * name  = actions[i].name;
        const char * value = actions[i].value;

        if (name == NULL) {
            break;
        }

        if (value != NULL) {
            ret = dprintf(fd, "\n%s() {\n%s\n}\n", name, value);

            if (ret < 0) {
                close(fd);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (recursiveDependentPackageNames != NULL && recursiveDependentPackageNames[0] != '\0') {
        ret = write_shell_script_file(fd, recursiveDependentPackageNames, packageInstalledRootDIR, packageInstalledRootDIRCapacity);

        if (ret != XCPKG_OK) {
            close(fd);
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "\n. %s/xcpkg-install\n", xcpkgCoreDIR);

    if (ret < 0) {
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    close(fd);

    return XCPKG_OK;
}

static int adjust_la_file(const char * filePath, const char * xcpkgHomeDIR, const size_t xcpkgHomeDIRLength) {
    size_t eCapacity = xcpkgHomeDIRLength + 14U;
    char   e[eCapacity];

    int ret = snprintf(e, eCapacity, "s|-L%s[^' ]*||g", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (pid == 0) {
        execlp("sed", "sed", "-i", "-e", "s/-Wl,--strip-debug//g", "-e", "s|-R[^' ]*||g", "-e", e, filePath, NULL);
        perror("sed");
        exit(255);
    } else {
        int childProcessExitStatusCode;

        if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (childProcessExitStatusCode == 0) {
            return XCPKG_OK;
        } else {
            if (WIFEXITED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' exit with status code: %d\n", e, filePath, WEXITSTATUS(childProcessExitStatusCode));
            } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' killed by signal: %d\n", e, filePath, WTERMSIG(childProcessExitStatusCode));
            } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                fprintf(stderr, "running command 'sed -i -e s/-Wl,--strip-debug//g -e s|-R[^' ]*||g -e %s %s' stopped by signal: %d\n", e, filePath, WSTOPSIG(childProcessExitStatusCode));
            }

            return XCPKG_ERROR;
        }
    }
}

static int adjust_la_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity, const char * xcpkgHomeDIR, const size_t xcpkgHomeDIRLength) {
    size_t packageLibDIRLength = packageInstalledDIRCapacity + 5U;
    char   packageLibDIR[packageLibDIRLength];

    int ret = snprintf(packageLibDIR, packageLibDIRLength, "%s/lib", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(packageLibDIR, &st) != 0) {
        return XCPKG_OK;
    }

    DIR * dir = opendir(packageLibDIR);

    if (dir == NULL) {
        perror(packageLibDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(packageLibDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".la") == 0) {
            size_t filePathCapacity = packageLibDIRLength + fileNameLength  + 2U;
            char   filePath[filePathCapacity];

            ret = snprintf(filePath, filePathCapacity, "%s/%s", packageLibDIR, dir_entry->d_name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return XCPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = adjust_la_file(filePath, xcpkgHomeDIR, xcpkgHomeDIRLength);

                if (ret != XCPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_pc_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    size_t packagePkgconfigDIRLength = packageInstalledDIRCapacity + 15U;
    char   packagePkgconfigDIR[packagePkgconfigDIRLength];

    int ret = snprintf(packagePkgconfigDIR, packagePkgconfigDIRLength, "%s/lib/pkgconfig", packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(packagePkgconfigDIR, &st) != 0) {
        return XCPKG_OK;
    }

    DIR * dir = opendir(packagePkgconfigDIR);

    if (dir == NULL) {
        perror(packagePkgconfigDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(packagePkgconfigDIR);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t fileNameLength = strlen(dir_entry->d_name);

        if (fileNameLength < 4U) {
            continue;
        }

        char * fileNameSuffix = dir_entry->d_name + fileNameLength - 3U;

        if (strcmp(fileNameSuffix, ".pc") == 0) {
            size_t filePathCapacity = packagePkgconfigDIRLength + fileNameLength  + 2U;
            char   filePath[filePathCapacity];

            ret = snprintf(filePath, filePathCapacity, "%s/%s", packagePkgconfigDIR, dir_entry->d_name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (stat(filePath, &st) != 0) {
                closedir(dir);
                return XCPKG_ERROR;
            }

            if (S_ISREG(st.st_mode)) {
                int ret = XCPKG_OK;

                if (ret != XCPKG_OK) {
                    closedir(dir);
                    return ret;
                }
            }
        }
    }
}

static int adjust_macho_files(const char * packageInstalledDIR, const size_t packageInstalledDIRCapacity) {
    return XCPKG_OK;
}

static int backup_formulas(const char * sessionDIR, const size_t sessionDIRLength, const char * recursiveDependentPackageNames) {
    size_t fromFilePathCapacity = sessionDIRLength + 60U;
    char   fromFilePath[fromFilePathCapacity];

    char   toFilePath[70];

    ///////////////////////////////////////

    char * m;

    for (size_t i = 0U; ; i++) {
        if (sessionDIR[i] == '\0') {
            fromFilePath[i] = '/';
            m = fromFilePath + i + 1;
            break;
        }

        fromFilePath[i] = sessionDIR[i];
    }

    ///////////////////////////////////////

    const char * s = "dependencies";
    char * n;

    for (size_t i = 0U; ; i++) {
        if (s[i] == '\0') {
            toFilePath[i] = '/';
            n = toFilePath + i + 1;
            break;
        }

        toFilePath[i] = s[i];
    }

    ///////////////////////////////////////

    if (mkdir(toFilePath, S_IRWXU) != 0) {
        if (errno != EEXIST) {
            perror(toFilePath);
            return XCPKG_ERROR;
        }
    }

    ///////////////////////////////////////

    const char * p = recursiveDependentPackageNames;

loop:
    if (p[0] == '\0') {
        return XCPKG_OK;
    }

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    /////////////////////////////

    char * x = m;
    char * y = n;

    for (;;) {
        x[0] = p[0];
        y[0] = p[0];

        if (p[0] == ' ' || p[0] == '\0') break;

        x++;
        y++;
        p++;
    }

    s = ".yml";

    for (;;) {
        x[0] = s[0];
        y[0] = s[0];

        if (s[0] == '\0') break;

        x++;
        y++;
        s++;
    }

    fprintf(stderr, "copy %s => %s\n", fromFilePath, toFilePath);

    int ret = xcpkg_copy_file(fromFilePath, toFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

    return XCPKG_OK;
}

static int generate_manifest_r(const char * dirPath, const size_t offset, FILE * installedManifestFile) {
    if (dirPath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (dirPath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return XCPKG_ERROR;
    }

    size_t dirPathLength = strlen(dirPath);

    int ret = XCPKG_OK;

    struct stat st;

    for (;;) {
        errno = 0;

        struct dirent * dir_entry = readdir(dir);

        if (dir_entry == NULL) {
            if (errno == 0) {
                closedir(dir);
                return XCPKG_OK;
            } else {
                perror(dirPath);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }

        if ((strcmp(dir_entry->d_name, ".") == 0) || (strcmp(dir_entry->d_name, "..") == 0)) {
            continue;
        }

        size_t filePathCapacity = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char   filePath[filePathCapacity];

        ret = snprintf(filePath, filePathCapacity, "%s/%s", dirPath, dir_entry->d_name);

        if (ret < 0) {
            perror(NULL);
            closedir(dir);
            return XCPKG_ERROR;
        }

        if (stat(filePath, &st) != 0) {
            perror(filePath);
            closedir(dir);
            return XCPKG_ERROR;
        }

        if (S_ISDIR(st.st_mode)) {
            ret = fprintf(installedManifestFile, "d|%s/\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return XCPKG_ERROR;
            }

            ret = generate_manifest_r(filePath, offset, installedManifestFile);

            if (ret != XCPKG_OK) {
                closedir(dir);
                return ret;
            }
        } else {
            ret = fprintf(installedManifestFile, "f|%s\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }
    }
}

int generate_manifest(const char * installedDIRPath) {
    size_t installedDIRLength = strlen(installedDIRPath);

    size_t installedManifestFilePathLength = installedDIRLength + sizeof(XCPKG_MANIFEST_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);
    char   installedManifestFilePath[installedManifestFilePathLength];

    int ret = snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s%s", installedDIRPath, XCPKG_MANIFEST_FILEPATH_RELATIVE_TO_INSTALLED_ROOT);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    FILE * installedManifestFile = fopen(installedManifestFilePath, "w");

    if (installedManifestFile == NULL) {
        perror(installedManifestFilePath);
        return XCPKG_ERROR;
    }

    ret = generate_manifest_r(installedDIRPath, installedDIRLength + 1, installedManifestFile);

    fclose(installedManifestFile);

    return ret;
}

static int generate_receipt(const char * packageName, const XCPKGFormula * formula, const char * targetPlatformSpec, const SysInfo * sysinfo, const time_t ts) {
    const char * const f = "RECEIPT.yml";

    FILE * receiptFile = fopen(f, "w");

    if (receiptFile == NULL) {
        perror(f);
        return XCPKG_ERROR;
    }

    FILE * formulaFile = fopen(formula->path, "r");

    if (formulaFile == NULL) {
        perror(formula->path);
        fclose(receiptFile);
        return XCPKG_ERROR;
    }

    fprintf(receiptFile, "pkgname: %s\n", packageName);

    if (formula->pkgtype_is_calculated) {
        fprintf(receiptFile, "pkgtype: %s\n", formula->pkgtype == XCPKGPkgType_exe ? "exe" : "lib");
    }

    if (formula->version_is_calculated) {
        fprintf(receiptFile, "version: %s\n", formula->version);
    }

    if (formula->bsystem_is_calculated) {
        fprintf(receiptFile, "bsystem: %s\n", formula->bsystem);
    }

    if (formula->web_url_is_calculated) {
        fprintf(receiptFile, "web-url: %s\n", formula->web_url);
    }

    if (formula->git_url_is_calculated) {
        fprintf(receiptFile, "git-url: %s\n", formula->git_url);
    }

    if (formula->binbstd_is_calculated) {
        fprintf(receiptFile, "binbstd: %d\n", formula->binbstd);
    }

    if (formula->ltoable_is_calculated) {
        fprintf(receiptFile, "ltoable: %d\n", formula->ltoable);
    }

    if (formula->mslable_is_calculated) {
        fprintf(receiptFile, "mslable: %d\n", formula->support_create_mostly_statically_linked_executable);
    }

    if (formula->movable_is_calculated) {
        fprintf(receiptFile, "movable: %d\n", formula->movable);
    }

    if (formula->symlink_is_calculated) {
        fprintf(receiptFile, "symlink: %d\n", formula->symlink);
    }

    if (formula->parallel_is_calculated) {
        fprintf(receiptFile, "parallel: %d\n", formula->support_build_in_parallel);
    }

    char   buff[2048];
    size_t size = 0U;

    for (;;) {
        size = fread(buff, 1, 2048, formulaFile);

        if (ferror(formulaFile)) {
            perror(formula->path);
            fclose(formulaFile);
            fclose(receiptFile);
            return XCPKG_ERROR;
        }

        if (size > 0) {
            if (fwrite(buff, 1, size, receiptFile) != size || ferror(receiptFile)) {
                perror(f);
                fclose(receiptFile);
                fclose(formulaFile);
                return XCPKG_ERROR;
            }
        }

        if (feof(formulaFile)) {
            fclose(formulaFile);
            break;
        }
    }

    fprintf(receiptFile, "\nbuiltfor: %s\nbuiltby: %s\nbuiltat: %ld\n\n", targetPlatformSpec, XCPKG_VERSION_STRING, ts);

    fprintf(receiptFile, "build-on:\n    os-arch: %s\n    os-vers: %s\n    os-ncpu: %u\n    os-euid: %u\n    os-egid: %u\n", sysinfo->arch, sysinfo->vers, sysinfo->ncpu, sysinfo->euid, sysinfo->egid);

    fclose(receiptFile);

    return XCPKG_OK;
}

static int xcpkg_build_for_native(
        const XCPKGFormula * formula,
        const char * packageName,
        const size_t packageNameLength,
        const char * packageWorkingTopDIR,
        const size_t packageWorkingTopDIRCapacity,
        const char * nativePackageInstalledRootDIR,
        const size_t nativePackageInstalledRootDIRCapacity,
        const char * packageInstalledSHA,
        const char * shellScriptFileName,
        const bool verbose) {
    size_t receiptFilePathCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 14U;
    char   receiptFilePath[receiptFilePathCapacity];

    int ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/%s/receipt.txt", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(receiptFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char buf[65] = {0};

            ret = xcpkg_read_the_first_n_bytes_of_a_file(receiptFilePath, 64, buf);

            if (ret != XCPKG_OK) {
                return ret;
            }

            if (strcmp(buf, formula->src_sha) == 0) {
                fprintf(stderr, "native package '%s' already has been installed.\n", packageName);

                size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
                char   packageInstalledDIR[packageInstalledDIRCapacity];

                ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

                if (ret < 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }

                setenv_fn funs[3] = { setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

                for (int i = 0; i < 3; i++) {
                    ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

                    if (ret != XCPKG_OK) {
                        return ret;
                    }
                }

                return XCPKG_OK;
            }
        } else {
            fprintf(stderr, "%s was expected to be a regular file, but it was not.\n", receiptFilePath);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t workingDIRCapacity = packageWorkingTopDIRCapacity + packageNameLength + 14U;
    char   workingDIR[workingDIRCapacity];

    ret = snprintf(workingDIR, workingDIRCapacity, "%s/native-build-%s", packageWorkingTopDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir (workingDIR, S_IRWXU) != 0) {
        perror(workingDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstallDIRCapacity = nativePackageInstalledRootDIRCapacity + 66U;
    char   packageInstallDIR[packageInstallDIRCapacity];

    ret = snprintf(packageInstallDIR, packageInstallDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_fork_exec2(3, "/bin/sh", shellScriptFileName, "native");

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (stat(packageInstallDIR, &st) != 0) {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t receiptFilePath2Capacity = packageInstallDIRCapacity + 12U;
    char   receiptFilePath2[receiptFilePath2Capacity];

    ret = snprintf(receiptFilePath2, receiptFilePath2Capacity, "%s/receipt.txt", packageInstallDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_write_file(receiptFilePath2, formula->src_sha, 0);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (nativePackageInstalledRootDIR) != 0) {
        perror(nativePackageInstalledRootDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "native package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = xcpkg_rm_rf(packageName, false, verbose);

                        if (ret != XCPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return XCPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return XCPKG_ERROR;
            }
        }
    }

    size_t packageInstalledDIRCapacity = nativePackageInstalledRootDIRCapacity + packageNameLength + 2U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", nativePackageInstalledRootDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    setenv_fn funs[3] = { setenv_PATH, setenv_ACLOCAL_PATH, setenv_XDG_DATA_DIRS };

    for (int i = 0; i < 3; i++) {
        ret = funs[i](packageInstalledDIR, packageInstalledDIRCapacity);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}

typedef struct {
    char * packageName;
    XCPKGFormula * formula;
} XCPKGPackage;

static inline __attribute__((always_inline)) bool is_a_in_b(const char * a, const char * b) {
loop:
    if (b[0] == '\0') return false;

    if (b[0] == ' ') {
        b++;
        goto loop;
    }

    for (;;) {
        if (a[0] == '\0') {
            return (b[0] == ' ' || b[0] == '\0');
        }

        if (b[0] == '\0') {
            return (a[0] == '\0');
        }

        if (a[0] == b[0]) {
            a++;
            b++;
        } else {
            if (b[0] == ' ') {
                b++;
                goto loop;
            } else {
                for (;;) {
                    b++;

                    if (b[0] == '\0') {
                        return false;
                    }

                    if (b[0] == ' ') {
                        b++;
                        goto loop;
                    }
                }
            }
        }
    }
}

typedef struct {
    char * ptr;
    size_t length;
    size_t capacity;
} StringBuf;

static inline __attribute__((always_inline)) int string_buffer_append(StringBuf * const stringBuf, const char * s) {
    char * p;

    size_t m = stringBuf->length;
    size_t n = strlen(s);

    size_t oldCapacity = stringBuf->capacity;
    size_t newCapacity = m + n + 1U;

    if (newCapacity > oldCapacity) {
        newCapacity += 1024U;

        p = (char*)realloc(stringBuf->ptr, newCapacity * sizeof(char));

        if (p == NULL) {
            return XCPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            stringBuf->ptr = p;
            stringBuf->capacity = newCapacity;
        }
    }

    p = stringBuf->ptr + m;

    for (size_t i = 0U; i <= n; i++) {
        p[i] = s[i];
    }

    stringBuf->length = m + n;

    return XCPKG_OK;
}

static int generate_dependencies_graph(const char * packageName, XCPKGPackage ** packageSet, size_t packageSetSize, StringBuf * txt, StringBuf * dot, StringBuf * d2) {
    int ret = XCPKG_OK;

    char * p;
    size_t i;

    size_t  packageNameStackCapacity = 1U;
    size_t  packageNameStackSize     = 1U;
    char ** packageNameStack = (char**)malloc(sizeof(char*));

    if (packageNameStack == NULL) {
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = (char*)packageName;

    while (packageNameStackSize != 0U) {
        char * packageName = packageNameStack[--packageNameStackSize];
        packageNameStack[packageNameStackSize] = NULL;

        ////////////////////////////////////////////////////////////////

        if (txt->ptr != NULL) {
            if (is_a_in_b(packageName, txt->ptr)) {
                continue;
            }

            ret = string_buffer_append(txt, " ");

            if (ret != XCPKG_OK) {
                goto finalize;
            }
        }

        ret = string_buffer_append(txt, packageName);

        if (ret != XCPKG_OK) {
            goto finalize;
        }

        ////////////////////////////////////////////////////////////////

        p = NULL;

        for (i = 0U; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                p = packageSet[i]->formula->dep_pkg;
                break;
            }
        }

        ////////////////////////////////////////////////////////////////

        if (p == NULL || p[0] == '\0') continue;

        for(;;) {
            if (p[0] == '\0') break;

            if (p[0] == '\n' || p[0] == ' ') {
                p++;
                continue;
            } else {
                break;
            }
        }

        if (p[0] == '\0') continue;

        ////////////////////////////////////////////////////////////////

        if (dot->ptr == NULL) {
            ret = string_buffer_append(dot, "digraph G {\n");

            if (ret != XCPKG_OK) {
                goto finalize;
            }
        }

        ret = string_buffer_append(dot, "    \"");

        if (ret != XCPKG_OK) {
            goto finalize;
        }

        ret = string_buffer_append(dot, packageName);

        if (ret != XCPKG_OK) {
            goto finalize;
        }

        ret = string_buffer_append(dot, "\" -> {");

        if (ret != XCPKG_OK) {
            goto finalize;
        }

        ////////////////////////////////////////////////////////////////

        while (p[0] != '\0') {
            if (p[0] == ' ' || p[0] == '\n') {
                p++;
                continue;
            }

            for (i = 0U; ; i++) {
                if (p[i] == ' ' || p[i] == '\n' || p[i] == '\0') {
                    break;
                }
            }

            //////////////////////////////////////

            char * depPackageName = NULL;

            for (size_t j = 0U; j < packageSetSize; j++) {
                if (_str_equal(p, packageSet[j]->packageName)) {
                    depPackageName = packageSet[j]->packageName;
                    break;
                }
            }

            //////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** q = (char**)realloc(packageNameStack, (packageNameStackCapacity + 8U) * sizeof(char*));

                if (q == NULL) {
                    free(packageNameStack);
                    return XCPKG_ERROR_MEMORY_ALLOCATE;
                }

                packageNameStack = q;
                packageNameStackCapacity += 8U;
            }

            packageNameStack[packageNameStackSize] = depPackageName;
            packageNameStackSize++;

            //////////////////////////////////////

            ret = string_buffer_append(dot, " \"");

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            ret = string_buffer_append(dot, depPackageName);

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            ret = string_buffer_append(dot, "\"");

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            //////////////////////////////////////

            ret = string_buffer_append(d2, "\"");

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            ret = string_buffer_append(d2, packageName);

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            ret = string_buffer_append(d2, "\" -> \"");

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            ret = string_buffer_append(d2, depPackageName);

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            ret = string_buffer_append(d2, "\"\n");

            if (ret != XCPKG_OK) {
                goto finalize;
            }

            //////////////////////////////////////

            p += i;
        }

        ret = string_buffer_append(dot, " }\n");

        if (ret != XCPKG_OK) {
            goto finalize;
        }
    }

    if (dot->ptr != NULL) {
        ret = string_buffer_append(dot, "}\n");

        if (ret != XCPKG_OK) {
            goto finalize;
        }
    }

finalize:
    free(packageNameStack);
    return ret;
}

static int xcpkg_install_package(
        const char * packageName,
        const char * targetPlatformSpec,

        const XCPKGFormula * formula,
        const XCPKGInstallOptions * installOptions,
        const XCPKGToolChain * toolchain,
        const SysInfo * sysinfo,

        const char * ccForNativeBuild,
        const char * cxxForNativeBuild,
        const char * cppForNativeBuild,
        const char * objcForNativeBuild,

        const char * ccForTargetBuild,
        const char * cxxForTargetBuild,
        const char * cppForTargetBuild,
        const char * objcForTargetBuild,

        const char *  ccFlagsForNativeBuild,
        const char * cxxFlagsForNativeBuild,
        const char * cppFlagsForNativeBuild,
        const char *  ldFlagsForNativeBuild,

        const char * extraCCFlagsForTargetBuild,
        const char * extraLDFlagsForTargetBuild,

        const char * uppmPackageInstalledRootDIR,
        const size_t uppmPackageInstalledRootDIRCapacity,
        const char * xcpkgExeFilePath,
        const char * xcpkgHomeDIR,
        const size_t xcpkgHomeDIRLength,
        const char * xcpkgCoreDIR,
        const size_t xcpkgCoreDIRCapacity,
        const char * xcpkgDownloadsDIR,
        const size_t xcpkgDownloadsDIRCapacity,
        const char * sessionDIR,
        const size_t sessionDIRLength,
        const StringBuf * txt,
        const StringBuf * dot,
        const StringBuf * d2) {
    fprintf(stderr, "%s=============== Installing%s %s%s/%s%s %s===============%s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, targetPlatformSpec, packageName, COLOR_OFF, COLOR_PURPLE, COLOR_OFF);

    const time_t ts = time(NULL);

    const size_t packageNameLength = strlen(packageName);

    const size_t targetPlatformSpecLength = strlen(targetPlatformSpec);

    char buf[targetPlatformSpecLength + 1];

    char * targetPlatformName = buf;
    char * targetPlatformVers = NULL;
    char * targetPlatformArch = NULL;

    for (size_t i = 0; i < targetPlatformSpecLength; i++) {
        buf[i] = targetPlatformSpec[i];

        if (buf[i] == '-') {
            buf[i] = '\0';

            if (targetPlatformVers == NULL) {
                targetPlatformVers = buf + i + 1;
            } else if (targetPlatformArch == NULL) {
                targetPlatformArch = buf + i + 1;
            }
        }
    }

    buf[targetPlatformSpecLength] = '\0';

    //printf("targetPlatformName=%s\n", targetPlatformName);
    //printf("targetPlatformVers=%s\n", targetPlatformVers);
    //printf("targetPlatformArch=%s\n", targetPlatformArch);

    //////////////////////////////////////////////////////////////////////////////

    size_t njobs;

    if (formula->support_build_in_parallel) {
        if (installOptions->parallelJobsCount > 0) {
            njobs = installOptions->parallelJobsCount;
        } else {
            njobs = sysinfo->ncpu;
        }
    } else {
        njobs = 1U;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t strBufSize = packageNameLength + 50U;
    char   strBuf[strBufSize];

    int ret = snprintf(strBuf, strBufSize, "%s:%ld:%d", packageName, ts, getpid());

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char packageInstalledSHA[65] = {0};

    ret = sha256sum_of_string(packageInstalledSHA, strBuf);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageWorkingTopDIRCapacity = sessionDIRLength + targetPlatformSpecLength + packageNameLength + 4U;
    char   packageWorkingTopDIR[packageWorkingTopDIRCapacity];

    ret = snprintf(packageWorkingTopDIR, packageWorkingTopDIRCapacity, "%s/%s-%s", sessionDIR, targetPlatformSpec, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir (packageWorkingTopDIR, S_IRWXU) != 0) {
        perror(packageWorkingTopDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    const KV toolsForNativeBuild[] = {
        { "CC",        ccForNativeBuild },
        { "OBJC",      objcForNativeBuild },
        { "CXX",       cxxForNativeBuild },
        { "CPP",       cppForNativeBuild },

        { "AS",        toolchain->as },
        { "AR",        toolchain->ar },
        { "RANLIB",    toolchain->ranlib },
        { "LD",        toolchain->ld },
        { "NM",        toolchain->nm },
        { "SIZE",      toolchain->size },
        { "STRIP",     toolchain->strip },
        { "STRINGS",   toolchain->strings },
        { "OBJDUMP",   toolchain->objdump },

        { NULL, NULL }
    };

    char key[20];

    for (int i = 0; ; i++) {
        const char * name  = toolsForNativeBuild[i].name;
        const char * value = toolsForNativeBuild[i].value;

        if (name == NULL) {
            break;
        }

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (unsetenv(key) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv(key, value, 1) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    const KV flagsForNativeBuild[] = {
        { "CFLAGS",    ccFlagsForNativeBuild },
        { "CXXFLAGS", cxxFlagsForNativeBuild },
        { "CPPFLAGS", cppFlagsForNativeBuild },
        { "LDFLAGS",   ldFlagsForNativeBuild },
        { NULL, NULL }
    };

    for (size_t i = 0; ; i++) {
        const char * name  = flagsForNativeBuild[i].name;
        const char * value = flagsForNativeBuild[i].value;

        if (name == NULL) {
            break;
        }

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (unsetenv(key) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }

            ret = snprintf(key, 20, "%s_FOR_BUILD", name);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (setenv(key, value, 1) != 0) {
                perror(key);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // https://keith.github.io/xcode-man-pages/xcrun.1.html

    // https://clang.llvm.org/docs/CommandGuide/clang.html#envvar-MACOSX_DEPLOYMENT_TARGET
    // https://clang.llvm.org/docs/CommandGuide/clang.html#cmdoption-mmacosx-version-min
    // If -mmacosx-version-min is unspecified, the default deployment target is read from MACOSX_DEPLOYMENT_TARGET environment variable

    const char* unsetenvs[] = {
        "SDKROOT",
        "MACOSX_DEPLOYMENT_TARGET",
        "PKG_CONFIG_LIBDIR",
        "PKG_CONFIG_PATH",
        "ACLOCAL_PATH",
        "XDG_DATA_DIRS",
        "PERL_EXT_CC",
        "PERL_EXT_CFLAGS",
        "PERL_EXT_CPPFLAGS",
        "PERL_EXT_LDFLAGS",
        "PERL5LIB",
        "LIBS",
        "LDDLFLAGS",
        NULL
    };

    for (int i = 0; unsetenvs[i] != NULL; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = setenv_ACLOCAL_PATH(xcpkgCoreDIR, xcpkgCoreDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ret = setenv_XDG_DATA_DIRS(xcpkgCoreDIR, xcpkgCoreDIRCapacity);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t nativePackageInstalledRootDIRCapacity = xcpkgHomeDIRLength + 8U;
    char   nativePackageInstalledRootDIR[nativePackageInstalledRootDIRCapacity];

    ret = snprintf(nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, "%s/native", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = install_native_packages_via_uppm_or_build(formula->dep_upp, xcpkgHomeDIR, xcpkgHomeDIRLength, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, sessionDIR, sessionDIRLength, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, installOptions, njobs, flagsForNativeBuild);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_pip != NULL) {
        const char * const s = "/python3/bin/python3 -m pip install --upgrade ";

        size_t sLength = strlen(s);

        char* a[2] = { "pip", formula->dep_pip };

        for (int i = 0; i < 2; i++) {
            size_t pipInstallCmdCapacity = uppmPackageInstalledRootDIRCapacity + sLength + strlen(a[i]);
            char   pipInstallCmd[pipInstallCmdCapacity];

            ret = snprintf(pipInstallCmd, pipInstallCmdCapacity, "%s%s%s", uppmPackageInstalledRootDIR, s, a[i]);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_fork_exec(pipInstallCmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dep_plm != NULL) {
        size_t cpanInstallCmdCapacity = uppmPackageInstalledRootDIRCapacity + strlen(formula->dep_plm) + 24U;
        char   cpanInstallCmd[cpanInstallCmdCapacity];

        ret = snprintf(cpanInstallCmd, cpanInstallCmdCapacity, "%s/perl/bin/cpan %s", uppmPackageInstalledRootDIR, formula->dep_plm);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_fork_exec(cpanInstallCmd);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        ret = setup_rust_toolchain(installOptions, sessionDIR, sessionDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (packageWorkingTopDIR) != 0) {
        perror(packageWorkingTopDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* dirs[7] = {"src", "fix", "res", "bin", "lib", "include", "tmp"};

    for (size_t i = 0U; i < 7U; i++) {
        if (mkdir(dirs[i], S_IRWXU) != 0) {
            perror(dirs[i]);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRootDIRCapacity = xcpkgHomeDIRLength + targetPlatformSpecLength + 15U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed/%s", xcpkgHomeDIR, targetPlatformSpec);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t packageInstalledDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char   packageInstalledDIR[packageInstalledDIRCapacity];

    ret = snprintf(packageInstalledDIR, packageInstalledDIRCapacity, "%s/%s", packageInstalledRootDIR, packageInstalledSHA);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (packageWorkingTopDIR) != 0) {
        perror(packageWorkingTopDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * const shellScriptFileName = "vars.sh";

    const bool isCrossBuild = !((strcmp("MacOSX", targetPlatformName) == 0) && (strcmp(sysinfo->arch, targetPlatformArch) == 0));

    ret = generate_shell_script_file(shellScriptFileName, packageName, formula, installOptions, sysinfo, uppmPackageInstalledRootDIR, nativePackageInstalledRootDIR, xcpkgExeFilePath, ts, njobs, isCrossBuild, targetPlatformSpec, targetPlatformName, targetPlatformVers, targetPlatformArch, xcpkgHomeDIR, xcpkgCoreDIR, xcpkgDownloadsDIR, sessionDIR, packageWorkingTopDIR, packageInstalledRootDIR, packageInstalledRootDIRCapacity, packageInstalledDIR, txt->ptr);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->dofetch != NULL) {
        ret = xcpkg_fork_exec2(3, "/bin/sh", shellScriptFileName, "dofetch");

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        if (formula->git_url != NULL) {
            const char * remoteRef;

            if (formula->git_sha == NULL) {
                remoteRef = (formula->git_ref == NULL) ? "HEAD" : formula->git_ref;
            } else {
                remoteRef = formula->git_sha;
            }

            ret = xcpkg_git_sync("src", formula->git_url, remoteRef, "refs/remotes/origin/master", "master", formula->git_nth);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    } else {
        if (formula->src_is_dir) {
            char * srcDIR = &formula->src_url[6];
            size_t srcDIRLength = strlen(srcDIR);

            size_t cmdCapacity = srcDIRLength + 14U;
            char   cmd[cmdCapacity];

            ret = snprintf(cmd, cmdCapacity, "cp -r %s/. src/", srcDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = xcpkg_fork_exec(cmd);

            if (ret != XCPKG_OK) {
                return ret;
            }
        } else {
            ret = xcpkg_http_fetch_then_unpack(formula->src_url, formula->src_uri, formula->src_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "src", 4U, installOptions->verbose_net);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    if (formula->fix_url != NULL) {
        ret = xcpkg_http_fetch_then_unpack(formula->fix_url, formula->fix_uri, formula->fix_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "fix", 4U, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (formula->res_url != NULL) {
        ret = xcpkg_http_fetch_then_unpack(formula->res_url, formula->res_uri, formula->res_sha, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "res", 4U, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (formula->patches != NULL) {
        ret = fetch_fixlist(formula->patches, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (formula->reslist != NULL) {
        ret = fetch_reslist(formula->reslist, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (formula->useBuildSystemAutogen || formula->useBuildSystemAutotools || formula->useBuildSystemConfigure) {
        ret = fetch_gnu_config(sessionDIR, sessionDIRLength + 1, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->do12345 != NULL) {
        ret = xcpkg_build_for_native(formula, packageName, packageNameLength, packageWorkingTopDIR, packageWorkingTopDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, packageInstalledSHA, shellScriptFileName, installOptions->verbose_net);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    ///                        below is for target                             ///
    //////////////////////////////////////////////////////////////////////////////

    const KV toolsForTargetBuild[] = {
        { "CC",        ccForTargetBuild },
        { "OBJC",      objcForTargetBuild },
        { "CXX",       cxxForTargetBuild },
        { "CPP",       cppForTargetBuild },

        { "AS",        toolchain->as },
        { "AR",        toolchain->ar },
        { "RANLIB",    toolchain->ranlib },
        { "LD",        toolchain->ld },
        { "NM",        toolchain->nm },
        { "SIZE",      toolchain->size },
        { "STRIP",     toolchain->strip },
        { "STRINGS",   toolchain->strings },
        { "OBJDUMP",   toolchain->objdump },

        { NULL, NULL }
    };

    for (int i = 0; ; i++) {
        const char * name  = toolsForTargetBuild[i].name;
        const char * value = toolsForTargetBuild[i].value;

        if (name == NULL) {
            break;
        }

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char sdkName[20];

    for (int i = 0; ; i++) {
        if (targetPlatformName[i] >= 'A' && targetPlatformName[i] <= 'Z') {
            sdkName[i] = targetPlatformName[i] + 32;
        } else {
            sdkName[i] = targetPlatformName[i];
        }

        if (targetPlatformName[i] == '\0') {
            break;
        }
    }

    char sysrootForTargetBuild[PATH_MAX]; sysrootForTargetBuild[0] = '\0';

    ret = xcpkg_sdk_path(sdkName, sysrootForTargetBuild);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (sysrootForTargetBuild[0] == '\0') {
        fprintf(stderr, "Can not locate %s sdk path.\n", targetPlatformName);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t commonflagsCapacity = strlen(sysrootForTargetBuild) + 80U;
    char   commonflags[commonflagsCapacity];

    ret = snprintf(commonflags, commonflagsCapacity, "-isysroot %s -m%s-version-min=%s -arch %s -Qunused-arguments", sysrootForTargetBuild, sdkName, targetPlatformVers, targetPlatformArch);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("XCPKG_TARGET_FLAGS", commonflags, 1) != 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ccflags == NULL) {
        if (setenv("CFLAGS", extraCCFlagsForTargetBuild, 1) != 0) {
            perror("CFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t ccflagsCapacity = strlen(extraCCFlagsForTargetBuild) + strlen(formula->ccflags) + 2U;
        char   ccflags[ccflagsCapacity];

        ret = snprintf(ccflags, ccflagsCapacity, "%s %s", extraCCFlagsForTargetBuild, formula->ccflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CFLAGS", ccflags, 1) != 0) {
            perror("CFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->xxflags == NULL) {
        if (setenv("CXXFLAGS", extraCCFlagsForTargetBuild, 1) != 0) {
            perror("CXXFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t cxxflagsCapacity = strlen(extraCCFlagsForTargetBuild) + strlen(formula->xxflags) + 2U;
        char   cxxflags[cxxflagsCapacity];

        ret = snprintf(cxxflags, cxxflagsCapacity, "%s %s", extraCCFlagsForTargetBuild, formula->xxflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CXXFLAGS", cxxflags, 1) != 0) {
            perror("CXXFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ppflags == NULL) {
        size_t cppflagsCapacity = packageWorkingTopDIRCapacity + 10U;
        char   cppflags[cppflagsCapacity];

        ret = snprintf(cppflags, cppflagsCapacity, "-I%s/include", packageWorkingTopDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CPPFLAGS", cppflags, 1) != 0) {
            perror("CPPFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t cppflagsCapacity = packageWorkingTopDIRCapacity + strlen(formula->ppflags) + 12U;
        char   cppflags[cppflagsCapacity];

        ret = snprintf(cppflags, cppflagsCapacity, "-I%s/include %s", packageWorkingTopDIR, formula->ppflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("CPPFLAGS", cppflags, 1) != 0) {
            perror("CPPFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->ldflags == NULL) {
        size_t ldflagsCapacity = strlen(extraLDFlagsForTargetBuild) + packageWorkingTopDIRCapacity + packageInstalledRootDIRCapacity + packageNameLength + 50U;
        char   ldflags[ldflagsCapacity];

        ret = snprintf(ldflags, ldflagsCapacity, "%s -L%s/lib -Wl,-rpath,%s/%s/lib", extraLDFlagsForTargetBuild, packageWorkingTopDIR, packageInstalledRootDIR, packageName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("LDFLAGS", ldflags, 1) != 0) {
            perror("LDFLAGS");
            return XCPKG_ERROR;
        }
    } else {
        size_t ldflagsCapacity = strlen(extraLDFlagsForTargetBuild) + packageWorkingTopDIRCapacity + packageInstalledRootDIRCapacity + packageNameLength + strlen(formula->ldflags) + 50U;
        char   ldflags[ldflagsCapacity];

        ret = snprintf(ldflags, ldflagsCapacity, "%s -L%s/lib -Wl,-rpath,%s/%s/lib %s", extraLDFlagsForTargetBuild, packageWorkingTopDIR, packageInstalledRootDIR, packageName, formula->ldflags);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (setenv("LDFLAGS", ldflags, 1) != 0) {
            perror("LDFLAGS");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    bool needToCopyStaticLibs = formula->support_create_mostly_statically_linked_executable && (!installOptions->linkSharedLibs);

    if (needToCopyStaticLibs) {
        if (setenv("XCPKG_MSLE", "1", 1) == -1) {
            perror("XCPKG_MSLE");
            return XCPKG_ERROR;
        }
    } else {
        if (unsetenv("XCPKG_MSLE") == -1) {
            perror("XCPKG_MSLE");
            return XCPKG_ERROR;
        }
    }

    if (txt->ptr != NULL) {
        ret = config_envs_for_target(txt->ptr, packageInstalledRootDIR, packageInstalledRootDIRCapacity, nativePackageInstalledRootDIR, nativePackageInstalledRootDIRCapacity, packageWorkingTopDIR, packageWorkingTopDIRCapacity, needToCopyStaticLibs, isCrossBuild);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCmake) {
        // https://cmake.org/cmake/help/latest/envvar/CMAKE_GENERATOR.html
        if (setenv("CMAKE_GENERATOR", formula->useBuildSystemNinja ? "Ninja" : "Unix Makefiles", 1) != 0) {
            perror("CMAKE_GENERATOR");
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_TYPE.html
        if (setenv("CMAKE_BUILD_TYPE", installOptions->buildType == XCPKGBuildProfile_release ? "Release" : "Debug", 1) != 0) {
            perror("CMAKE_BUILD_TYPE");
            return XCPKG_ERROR;
        }

        char ns[4];

        ret = snprintf(ns, 4, "%zu", njobs);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_BUILD_PARALLEL_LEVEL.html
        if (setenv("CMAKE_BUILD_PARALLEL_LEVEL", ns, 1) != 0) {
            perror("CMAKE_BUILD_PARALLEL_LEVEL");
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/envvar/CMAKE_EXPORT_COMPILE_COMMANDS.html
        if (setenv("CMAKE_EXPORT_COMPILE_COMMANDS", installOptions->exportCompileCommandsJson ? "ON" : "OFF", 1) != 0) {
            perror("CMAKE_EXPORT_COMPILE_COMMANDS");
            return XCPKG_ERROR;
        }

        // https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html#manual:cmake-env-variables(7)

        const char* cmakeenvs[] = {
            "CMAKE_PREFIX_PATH",
            "CMAKE_APPLE_SILICON_PROCESSOR",
            "CMAKE_BUILD_TYPE",
            "CMAKE_CONFIGURATION_TYPES",
            "CMAKE_CONFIG_TYPE",
            "CMAKE_GENERATOR_INSTANCE",
            "CMAKE_GENERATOR_PLATFORM",
            "CMAKE_GENERATOR_TOOLSET",
            "CMAKE_INSTALL_MODE",
            "CMAKE_C_COMPILER_LAUNCHER",
            "CMAKE_C_LINKER_LAUNCHER",
            "CMAKE_CXX_COMPILER_LAUNCHER",
            "CMAKE_CXX_LINKER_LAUNCHER",
            "CMAKE_MSVCIDE_RUN_PATH",
            "CMAKE_NO_VERBOSE",
            "CMAKE_OSX_ARCHITECTURES",
            "CMAKE_TOOLCHAIN_FILE",
            "DESTDIR",
            "CTEST_INTERACTIVE_DEBUG_MODE",
            "CTEST_OUTPUT_ON_FAILURE",
            "CTEST_PARALLEL_LEVEL",
            "CTEST_PROGRESS_OUTPUT",
            "CTEST_USE_LAUNCHERS_DEFAULT",
            "DASHBOARD_TEST_FROM_CTEST",
            NULL
        };

        for (int i = 0; ; i++) {
            const char * name = cmakeenvs[i];

            if (name == NULL) {
                break;
            }

            if (unsetenv(name) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemGolang) {
        // https://pkg.go.dev/cmd/cgo
        // https://golang.org/doc/install/source#environment
        KV goenvs[8] = {
            { "GOOS",         "darwin" },
            { "GOARCH",       strcmp(targetPlatformArch, "x86_64") == 0 ? "amd64" : targetPlatformArch },
            { "GO111MODULE",  "auto" },
            { "CGO_ENABLED",  "0" },
            { "CGO_CFLAGS",   getenv("CFLAGS") },
            { "CGO_CXXFLAGS", getenv("CXXFLAGS") },
            { "CGO_CPPFLAGS", getenv("CPPFLAGS") },
            { "CGO_LDFLAGS",  getenv("LDFLAGS") },
        };

        for (int i = 0; i < 8; i++) {
            const char * name  = goenvs[i].name;
            const char * value = goenvs[i].value;

            if (value == NULL) {
                value = "";
            }

            if (setenv(name, value, 1) != 0) {
                perror(name);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char pathBuf[PATH_MAX];

    char * p;

    for (size_t i = 0U; ; i++) {
        if (packageWorkingTopDIR[i] == '\0') {
            pathBuf[i] = '/';
            p = pathBuf + i + 1;
            break;
        }

        pathBuf[i] = packageWorkingTopDIR[i];
    }

    const char * s = "lib";

    for (size_t i = 0U; ; i++) {
        p[i] = s[i];

        if (p[i] == '\0') {
            break;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemCargo) {
        ret = setup_rust_env(targetPlatformArch, pathBuf, packageWorkingTopDIRCapacity + 4U, isCrossBuild, njobs);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // override the default search directory (usually /usr/lib/pkgconfig:/usr/share/pkgconfig)
    // because we only want to use our own
    if (setenv("PKG_CONFIG_LIBDIR", pathBuf, 1) != 0) {
        perror("PKG_CONFIG_LIBDIR");
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (installOptions->dryrun) {
        const char * const SHELL = getenv("SHELL");

        if (SHELL == NULL) {
            fprintf(stderr, "SHELL environment variable is not set.\n");
            return XCPKG_ERROR;
        }

        if (SHELL[0] == '\0') {
            fprintf(stderr, "SHELL environment variable 's value should be a non-empty string.\n");
            return XCPKG_ERROR;
        }

        execl(SHELL, SHELL, NULL);

        perror(SHELL);

        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_fork_exec2(3, "/bin/sh", shellScriptFileName, "target");

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (packageInstalledDIR) != 0) {
        perror(packageInstalledDIR);

        if (errno == ENOENT) {
            fprintf(stderr, "nothing was installed.\n");
        }

        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* a[2] = { ".crates.toml", ".crates2.json" };

    struct stat st;

    for (size_t i = 0; i < 2; i++) {
        if (stat(a[i], &st) == 0 && S_ISREG(st.st_mode)) {
            if (unlink(a[i]) != 0) {
                perror(a[i]);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * const packageMetaInfoDIR = ".xcpkg";

    if (mkdir(packageMetaInfoDIR, S_IRWXU) != 0) {
        if (errno != EEXIST) {
            perror(packageMetaInfoDIR);
            return XCPKG_ERROR;
        }
    }

    if (chdir (packageMetaInfoDIR) != 0) {
        perror(packageMetaInfoDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (txt->ptr != NULL) {
        ret = backup_formulas(sessionDIR, sessionDIRLength, txt->ptr);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_write_file("dependencies.dot", dot->ptr, dot->length);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_write_file("dependencies.d2", d2->ptr, d2->length);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    s = "src/";

    for (size_t i = 0U; ; i++) {
        p[i] = s[i];

        if (p[i] == '\0') {
            p = &p[i];
            break;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* arr1[12] = { "AUTHORS", "LICENSE", "COPYING", "FAQ", "TODO", "NEWS", "THANKS", "CHANGELOG", "CHANGES", "README", "CONTRIBUTORS", "CONTRIBUTING" };

    const char* arr2[3] = { "", ".md", ".rst" };

    for (size_t i = 0U; i < 12U; i++) {
        char * q = p;

        for (size_t k = 0U; ; k++) {
            q[k] = arr1[i][k];

            if (q[k] == '\0') {
                q = &q[k];
                break;
            }
        }

        for (size_t j = 0U; j < 3U; j++) {
            for (size_t k = 0U; ; k++) {
                q[k] = arr2[j][k];

                if (q[k] == '\0') {
                    break;
                }
            }

            fprintf(stderr, "%s => %s\n", pathBuf, p);

            if (stat(pathBuf, &st) == 0 && S_ISREG(st.st_mode)) {
                ret = xcpkg_rename_or_copy_file(pathBuf, p);

                if (ret != XCPKG_OK) {
                    return ret;
                }
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install config.log

    s = "config.log";

    for (size_t i = 0U; ; i++) {
        p[i] = s[i];

        if (p[i] == '\0') {
            break;
        }
    }

    if (stat(pathBuf, &st) == 0 && S_ISREG(st.st_mode)) {
        ret = xcpkg_rename_or_copy_file(pathBuf, s);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////
    // install compile_commands.json

    if (installOptions->exportCompileCommandsJson) {
        s = "compile_commands.json";

        for (size_t i = 0U; ; i++) {
            p[i] = s[i];

            if (p[i] == '\0') {
                break;
            }
        }

        if (stat(pathBuf, &st) == 0 && S_ISREG(st.st_mode)) {
            ret = xcpkg_rename_or_copy_file(pathBuf, s);

            if (ret != XCPKG_OK) {
                return ret;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    s = "dependencies/lib/";

    if (rmdir(s) == -1) {
        if (errno != ENOENT && errno != ENOTEMPTY) {
            perror(s);
            return XCPKG_ERROR;
        }
    } else {
        s = "dependencies/";

        if (rmdir(s) == -1) {
            if (errno != ENOENT && errno != ENOTEMPTY) {
                perror(s);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = generate_manifest(packageInstalledDIR);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = generate_receipt(packageName, formula, targetPlatformSpec, sysinfo, ts);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t treeCmdCapacity = uppmPackageInstalledRootDIRCapacity + packageInstalledDIRCapacity + 30U;
    char   treeCmd[treeCmdCapacity];

    ret = snprintf(treeCmd, treeCmdCapacity, "%s/tree/bin/tree -a --dirsfirst %s", uppmPackageInstalledRootDIR, packageInstalledDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ret = xcpkg_fork_exec(treeCmd);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (chdir (packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        if (symlink(packageInstalledSHA, packageName) == 0) {
            fprintf(stderr, "package '%s' was successfully installed.\n", packageName);
            break;
        } else {
            if (errno == EEXIST) {
                if (lstat(packageName, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = xcpkg_rm_rf(packageName, false, installOptions->logLevel >= XCPKGLogLevel_verbose);

                        if (ret != XCPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(packageName) != 0) {
                            perror(packageName);
                            return XCPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(packageName);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    if (installOptions->keepSessionDIR) {
        return XCPKG_OK;
    } else {
        return xcpkg_rm_rf(packageWorkingTopDIR, false, installOptions->logLevel >= XCPKGLogLevel_verbose);
    }
}

static int check_and_read_formula_in_cache(const char * packageName, const char * targetPlatformName, const char * sessionDIR, XCPKGPackage *** ppackageSet, size_t * ppackageSetSize, size_t * ppackageSetCapacity) {
    size_t          packageSetCapacity = (*ppackageSetCapacity);
    size_t          packageSetSize     = (*ppackageSetSize);
    XCPKGPackage ** packageSet         = (*ppackageSet);

    size_t   packageNameStackCapacity = 10U;
    size_t   packageNameStackSize    = 0U;
    char * * packageNameStack = (char**)malloc(10 * sizeof(char*));

    if (packageNameStack == NULL) {
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStackSize = 1U;

    int ret = XCPKG_OK;

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0U) {
        size_t topIndex = packageNameStackSize - 1U;
        char * packageName = packageNameStack[topIndex];
        packageNameStack[topIndex] = NULL;
        packageNameStackSize--;

        XCPKGFormula * formula = NULL;

        for (size_t i = 0U; i < packageSetSize; i++) {
            if (strcmp(packageSet[i]->packageName, packageName) == 0) {
                free(packageName);

                packageName = packageSet[i]->packageName;
                formula = packageSet[i]->formula;

                size_t lastIndex = packageSetSize - 1U;

                if (i != lastIndex) {
                    XCPKGPackage * package = packageSet[i];

                    for (size_t j = i + 1U; j < packageSetSize; j++) {
                        packageSet[j - 1] = packageSet[j];
                    }

                    packageSet[lastIndex] = package;
                }

                break;
            }
        }

        if (formula == NULL) {
            char formulaFilePath[PATH_MAX];

            ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

            if (ret != XCPKG_OK) {
                free(packageName);
                goto finalize;
            }

            size_t formulaFilePath2Capacity = strlen(sessionDIR) + strlen(packageName) + 6U;
            char   formulaFilePath2[formulaFilePath2Capacity];

            ret = snprintf(formulaFilePath2, formulaFilePath2Capacity, "%s/%s.yml", sessionDIR, packageName);

            if (ret < 0) {
                perror(NULL);
                free(packageName);
                return XCPKG_ERROR;
            }

            ret = xcpkg_copy_file(formulaFilePath, formulaFilePath2);

            if (ret != XCPKG_OK) {
                free(packageName);
                goto finalize;
            }

            ret = xcpkg_formula_load(packageName, targetPlatformName, formulaFilePath2, &formula);

            if (ret != XCPKG_OK) {
                free(packageName);
                goto finalize;
            }

            if (packageSetSize == packageSetCapacity) {
                XCPKGPackage ** p = (XCPKGPackage**)realloc(packageSet, (packageSetCapacity + 10U) * sizeof(XCPKGPackage*));

                if (p == NULL) {
                    free(packageName);
                    xcpkg_formula_free(formula);
                    ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageSet = p;
                packageSetCapacity += 10U;
            }

            XCPKGPackage * package = (XCPKGPackage*)malloc(sizeof(XCPKGPackage));

            if (package == NULL) {
                free(packageName);
                xcpkg_formula_free(formula);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            package->formula = formula;
            package->packageName = packageName;

            packageSet[packageSetSize] = package;
            packageSetSize++;
        }

        if (formula->dep_pkg == NULL) {
            continue;
        }

        const char * p = formula->dep_pkg;

        size_t i;

        loop:
            if (p[0] == '\0') continue;

            if (p[0] == ' ') {
                p++;
                goto loop;
            }

            ///////////////////////////////////////////

            for (i = 0U; ; i++) {
                if (p[i] == ' ' || p[i] == '\0') break;
            }

            ///////////////////////////////////////////

            if (_str_equal(p, packageName)) {
                fprintf(stderr, "package '%s' depends itself.\n", packageName);
                ret = XCPKG_ERROR;
                goto finalize;
            }

            ///////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** q = (char**)realloc(packageNameStack, (packageNameStackCapacity + 10U) * sizeof(char*));

                if (q == NULL) {
                    ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                packageNameStack = q;
                packageNameStackCapacity += 10U;
            }

            char * q = (char*)malloc(i + 1);

            if (q == NULL) {
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            for (size_t j = 0U; j < i; j++) {
                q[j] = p[j];
            }

            q[i] = '\0';

            packageNameStack[packageNameStackSize] = q;
            packageNameStackSize++;

            if (p[i] == ' ') {
                p += i + 1;
                goto loop;
            }
    }

finalize:
    for (size_t i = 0U; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    if (ret == XCPKG_OK) {
        (*ppackageSet) = packageSet;
        (*ppackageSetSize) = packageSetSize;
        (*ppackageSetCapacity) = packageSetCapacity;
    } else {
        for (size_t i = 0U; i < packageSetSize; i++) {
            free(packageSet[i]->packageName);
            xcpkg_formula_free(packageSet[i]->formula);

            packageSet[i]->formula = NULL;
            packageSet[i]->packageName = NULL;

            free(packageSet[i]);
            packageSet[i] = NULL;
        }
    }

    return ret;
}

static int check_if_compiler_support_Wno_error_unused_command_line_argument(const char * sessionDIR, const size_t sessionDIRLength, const char * compiler, const bool iscxx, const char * ccflags, const char * ldflags) {
    size_t testCFilePathCapacity = sessionDIRLength + 10U;
    char   testCFilePath[testCFilePathCapacity];

    int ret = snprintf(testCFilePath, testCFilePathCapacity, "%s/test.c",  sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (iscxx) {
        testCFilePath[ret]     = 'c';
        testCFilePath[ret + 1] = '\0';
    }

    int fd = open(testCFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(testCFilePath);
        return XCPKG_ERROR;
    }

    const char * testCCode = "int main() {\nreturn 0;\n}\n";

    size_t testCCodeLength = strlen(testCCode);

    ssize_t written = write(fd, testCCode, testCCodeLength);

    if (written == -1) {
        perror(testCFilePath);
        close(fd);
        return XCPKG_ERROR;
    }

    close(fd);

    if ((size_t)written != testCCodeLength) {
        fprintf(stderr, "file not fully written: %s\n", testCFilePath);
        return XCPKG_ERROR;
    }

    size_t cmdCapacity = strlen(compiler) + strlen(ccflags) + strlen(ldflags) + testCFilePathCapacity + 45U;
    char   cmd[cmdCapacity];

    ret = snprintf(cmd, cmdCapacity, "%s -Wno-error=unused-command-line-argument %s %s %s", compiler, ccflags, ldflags, testCFilePath);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    return xcpkg_fork_exec(cmd);
}

static int setup_core_tools(const char * sessionDIR, const size_t sessionDIRLength, const char * xcpkgCoreDIR, const size_t xcpkgCoreDIRCapacity, bool verbose) {
    size_t okFilePathCapacity = xcpkgCoreDIRCapacity + 3U;
    char   okFilePath[okFilePathCapacity];

    int ret = snprintf(okFilePath, okFilePathCapacity, "%s/ok", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    struct stat st;

    if (stat(okFilePath, &st) == 0 && S_ISREG(st.st_mode)) {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////////////////////////////////////

    size_t tmpDIRCapacity = sessionDIRLength + 6U;
    char   tmpDIR[tmpDIRCapacity];

    ret = snprintf(tmpDIR, tmpDIRCapacity, "%s/core", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    if (mkdir(tmpDIR, S_IRWXU) != 0) {
        perror(tmpDIR);
        return XCPKG_ERROR;
    }

    if (chdir(tmpDIR) != 0) {
        perror(tmpDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_http_fetch_to_file("https://curl.se/ca/cacert.pem", "cacert.pem", verbose, verbose);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_write_file("xcpkg-install", XCPKG_INSTALL_SHELL_SCRIPT_STRING, XCPKG_INSTALL_SHELL_SCRIPT_STRING_LENGTH);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 6; i++) {
        const char * o;
        const char * s;
        const char * b;

        size_t l;

        switch (i) {
            case 0:
                o = "wrapper-native-cc";
                s = "wrapper-native-cc.c";
                b = XCPKG_WRAPPER_NATIVE_CC_C_SOURCE_STRING;
                l = XCPKG_WRAPPER_NATIVE_CC_C_SOURCE_STRING_LENGTH;
                break;
            case 1:
                o = "wrapper-native-c++";
                s = "wrapper-native-c++.c";
                b = XCPKG_WRAPPER_NATIVE_CXX_C_SOURCE_STRING;
                l = XCPKG_WRAPPER_NATIVE_CXX_C_SOURCE_STRING_LENGTH;
                break;
            case 2:
                o = "wrapper-native-objc";
                s = "wrapper-native-objc.c";
                b = XCPKG_WRAPPER_NATIVE_OBJC_C_SOURCE_STRING;
                l = XCPKG_WRAPPER_NATIVE_OBJC_C_SOURCE_STRING_LENGTH;
                break;
            case 3:
                o = "wrapper-target-cc";
                s = "wrapper-target-cc.c";
                b = XCPKG_WRAPPER_TARGET_CC_C_SOURCE_STRING;
                l = XCPKG_WRAPPER_TARGET_CC_C_SOURCE_STRING_LENGTH;
                break;
            case 4:
                o = "wrapper-target-c++";
                s = "wrapper-target-c++.c";
                b = XCPKG_WRAPPER_TARGET_CXX_C_SOURCE_STRING;
                l = XCPKG_WRAPPER_TARGET_CXX_C_SOURCE_STRING_LENGTH;
                break;
            case 5:
                o = "wrapper-target-objc";
                s = "wrapper-target-objc.c";
                b = XCPKG_WRAPPER_TARGET_OBJC_C_SOURCE_STRING;
                l = XCPKG_WRAPPER_TARGET_OBJC_C_SOURCE_STRING_LENGTH;
                break;
        }

        ret = xcpkg_write_file(s, b, l);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_fork_exec2(8, "/usr/bin/cc", "-std=c99", "-Os", "-Wl,-S", "-flto", "-o", o, s);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////

    ret = xcpkg_write_file("ok", XCPKG_VERSION_STRING, strlen(XCPKG_VERSION_STRING));

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////////

    if (chdir(sessionDIR) != 0) {
        perror(sessionDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////////

    for (;;) {
        if (rename("core", xcpkgCoreDIR) == 0) {
            size_t cacertFilePathCapacity = xcpkgCoreDIRCapacity + 11U;
            char   cacertFilePath[cacertFilePathCapacity];

            ret = snprintf(cacertFilePath, cacertFilePathCapacity, "%s/cacert.pem", xcpkgCoreDIR);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            // https://www.openssl.org/docs/man1.1.1/man3/SSL_CTX_set_default_verify_paths.html
            if (setenv("SSL_CERT_FILE", cacertFilePath, 1) != 0) {
                perror("SSL_CERT_FILE");
                return XCPKG_ERROR;
            }

            return XCPKG_OK;
        } else {
            if (errno == ENOTEMPTY || errno == EEXIST) {
                if (lstat(xcpkgCoreDIR, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        ret = xcpkg_rm_rf(xcpkgCoreDIR, false, verbose);

                        if (ret != XCPKG_OK) {
                            return ret;
                        }
                    } else {
                        if (unlink(xcpkgCoreDIR) != 0) {
                            perror(xcpkgCoreDIR);
                            return XCPKG_ERROR;
                        }
                    }
                }
            } else {
                perror(xcpkgCoreDIR);
                return XCPKG_ERROR;
            }
        }
    }
}

int xcpkg_install(const char * packageName, const char * targetPlatformSpec, const XCPKGInstallOptions * installOptions) {
    // redirect all stdout and stderr to /dev/null
    if (installOptions->logLevel == XCPKGLogLevel_silent) {
        int fd = open("/dev/null", O_CREAT | O_TRUNC | O_WRONLY, 0666);

        if (fd < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }

        if (dup2(fd, STDERR_FILENO) < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }
    }

    const char * const PATH = getenv("PATH");

    if (PATH == NULL || PATH[0] == '\0') {
        return XCPKG_ERROR_ENV_PATH_NOT_SET;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (installOptions->verbose_cc) {
        if (setenv("XCPKG_VERBOSE", "1", 1) != 0) {
            perror("XCPKG_VERBOSE");
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    // https://perldoc.perl.org/cpan#PERL_MM_USE_DEFAULT
    // Would you like to configure as much as possible automatically?
    if (setenv("PERL_MM_USE_DEFAULT", "1", 1) != 0) {
        perror("PERL_MM_USE_DEFAULT");
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * unsetenvs[] = {
        // autoreconf --help
        "AUTOCONF",
        "AUTOHEADER",
        "AUTOM4TE",
        "AUTOMAKE",
        "AUTOPOINT",
        "ACLOCAL",
        "GTKDOCIZE",
        "INTLTOOLIZE",
        "LIBTOOLIZE",
        "M4",
        "MAKE",
        "autom4te_perllibdir",
        "trailer_m4",

        // https://gcc.gnu.org/onlinedocs/cpp/Environment-Variables.html
        // https://gcc.gnu.org/onlinedocs/gcc/Environment-Variables.html
        // https://clang.llvm.org/docs/CommandGuide/clang.html#environment
        "CPATH",
        "C_INCLUDE_PATH",
        "CPLUS_INCLUDE_PATH",
        "OBJC_INCLUDE_PATH",
        "OBJCPLUS_INCLUDE_PATH",
        "DEPENDENCIES_OUTPUT",
        "SUNPRO_DEPENDENCIES",
        "SOURCE_DATE_EPOCH",
        "GCC_EXEC_PREFIX",
        "COMPILER_PATH",
        "LIBRARY_PATH",

        // https://cmake.org/cmake/help/latest/envvar/MACOSX_DEPLOYMENT_TARGET.html
        // https://clang.llvm.org/docs/CommandGuide/clang.html#environment
        "MACOSX_DEPLOYMENT_TARGET",
        "IPHONEOS_DEPLOYMENT_TARGET",
        "WATCHOS_DEPLOYMENT_TARGET",

        // https://cmake.org/cmake/help/latest/envvar/DESTDIR.html
        // https://www.gnu.org/prep/standards/html_node/DESTDIR.html
        // https://mensinda.github.io/meson/Installing.html#destdir-support
        "DESTDIR",

        "TARGET_ARCH",

        "XCPKG_TARGET_LDFLAGS",
        "XCPKG_TARGET_CCFLAGS",
        "XCPKG_TARGET_CXXFLAGS",
        "XCPKG_TARGET_OBJCFLAGS",

        NULL
    };

    for (int i = 0; unsetenvs[i] != NULL; i++) {
        if (unsetenv(unsetenvs[i]) != 0) {
            perror(unsetenvs[i]);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t xcpkgCoreDIRCapacity = xcpkgHomeDIRLength + 6U;
    char   xcpkgCoreDIR[xcpkgCoreDIRCapacity];

    ret = snprintf(xcpkgCoreDIR, xcpkgCoreDIRCapacity, "%s/core", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t xcpkgDownloadsDIRCapacity = xcpkgHomeDIRLength + 11U;
    char   xcpkgDownloadsDIR[xcpkgDownloadsDIRCapacity];

    ret = snprintf(xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, "%s/downloads", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(xcpkgDownloadsDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(xcpkgDownloadsDIR) != 0) {
                perror(xcpkgDownloadsDIR);
                return XCPKG_ERROR;
            }

            if (mkdir(xcpkgDownloadsDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(xcpkgDownloadsDIR);
                    return XCPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(xcpkgDownloadsDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(xcpkgDownloadsDIR);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = setup_core_tools(sessionDIR, sessionDIRLength, xcpkgCoreDIR, xcpkgCoreDIRCapacity, installOptions->verbose_net);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t uppmPackageInstalledRootDIRCapacity = xcpkgHomeDIRLength + 16U;
    char   uppmPackageInstalledRootDIR[uppmPackageInstalledRootDIRCapacity];

    ret = snprintf(uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRCapacity, "%s/uppm/installed", xcpkgHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t pkgconfigCapacity = uppmPackageInstalledRootDIRCapacity + 26U;
    char   pkgconfig[pkgconfigCapacity];

    ret = snprintf(pkgconfig, pkgconfigCapacity, "%s/pkg-config/bin/pkg-config", uppmPackageInstalledRootDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (setenv("PKG_CONFIG", pkgconfig, 1) != 0) {
        perror("PKG_CONFIG");
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo sysinfo = {0};

    ret = sysinfo_make(&sysinfo);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////

    size_t capacity = xcpkgCoreDIRCapacity + 21U;

    char ccForNativeBuild[capacity];

    ret = snprintf(ccForNativeBuild, capacity, "%s/wrapper-native-cc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char cxxForNativeBuild[capacity];

    ret = snprintf(cxxForNativeBuild, capacity, "%s/wrapper-native-c++", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char objcForNativeBuild[capacity];

    ret = snprintf(objcForNativeBuild, capacity, "%s/wrapper-native-objc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t cppCapacity = capacity + 4U;

    char cppForNativeBuild[cppCapacity];

    ret = snprintf(cppForNativeBuild, cppCapacity, "%s -E", ccForNativeBuild);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    char ccForTargetBuild[capacity];

    ret = snprintf(ccForTargetBuild, capacity, "%s/wrapper-target-cc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char cxxForTargetBuild[capacity];

    ret = snprintf(cxxForTargetBuild, capacity, "%s/wrapper-target-c++", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char objcForTargetBuild[capacity];

    ret = snprintf(objcForTargetBuild, capacity, "%s/wrapper-target-objc", xcpkgCoreDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char cppForTargetBuild[cppCapacity];

    ret = snprintf(cppForTargetBuild, cppCapacity, "%s -E", ccForTargetBuild);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    XCPKGToolChain toolchain = {0};

    ret = xcpkg_toolchain_find(&toolchain);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (installOptions->verbose_xcode) {
        xcpkg_toolchain_dump(&toolchain);
    }

    const KV proxiedTools[3] = {
        { "XCPKG_CC",   toolchain.cc  },
        { "XCPKG_CXX",  toolchain.cxx },
        { "XCPKG_OBJC", toolchain.cc  }
    };

    for (int i = 0; i < 3; i++) {
        const char * name  = proxiedTools[i].name;
        const char * value = proxiedTools[i].value;

        if (value == NULL) {
            if (unsetenv(name) != 0) {
                perror(name);
                xcpkg_toolchain_free(&toolchain);
                return XCPKG_ERROR;
            }
        } else {
            if (setenv(name, value, 1) != 0) {
                perror(name);
                xcpkg_toolchain_free(&toolchain);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char sysrootForNativeBuild[PATH_MAX]; sysrootForNativeBuild[0] = '\0';

    ret = xcpkg_sdk_path("macosx", sysrootForNativeBuild);

    if (ret != XCPKG_OK) {
        xcpkg_toolchain_free(&toolchain);
        return ret;
    }

    if (sysrootForNativeBuild[0] == '\0') {
        xcpkg_toolchain_free(&toolchain);
        fprintf(stderr, "Can not locate MacOSX sdk path.\n");
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t commonFlagsForNativeBuildCapacity = strlen(sysrootForNativeBuild) + 80U;
    char   commonFlagsForNativeBuild[commonFlagsForNativeBuildCapacity];

    ret = snprintf(commonFlagsForNativeBuild, commonFlagsForNativeBuildCapacity, "-isysroot %s -mmacosx-version-min=%s -arch %s -Qunused-arguments", sysrootForNativeBuild, sysinfo.vers, sysinfo.arch);

    if (ret < 0) {
        perror(NULL);
        xcpkg_toolchain_free(&toolchain);
        return XCPKG_ERROR;
    }

    if (setenv("XCPKG_NATIVE_FLAGS", commonFlagsForNativeBuild, 1) != 0) {
        perror("XCPKG_NATIVE_FLAGS");
        xcpkg_toolchain_free(&toolchain);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t extraCCFlagsCapacity = 50U;
    char   extraCCFlags[extraCCFlagsCapacity];

    const char * s = "-fPIC -fno-common";

    char * p = extraCCFlags;

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (s[i] == '\0') {
            p = &p[i];
            break;
        }
    }

    if (installOptions->buildType == XCPKGBuildProfile_release) {
        s = " -Os";
    } else {
        s = " -g -O0";
    }

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (s[i] == '\0') {
            p = &p[i];
            break;
        }
    }

    if (installOptions->verbose_cc) {
        s = " -v";

        for (int i = 0; ; i++) {
            p[i] = s[i];

            if (s[i] == '\0') {
                p = &p[i];
                break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t extraLDFlagsCapacity = 50U;
    char   extraLDFlags[extraLDFlagsCapacity];

    s = "-Wl,-search_paths_first";

    p = extraLDFlags;

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (s[i] == '\0') {
            p = &p[i];
            break;
        }
    }

    if (installOptions->buildType == XCPKGBuildProfile_release) {
        s = " -Wl,-S";

        for (int i = 0; ; i++) {
            p[i] = s[i];

            if (s[i] == '\0') {
                p = &p[i];
                break;
            }
        }
    }

    if (installOptions->verbose_ld) {
        s = " -Wl,-v";

        for (int i = 0; ; i++) {
            p[i] = s[i];

            if (s[i] == '\0') {
                p = &p[i];
                break;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    char xcpkgExeFilePath[PATH_MAX];

    ret = selfpath(xcpkgExeFilePath);

    if (ret == -1) {
        perror(NULL);
        xcpkg_toolchain_free(&toolchain);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    size_t          packageSetCapacity = 0U;
    size_t          packageSetSize     = 0U;
    XCPKGPackage ** packageSet         = NULL;

    ret = check_and_read_formula_in_cache(packageName, NULL, sessionDIR, &packageSet, &packageSetSize, &packageSetCapacity);

    if (ret != XCPKG_OK) {
        goto finalize;
    }

    //////////////////////////////////////////////////////////////////////////////

    if (packageSetSize > 1) {
        printf("install packages in order:");

        for (int i = packageSetSize - 1; i >= 0; i--) {
            printf(" %s", packageSet[i]->packageName);
        }

        printf("\n");
    }

    //////////////////////////////////////////////////////////////////////////////

    for (int i = packageSetSize - 1; i >= 0; i--) {
        XCPKGPackage * package = packageSet[i];
        char * packageName = package->packageName;

        if (!installOptions->force) {
            ret = xcpkg_check_if_the_given_package_is_installed(packageName, targetPlatformSpec);

            if (ret == XCPKG_OK) {
                fprintf(stderr, "package already has been installed : %s\n", packageName);
                continue;
            }
        }

        if (setenv("PATH", PATH, 1) != 0) {
            perror("PATH");
            goto finalize;
        }

        if (installOptions->verbose_formula) {
            xcpkg_formula_dump(package->formula);
        }

        StringBuf txt = {0};
        StringBuf dot = {0};
        StringBuf d2  = {0};

        if (package->formula->dep_pkg != NULL) {
            ret = generate_dependencies_graph(packageName, packageSet, packageSetSize, &txt, &dot, &d2);

            if (ret != XCPKG_OK) {
                free(txt.ptr);
                free(dot.ptr);
                free(d2.ptr);
                goto finalize;
            }
        }

        fprintf(stderr, "txt=%s\n", txt.ptr);
        fprintf(stderr, "dot=%s\n", dot.ptr);
        fprintf(stderr, "d2=%s\n", d2.ptr);

        ret = xcpkg_install_package(packageName, targetPlatformSpec, package->formula, installOptions, &toolchain, &sysinfo, ccForNativeBuild, cxxForNativeBuild, cppForNativeBuild, objcForNativeBuild, ccForTargetBuild, cxxForTargetBuild, cppForTargetBuild, objcForTargetBuild, extraCCFlags, extraCCFlags, "", extraLDFlags, extraCCFlags, extraLDFlags, uppmPackageInstalledRootDIR, uppmPackageInstalledRootDIRCapacity, xcpkgExeFilePath, xcpkgHomeDIR, xcpkgHomeDIRLength, xcpkgCoreDIR, xcpkgCoreDIRCapacity, xcpkgDownloadsDIR, xcpkgDownloadsDIRCapacity, sessionDIR, sessionDIRLength, &txt, &dot, &d2);

        free(txt.ptr);
        free(dot.ptr);
        free(d2.ptr);

        if (ret != XCPKG_OK) {
            goto finalize;
        }
    }

finalize:
    xcpkg_toolchain_free(&toolchain);

    for (size_t i = 0; i < packageSetSize; i++) {
        free(packageSet[i]->packageName);
        xcpkg_formula_free(packageSet[i]->formula);

        packageSet[i]->formula = NULL;
        packageSet[i]->packageName = NULL;

        free(packageSet[i]);
        packageSet[i] = NULL;
    }

    free(packageSet);
    packageSet = NULL;

    if (ret == XCPKG_OK) {
        if (!installOptions->keepSessionDIR) {
            ret = xcpkg_rm_rf(sessionDIR, false, false);
        }
    }

    return ret;
}
