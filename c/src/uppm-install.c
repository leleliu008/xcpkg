#include <stdatomic.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <libgen.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "core/sysinfo.h"
#include "core/log.h"
#include "core/tar.h"

#include "sha256sum.h"
#include "uppm.h"
#include "xcpkg.h"

static int generate_install_shell_script_file(
        const char * shellScriptFilePath,
        const char * uppmHomeDIR,
        const char * packageName,
        const char * packageInstalledRealDIR,
        const char * binFileName,
        const char * binFileType,
        const char * binFilePath,
        const UPPMFormula * formula) {
    int fd = open(shellScriptFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(shellScriptFilePath);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    int ret;

    if (true) {
        ret = dprintf(fd, "set -x\n");

        if (ret < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    ret = dprintf(fd, "set -e\n\n");

    if (ret < 0) {
        perror(NULL);
        close(fd);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    SysInfo sysinfo = {0};

    if (sysinfo_make(&sysinfo) != 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////////

    const char* k[4] = {
        "UPPM_VERSION_MAJOR",
        "UPPM_VERSION_MINOR",
        "UPPM_VERSION_PATCH",
        "NATIVE_OS_NCPU"
    };

    unsigned int v[] = {
        UPPM_VERSION_MAJOR,
        UPPM_VERSION_MINOR,
        UPPM_VERSION_PATCH,
        sysinfo.ncpu
    };

    const char* kvs[34] = {
        "NATIVE_OS_TYPE", "macos",
        "NATIVE_OS_VERS", sysinfo.vers,
        "NATIVE_OS_ARCH", sysinfo.arch,
        "UPPM_VERSION",    UPPM_VERSION,
        "UPPM_HOME",       uppmHomeDIR,
        "UPPM_EXECUTABLE", NULL,
        "PKG_NAME",        packageName,
        "PKG_SUMMARY", formula->summary,
        "PKG_WEBPAGE", formula->webpage,
        "PKG_VERSION", formula->version,
        "PKG_BIN_URL", formula->bin_url,
        "PKG_BIN_SHA", formula->bin_sha,
        "PKG_DEP_PKG", formula->dep_pkg,
        "PKG_BIN_FILENAME", binFileName,
        "PKG_BIN_FILETYPE", binFileType,
        "PKG_BIN_FILEPATH", binFilePath,
        "PKG_INSTALL_DIR", packageInstalledRealDIR
    };

    //////////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 4; i++) {
        ret = dprintf(fd, "%s='%d'\n", k[i], v[i]);

        if (ret < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }
    }

    for (int i = 0; i < 34; i += 2) {
        const char * value = kvs[i + 1];

        if (value == NULL) value = "";

        ret = dprintf(fd, "%s='%s'\n", kvs[i], value);

        if (ret < 0) {
            perror(NULL);
            close(fd);
            return XCPKG_ERROR;
        }
    }

    //////////////////////////////////////////////////////////////////////////////

    const char * str =
        "for item in $PKG_DEP_PKG\n"
        "do\n"
        "if [ -d \"$UPPM_HOME/installed/$item/bin\" ] ; then\n"
        "PATH=\"$UPPM_HOME/installed/$item/bin:$PATH\"\n"
        "fi\n"
        "done\n\n"
        "pwd\n";

    ret = dprintf(fd, "%s\n%s\n", str, formula->install);

    if (ret < 0) {
        perror(NULL);
        close(fd);
        return XCPKG_ERROR;
    }

    close(fd);

    return XCPKG_OK;
}

static int uppm_record_installed_files_r(const char * dirPath, const size_t offset, const int outputFD) {
    if (dirPath == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (dirPath[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    size_t dirPathLength = strlen(dirPath);

    DIR * dir = opendir(dirPath);

    if (dir == NULL) {
        perror(dirPath);
        return XCPKG_ERROR;
    }

    int ret;

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

        size_t filePathLength = dirPathLength + strlen(dir_entry->d_name) + 2U;
        char   filePath[filePathLength];

        ret = snprintf(filePath, filePathLength, "%s/%s", dirPath, dir_entry->d_name);

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
            ret = dprintf(outputFD, "d|%s/\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return XCPKG_ERROR;
            }

            ret = uppm_record_installed_files_r(filePath, offset, outputFD);

            if (ret != XCPKG_OK) {
                closedir(dir);
                return ret;
            }
        } else {
            ret = dprintf(outputFD, "f|%s\n", &filePath[offset]);

            if (ret < 0) {
                perror(NULL);
                closedir(dir);
                return XCPKG_ERROR;
            }
        }
    }
}

static int uppm_record_installed_files(const char * installedDIRPath) {
    size_t installedDIRLength = strlen(installedDIRPath);

    size_t installedManifestFilePathLength = installedDIRLength + 20U;
    char   installedManifestFilePath[installedManifestFilePathLength];

    int ret = snprintf(installedManifestFilePath, installedManifestFilePathLength, "%s/.uppm/manifest.txt", installedDIRPath);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int fd = open(installedManifestFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (fd == -1) {
        perror(installedManifestFilePath);
        return XCPKG_ERROR;
    }

    ret = uppm_record_installed_files_r(installedDIRPath, installedDIRLength + 1, fd);

    close(fd);

    return ret;
}

static int uppm_install_internal(const char * uppmHomeDIR, const size_t uppmHomeDIRLength, const char * packageName, const UPPMFormula * formula, const bool verbose, const bool force) {
    if (!force) {
        int ret = uppm_check_if_the_given_package_is_installed(packageName, uppmHomeDIR, uppmHomeDIRLength);

        if (ret == XCPKG_OK) {
            fprintf(stderr, "uppm package '%s' already has been installed.\n", packageName);
            return XCPKG_OK;
        }

        if (ret != UPPM_ERROR_PACKAGE_NOT_INSTALLED) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    if (verbose) {
        fprintf(stderr, "uppm package '%s' is being installed.\n", packageName);
    }

    //////////////////////////////////////////////////////////////////////////

    size_t tmpStrCapacity = strlen(formula->bin_url) + 30U;
    char   tmpStr[tmpStrCapacity];

    int ret = snprintf(tmpStr, tmpStrCapacity, "%s|%ld|%d", formula->bin_url, time(NULL), getpid());

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    char sessionID[65] = {0};

    ret = sha256sum_of_string(sessionID, tmpStr);

    if (ret != 0) {
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    struct stat st;

    size_t downloadDIRCapacity = uppmHomeDIRLength + 11U;
    char   downloadDIR[downloadDIRCapacity];

    ret = snprintf(downloadDIR, downloadDIRCapacity, "%s/downloads", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (lstat(downloadDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(downloadDIR) != 0) {
                perror(downloadDIR);
                return XCPKG_ERROR;
            }

            if (mkdir(downloadDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(downloadDIR);
                    return XCPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(downloadDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(downloadDIR);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    char binFileNameExtension[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

    ret = xcpkg_extract_filetype_from_url(formula->bin_url, binFileNameExtension, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    size_t binFileNameLength = strlen(binFileNameExtension) + 65U;
    char   binFileName[binFileNameLength];

    ret = snprintf(binFileName, binFileNameLength, "%s%s", formula->bin_sha, binFileNameExtension);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t binFilePathLength = downloadDIRCapacity + binFileNameLength + 1U;
    char   binFilePath[binFilePathLength];

    ret = snprintf(binFilePath, binFilePathLength, "%s/%s", downloadDIR, binFileName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    bool needFetch = true;

    if (lstat(binFilePath, &st) == 0) {
        if (S_ISREG(st.st_mode)) {
            char actualSHA256SUM[65] = {0};

            if (sha256sum_of_file(actualSHA256SUM, binFilePath) != 0) {
                return XCPKG_ERROR;
            }

            if (strcmp(actualSHA256SUM, formula->bin_sha) == 0) {
                needFetch = false;
            }
        }
    }

    if (needFetch) {
        char * tmpFileName = sessionID;

        size_t tmpFilePathCapacity = downloadDIRCapacity + 65U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/%s", downloadDIR, tmpFileName);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_http_fetch_to_file(formula->bin_url, tmpFilePath, verbose, verbose);

        if (ret != XCPKG_OK) {
            return ret;
        }

        char actualSHA256SUM[65] = {0};

        if (sha256sum_of_file(actualSHA256SUM, tmpFilePath) != 0) {
            return XCPKG_ERROR;
        }

        if (strcmp(actualSHA256SUM, formula->bin_sha) == 0) {
            if (rename(tmpFilePath, binFilePath) == 0) {
                printf("%s -> %s\n", tmpFilePath, binFilePath);
                //printf("%s\n", binFilePath);
            } else {
                perror(binFilePath);
                return XCPKG_ERROR;
            }
        } else {
            fprintf(stderr, "sha256sum mismatch.\n    expect : %s\n    actual : %s\n", formula->bin_sha, actualSHA256SUM);
            return XCPKG_ERROR_SHA256_MISMATCH;
        }
    } else {
        if (verbose) {
            fprintf(stderr, "%s already have been fetched.\n", binFilePath);
        }
    }

    //////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRootDIRCapacity = uppmHomeDIRLength + 21U;
    char   packageInstalledRootDIR[packageInstalledRootDIRCapacity];

    ret = snprintf(packageInstalledRootDIR, packageInstalledRootDIRCapacity, "%s/installed", uppmHomeDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (lstat(packageInstalledRootDIR, &st) == 0) {
        if (!S_ISDIR(st.st_mode)) {
            if (unlink(packageInstalledRootDIR) != 0) {
                perror(packageInstalledRootDIR);
                return XCPKG_ERROR;
            }

            if (mkdir(packageInstalledRootDIR, S_IRWXU) != 0) {
                if (errno != EEXIST) {
                    perror(packageInstalledRootDIR);
                    return XCPKG_ERROR;
                }
            }
        }
    } else {
        if (mkdir(packageInstalledRootDIR, S_IRWXU) != 0) {
            if (errno != EEXIST) {
                perror(packageInstalledRootDIR);
                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////

    size_t packageInstalledRealDIRCapacity = packageInstalledRootDIRCapacity + 66U;
    char   packageInstalledRealDIR[packageInstalledRealDIRCapacity];

    ret = snprintf(packageInstalledRealDIR, packageInstalledRealDIRCapacity, "%s/%s", packageInstalledRootDIR, sessionID);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (lstat(packageInstalledRealDIR, &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
            ret = xcpkg_rm_rf(packageInstalledRealDIR, false, verbose);

            if (ret != XCPKG_OK) {
                return ret;
            }

            if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
                perror(packageInstalledRealDIR);
                return XCPKG_ERROR;
            }
        } else {
            if (unlink(packageInstalledRealDIR) != 0) {
                perror(packageInstalledRealDIR);
                return XCPKG_ERROR;
            }

            if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
                perror(packageInstalledRealDIR);
                return XCPKG_ERROR;
            }
        }
    }

    if (mkdir(packageInstalledRealDIR, S_IRWXU) != 0) {
        perror(packageInstalledRealDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////////

    if (strcmp(binFileNameExtension, ".zip") == 0 ||
        strcmp(binFileNameExtension, ".tgz") == 0 ||
        strcmp(binFileNameExtension, ".txz") == 0 ||
        strcmp(binFileNameExtension, ".tlz") == 0 ||
        strcmp(binFileNameExtension, ".tbz2") == 0) {

        if (formula->unpackd == NULL) {
            ret = tar_extract(packageInstalledRealDIR, binFilePath, ARCHIVE_EXTRACT_TIME, verbose, 1);
        } else {
            size_t extractDIRCapacity = packageInstalledRealDIRCapacity + strlen(formula->unpackd) + 1U;
            char   extractDIR[extractDIRCapacity];

            ret = snprintf(extractDIR, extractDIRCapacity, "%s/%s", packageInstalledRealDIR, formula->unpackd);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            ret = tar_extract(extractDIR, binFilePath, ARCHIVE_EXTRACT_TIME, verbose, 1);
        }

        if (ret != 0) {
            return abs(ret) + XCPKG_ERROR_ARCHIVE_BASE;
        }
    } else {
        size_t toFilePathCapacity = packageInstalledRealDIRCapacity + 66U;
        char   toFilePath[toFilePathCapacity];

        ret = snprintf(toFilePath, toFilePathCapacity, "%s/%s", packageInstalledRealDIR, sessionID);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_copy_file(binFilePath, toFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    if (chdir(packageInstalledRealDIR) != 0) {
        perror(packageInstalledRealDIR);
        return XCPKG_ERROR;
    }

    if (formula->install != NULL) {
        size_t shellScriptFilePathCapacity = packageInstalledRealDIRCapacity + 16U;
        char   shellScriptFilePath[shellScriptFilePathCapacity];

        ret = snprintf(shellScriptFilePath, shellScriptFilePathCapacity, "%s/uppm-install.sh", packageInstalledRealDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = generate_install_shell_script_file(shellScriptFilePath, uppmHomeDIR, packageName, packageInstalledRealDIR, binFileName, binFileNameExtension, binFilePath, formula);

        if (ret != XCPKG_OK) {
            return ret;
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        if (pid == 0) {
            fprintf(stderr, "%s==>%s %s/bin/sh %s %s\n", COLOR_PURPLE, COLOR_OFF, COLOR_GREEN, shellScriptFilePath, COLOR_OFF);
            execl ("/bin/sh", "/bin/sh", shellScriptFilePath, NULL);
            perror("/bin/sh");
            exit(255);
        } else {
            int childProcessExitStatusCode;

            if (waitpid(pid, &childProcessExitStatusCode, 0) < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            if (childProcessExitStatusCode != 0) {
                if (WIFEXITED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running shell code exit with status code: %d\n", WEXITSTATUS(childProcessExitStatusCode));
                } else if (WIFSIGNALED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running shell code killed by signal: %d\n", WTERMSIG(childProcessExitStatusCode));
                } else if (WIFSTOPPED(childProcessExitStatusCode)) {
                    fprintf(stderr, "running shell code stopped by signal: %d\n", WSTOPSIG(childProcessExitStatusCode));
                }

                return XCPKG_ERROR;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////

    size_t packageInstalledMetaInfoDIRCapacity = packageInstalledRealDIRCapacity + 6U;
    char   packageInstalledMetaInfoDIR[packageInstalledMetaInfoDIRCapacity];

    ret = snprintf(packageInstalledMetaInfoDIR, packageInstalledMetaInfoDIRCapacity, "%s/.uppm", packageInstalledRealDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (mkdir(packageInstalledMetaInfoDIR, S_IRWXU) != 0) {
        perror(packageInstalledMetaInfoDIR);
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    ret = uppm_record_installed_files(packageInstalledRealDIR);

    if (ret != XCPKG_OK) {
        return XCPKG_ERROR;
    }

    //////////////////////////////////////////////////////////////////////

    size_t receiptFilePathCapacity = packageInstalledMetaInfoDIRCapacity + 12U;
    char   receiptFilePath[receiptFilePathCapacity];

    ret = snprintf(receiptFilePath, receiptFilePathCapacity, "%s/receipt.yml", packageInstalledMetaInfoDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int receiptFD = open(receiptFilePath, O_CREAT | O_TRUNC | O_WRONLY, 0666);

    if (receiptFD == -1) {
        perror(receiptFilePath);
        return XCPKG_ERROR;
    }

    int formulaFD = open(formula->path, O_RDONLY);

    if (formulaFD == -1) {
        perror(formula->path);
        close(receiptFD);
        return XCPKG_ERROR;
    }

    dprintf(receiptFD, "pkgname: %s\n", packageName);

    char buf[1024];

    for (;;) {
        ssize_t readSize = read(formulaFD, buf, 1024);

        if (readSize == -1) {
            perror(NULL);
            close(formulaFD);
            close(receiptFD);
            return XCPKG_ERROR;
        }

        if (readSize == 0) {
            close(formulaFD);
            break;
        }

        ssize_t writeSize = write(receiptFD, buf, readSize);

        if (writeSize == -1) {
            perror(receiptFilePath);
            close(formulaFD);
            close(receiptFD);
            return XCPKG_ERROR;
        }

        if (writeSize != readSize) {
            fprintf(stderr, "not fully written to %s\n", receiptFilePath);
            close(formulaFD);
            close(receiptFD);
            return XCPKG_ERROR;
        }
    }

    dprintf(receiptFD, "\nsignature: %s\ntimestamp: %lu\n", UPPM_VERSION, time(NULL));

    close(receiptFD);

    if (chdir (packageInstalledRootDIR) != 0) {
        perror(packageInstalledRootDIR);
        return XCPKG_ERROR;
    }

    for (;;) {
        if (symlink(sessionID, packageName) == 0) {
            fprintf(stderr, "uppm package '%s' was successfully installed.\n", packageName);
            return XCPKG_OK;
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
                perror(packageInstalledRealDIR);
                return XCPKG_ERROR;
            }
        }
    }
}

int uppm_install(const char * uppmHomeDIR, const size_t uppmHomeDIRLength, const char * packageName, const bool verbose, const bool force) {
    UPPMFormula * formula = NULL;

    int ret = uppm_formula_lookup(uppmHomeDIR, uppmHomeDIRLength, packageName, &formula);

    if (ret != XCPKG_OK) {
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////

    char depPackageName[51];

    size_t i;

    const char * p = formula->dep_pkg;

    if (p == NULL) goto finally;

loop:
    if (p[0] == '\0') {
        goto finally;
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

        depPackageName[i] = p[i];
    }

    depPackageName[i] = '\0';

    //////////////////////////////////////////////////////////////////////////////

    fprintf(stderr, "depPackageName=%s\n", depPackageName);
    ret = uppm_install(uppmHomeDIR, uppmHomeDIRLength, depPackageName, verbose, force);

    if (ret != XCPKG_OK) {
        uppm_formula_free(formula);
        return ret;
    }

    //////////////////////////////////////////////////////////////////////////////

    p += i;

    if (p[0] == ' ') {
        p++;
        goto loop;
    }

finally:
    ret = uppm_install_internal(uppmHomeDIR, uppmHomeDIRLength, packageName, formula, verbose, force);

    uppm_formula_free(formula);

    return ret;
}

int uppm_install_the_given_packages(const char * uppmHomeDIR, const size_t uppmHomeDIRLength, const char * packageNames[], size_t size) {
    for (size_t i = 0; i < size; i++) {
        const char * packageName = packageNames[i];

        UPPMFormula * formula = NULL;

        int ret = uppm_formula_lookup(uppmHomeDIR, uppmHomeDIRLength, packageName, &formula);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }
    return 0;
}
