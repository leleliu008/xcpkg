#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <limits.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/file.h>

#include "core/sysinfo.h"

#include "uppm.h"
#include "xcpkg.h"

static int uppm_formula_repo_url_of_official_core(char buf[], const size_t bufSize) {
    char osArch[11] = {0};

    if (sysinfo_arch(osArch, 10) != 0) {
        return XCPKG_ERROR;
    }

    char osVers[11] = {0};

    if (sysinfo_vers(osVers, 10) != 0) {
        return XCPKG_ERROR;
    }

    int osVersMajor = 0;

    for (int i = 0; i < 11; i++) {
        if (osVers[i] == '\0') {
            break;
        }

        if (osVers[i] == '.') {
            osVers[i] = '\0';
            osVersMajor = atoi(osVers);
            break;
        }
    }

    if (osVersMajor < 10) {
        fprintf(stderr, "MacOSX %d.x is not supported.\n", osVersMajor);
        return XCPKG_ERROR;
    }

    if (osVersMajor > 15) {
        osVersMajor = 15;
    }

    int ret = snprintf(buf, bufSize, "https://github.com/leleliu008/uppm-package-repository-macos-%d.0-%s", osVersMajor, osArch);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    } else {
        return XCPKG_OK;
    }
}

static int uppm_formula_repo_sync_official_core_internal(const char * formulaRepoDIR) {
    char formulaRepoUrl[120];

    int ret = uppm_formula_repo_url_of_official_core(formulaRepoUrl, 120);

    if (ret != XCPKG_OK) {
        return ret;
    }

    fprintf(stderr, "uppm formula repository is being synced from %s\n", formulaRepoUrl);

    ret = xcpkg_git_sync(formulaRepoDIR, formulaRepoUrl, "refs/heads/master", "refs/remote/heads/master", "master", 0);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    char ts[11];

    ret = snprintf(ts, 11, "%ld", time(NULL));

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t strCapacity = strlen(formulaRepoUrl) + (ret < 1) + 85U;
    char   str[strCapacity];

    ret = snprintf(str, strCapacity, "url: %s\nbranch: master\npinned: 0\nenabled: 1\ncreated: %s\nupdated: %s\n", formulaRepoUrl, ts, ts);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int strLength = ret;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    size_t formulaRepoConfigFilePathCapacity = strlen(formulaRepoDIR) + 24U;
    char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

    ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.uppm-formula-repo.yml", formulaRepoDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    return xcpkg_write_file(formulaRepoConfigFilePath, str, strLength);
}

int uppm_formula_repo_sync_official_core(const char * uppmHomeDIR, const size_t uppmHomeDIRLength) {
    size_t formulaRepoDIRCapacity = uppmHomeDIRLength + 23U;
    char   formulaRepoDIR[formulaRepoDIRCapacity];

    int ret = snprintf(formulaRepoDIR, formulaRepoDIRCapacity, "%s/repos.d/official-core", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////////////////////////////

    int formulaRepoDIRfd = 0;

    struct stat st;

    if (lstat(formulaRepoDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            formulaRepoDIRfd = open(formulaRepoDIR, O_DIRECTORY);

            if (formulaRepoDIRfd == -1) {
                puts("pppp");
                perror(formulaRepoDIR);
                return XCPKG_ERROR;
            }

            ret = flock(formulaRepoDIRfd, LOCK_EX);

            if (ret == -1) {
                perror(formulaRepoDIR);
                close(formulaRepoDIRfd);
                return XCPKG_ERROR;
            }

            size_t formulaRepoConfigFilePathCapacity = formulaRepoDIRCapacity + 24U;
            char   formulaRepoConfigFilePath[formulaRepoConfigFilePathCapacity];

            ret = snprintf(formulaRepoConfigFilePath, formulaRepoConfigFilePathCapacity, "%s/.uppm-formula-repo.yml", formulaRepoDIR);

            if (ret < 0) {
                perror(NULL);
                close(formulaRepoDIRfd);
                return XCPKG_ERROR;
            }

            if (stat(formulaRepoConfigFilePath, &st) == 0) {
                if (S_ISREG(st.st_mode)) {
                    UPPMFormulaRepo * formulaRepo = NULL;

                    ret = uppm_formula_repo_parse(formulaRepoConfigFilePath, &formulaRepo);

                    uppm_formula_repo_free(formulaRepo);

                    if (ret != XCPKG_OK) {
                        ret = xcpkg_rm_rf(formulaRepoDIR, true, false);

                        if (ret != XCPKG_OK) {
                            close(formulaRepoDIRfd);
                            return ret;
                        }
                    }
                } else {
                    ret = xcpkg_rm_rf(formulaRepoDIR, true, false);

                    if (ret != XCPKG_OK) {
                        close(formulaRepoDIRfd);
                        return ret;
                    }
                }
            } else {
                ret = xcpkg_rm_rf(formulaRepoDIR, true, false);

                if (ret != XCPKG_OK) {
                    close(formulaRepoDIRfd);
                    return ret;
                }
            }
        } else {
            if (unlink(formulaRepoDIR) != 0) {
                perror(formulaRepoDIR);
                return XCPKG_ERROR;
            }

            if (mkdir(formulaRepoDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(formulaRepoDIR);
                    return XCPKG_ERROR;
                }
            }
        }
    } else {
        ret = xcpkg_mkdir_p(formulaRepoDIR, false);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepoDIRfd == 0) {
        formulaRepoDIRfd = open(formulaRepoDIR, O_DIRECTORY);

        if (formulaRepoDIRfd == -1) {
            perror(formulaRepoDIR);
            return XCPKG_ERROR;
        }

        ret = flock(formulaRepoDIRfd, LOCK_EX);

        if (ret == -1) {
            perror(formulaRepoDIR);
            close(formulaRepoDIRfd);
            return XCPKG_ERROR;
        }
    }

    ret = uppm_formula_repo_sync_official_core_internal(formulaRepoDIR);

    close(formulaRepoDIRfd);

    return ret;
}
