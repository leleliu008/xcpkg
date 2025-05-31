#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "core/urlcode.h"

#include "xcpkg.h"


static inline int get_output_file_path(char outputFilePath[PATH_MAX], const char * packageName, XCPKGDependsOutputType outputType, const char * outputPath, const char * owd) {
    const char * suffix;

    switch (outputType) {
        case XCPKGDependsOutputType_D2 : suffix = "d2" ; break;
        case XCPKGDependsOutputType_DOT: suffix = "dot"; break;
        case XCPKGDependsOutputType_BOX: suffix = "box"; break;
        case XCPKGDependsOutputType_SVG: suffix = "svg"; break;
        case XCPKGDependsOutputType_PNG: suffix = "png"; break;
    }

    ////////////////////////////////////////////////////////////////

    size_t defaultFileNameCapacity = strlen(packageName) + 20U;
    char   defaultFileName[defaultFileNameCapacity];

    int ret = snprintf(defaultFileName, defaultFileNameCapacity, "%s-dependencies.%s", packageName, suffix);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    if (owd == NULL) {
        owd = ".";
    }

    ////////////////////////////////////////////////////////////////

    if (outputPath == NULL || outputPath[0] == '\0' || (outputPath[0] == '.' && outputPath[1] == '\0')) {
        ret = snprintf(outputFilePath, strlen(owd) + defaultFileNameCapacity, "%s/%s", owd, defaultFileName);
    } else {
        size_t outputPathLength = strlen(outputPath);

        if (outputPath[0] == '/') {
            if (outputPath[outputPathLength - 1] == '/') {
                ret = snprintf(outputFilePath, outputPathLength + defaultFileNameCapacity, "%s%s", outputPath, defaultFileName);
            } else {
                ret = 0;
                strncpy(outputFilePath, outputPath, outputPathLength);
                outputFilePath[outputPathLength] = '\0';
            }
        } else {
            if (outputPath[outputPathLength - 1] == '/') {
                ret = snprintf(outputFilePath, strlen(owd) + outputPathLength + defaultFileNameCapacity, "%s/%s%s", owd, outputPath, defaultFileName);
            } else {
                ret = snprintf(outputFilePath, strlen(owd) + outputPathLength + 2U, "%s/%s", owd, outputPath);
            }
        }
    }

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

static inline int xcpkg_tmpfile(char fp[PATH_MAX], const char * p, const size_t pLength) {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    ret = snprintf(fp, sessionDIRLength + 18U, "%s/dependencies.dot", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    return xcpkg_write_file(fp, p, pLength);
}

static inline int xcpkg_depends_output_d2(const char * packageName, const char * outputPath, const char * d2ScriptStr, const size_t d2ScriptStrLength) {
    if (outputPath == NULL) {
        printf("%s\n", d2ScriptStr);
        return XCPKG_OK;
    }

    ////////////////////////////////////////////////////////////////

    char tmpFilePath[PATH_MAX];

    int ret = xcpkg_tmpfile(tmpFilePath, d2ScriptStr, d2ScriptStrLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    char outputFilePath[PATH_MAX];

    ret = get_output_file_path(outputFilePath, packageName, XCPKGDependsOutputType_D2, outputPath, NULL);

    if (ret != XCPKG_OK) {
        return ret;
    }

    return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
}

static inline int xcpkg_depends_output_dot(const char * packageName, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    if (outputPath == NULL) {
        printf("%s\n", dotScriptStr);
        return XCPKG_OK;
    }

    ////////////////////////////////////////////////////////////////

    char tmpFilePath[PATH_MAX];

    int ret = xcpkg_tmpfile(tmpFilePath, dotScriptStr, dotScriptStrLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    char outputFilePath[PATH_MAX];

    ret = get_output_file_path(outputFilePath, packageName, XCPKGDependsOutputType_DOT, outputPath, NULL);

    if (ret != XCPKG_OK) {
        return ret;
    }

    return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
}

static inline int xcpkg_depends_output_box(const char * packageName, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    size_t urlEncodedBufLength = 3 * dotScriptStrLength + 1U;
    char   urlEncodedBuf[urlEncodedBufLength];
    memset(urlEncodedBuf, 0, urlEncodedBufLength);

    size_t urlEncodedRealLength = 0;

    if (url_encode(urlEncodedBuf, &urlEncodedRealLength, (unsigned char *)dotScriptStr, dotScriptStrLength, true) != 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    size_t urlCapacity = urlEncodedRealLength + 66U;
    char   url[urlCapacity];

    int ret = snprintf(url, urlCapacity, "https://dot-to-ascii.ggerganov.com/dot-to-ascii.php?boxart=1&src=%s", urlEncodedBuf);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    if (outputPath == NULL) {
        return xcpkg_http_fetch_to_stream(url, stdout, false, false);
    } else {
        char   sessionDIR[PATH_MAX];
        size_t sessionDIRLength;

        int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        size_t tmpFilePathLength = sessionDIRLength + 18U;
        char   tmpFilePath[tmpFilePathLength];

        ret = snprintf(tmpFilePath, tmpFilePathLength, "%s/dependencies.box", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        ret = xcpkg_http_fetch_to_file(url, tmpFilePath, false, false);

        if (ret != XCPKG_OK) {
            return ret;
        }

        char outputFilePath[PATH_MAX];

        ret = get_output_file_path(outputFilePath, packageName, XCPKGDependsOutputType_BOX, outputPath, NULL);

        if (ret != XCPKG_OK) {
            return ret;
        }

        return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
    }
}

static inline int xcpkg_depends_output_via_dot(const char * packageName, XCPKGDependsOutputType outputType, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    char cwd[PATH_MAX];

    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    char dotCommandPath[PATH_MAX];

    int ret = xcpkg_get_command_path_of_uppm_package("dot_static", "dot_static", dotCommandPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    char tmpFilePath[PATH_MAX];

    ret = xcpkg_tmpfile(tmpFilePath, dotScriptStr, dotScriptStrLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    const char * const type = (outputType == XCPKGDependsOutputType_PNG) ? "png" : "svg";

    if (outputPath == NULL) {
        execl (dotCommandPath, dotCommandPath, "-T", type, tmpFilePath, NULL);
        perror(dotCommandPath);
        return XCPKG_ERROR;
    } else {
        char outFilePath[PATH_MAX];

        ret = get_output_file_path(outFilePath, packageName, outputType, outputPath, cwd);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        //fprintf(stderr, "%s -v -T %s -o %s %s\n", dotCommandPath, type, outFilePath, tmpFilePath);

        execl (dotCommandPath, dotCommandPath, "-T", type, "-o", outFilePath, tmpFilePath, NULL);
        perror(dotCommandPath);
        return XCPKG_ERROR;
    }
}

static inline int xcpkg_depends_output_via_d2(const char * packageName, XCPKGDependsOutputType outputType, const char * outputPath, const char * d2ScriptStr, const size_t d2ScriptStrLength) {
    char cwd[PATH_MAX];

    if (getcwd(cwd, PATH_MAX) == NULL) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    char d2CommandPath[PATH_MAX];

    int ret = xcpkg_get_command_path_of_uppm_package("d2", "d2", d2CommandPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    char tmpFilePath[PATH_MAX];

    ret = xcpkg_tmpfile(tmpFilePath, d2ScriptStr, d2ScriptStrLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    if (outputPath == NULL) {
        execl (d2CommandPath, d2CommandPath, tmpFilePath, "-", NULL);
        perror(d2CommandPath);
        return XCPKG_ERROR;
    } else {
        char outFilePath[PATH_MAX];

        ret = get_output_file_path(outFilePath, packageName, outputType, outputPath, cwd);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        execl (d2CommandPath, d2CommandPath, tmpFilePath, outFilePath, NULL);
        perror(d2CommandPath);
        return XCPKG_ERROR;
    }
}

int xcpkg_depends(const char * packageName, const char * targetPlatformName, XCPKGDependsOutputType outputType, const char * outputPath, XCPKGDependsOutputDiagramEngine engine) {
    bool needGenerateDotScript;

    switch (outputType) {
        case XCPKGDependsOutputType_D2 : needGenerateDotScript = false; break;
        case XCPKGDependsOutputType_DOT: needGenerateDotScript = true;  break;
        case XCPKGDependsOutputType_BOX: needGenerateDotScript = true;  break;
        default:
            if (engine == XCPKGDependsOutputDiagramEngine_DOT) {
                needGenerateDotScript = true;
            } else {
                needGenerateDotScript = false;
            }
    }

    ////////////////////////////////////////////////////////////////

    int ret = XCPKG_OK;

    ////////////////////////////////////////////////////////////////

    char   buf[4096]; buf[0] = '\0';
    size_t bufLength = 0U;

    char * p = buf;

    if (needGenerateDotScript) {
        const char * const s = "digraph G {\n";

        for (size_t i = 0U; ; i++) {
            p[i] = s[i];

            if (p[i] == '\0') {
                p += i;
                bufLength = i;
                break;
            }
        }
    }

    ////////////////////////////////////////////////////////////////

    size_t  packageNameArrayCapacity = 0;
    size_t  packageNameArraySize     = 0;
    char ** packageNameArray         = NULL;

    ////////////////////////////////////////////////////////////////

    size_t  packageNameStackCapacity = 1;
    size_t  packageNameStackSize     = 1;
    char ** packageNameStack         = (char**)calloc(1, sizeof(char*));

    if (packageNameStack == NULL) {
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    packageNameStack[0] = strdup(packageName);

    if (packageNameStack[0] == NULL) {
        free(packageNameStack);
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    ////////////////////////////////////////////////////////////////

    while (packageNameStackSize > 0) {
        char * packageName = packageNameStack[--packageNameStackSize];
        packageNameStack[packageNameStackSize] = NULL;

        ////////////////////////////////////////////////////////////////

        bool hasHandled = false;

        for (size_t i = 0; i < packageNameArraySize; i++) {
            if (strcmp(packageNameArray[i], packageName) == 0) {
                free(packageName);
                hasHandled = true;
                break;
            }
        }

        if (hasHandled) continue;

        ////////////////////////////////////////////////////////////////

        if (packageNameArraySize == packageNameArrayCapacity) {
            char ** q = (char**)realloc(packageNameArray, (packageNameArrayCapacity + 10U) * sizeof(char*));

            if (q == NULL) {
                free(packageName);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            memset(q + packageNameArrayCapacity, 0, 10);

            packageNameArray = q;
            packageNameArrayCapacity += 10;
        }

        packageNameArray[packageNameArraySize++] = packageName;

        ////////////////////////////////////////////////////////////////

        XCPKGFormula * formula = NULL;

        ret = xcpkg_formula_load(packageName, targetPlatformName, NULL, &formula);

        if (ret != XCPKG_OK) {
            goto finalize;
        }

        ////////////////////////////////////////////////////////////////

        char * q = formula->dep_pkg;

        if (q == NULL || q[0] == '\0') {
            xcpkg_formula_free(formula);
            continue;
        }

        ////////////////////////////////////////////////////////////////

        for(;;) {
            if (q[0] == '\0') break;

            if (q[0] == '\n' || q[0] == ' ') {
                q++;
                continue;
            } else {
                break;
            }
        }

        ////////////////////////////////////////////////////////////////

        if (q[0] == '\0') {
            xcpkg_formula_free(formula);
            continue;
        }

        ////////////////////////////////////////////////////////////////

        size_t i;

        for (i = 0U; q[i] != '\0'; i++);

        char s[i + 1];

        for (i = 0U; q[i] != '\0'; i++) {
            s[i] = q[i];
        }

        s[i] = '\0';

        q = s;

        xcpkg_formula_free(formula);

        ////////////////////////////////////////////////////////////////

        if (needGenerateDotScript) {
            p[0] = ' ';
            p[1] = ' ';
            p[2] = ' ';
            p[3] = ' ';
            p[4] = '"';
            p += 5;
            bufLength += 5;

            ///////////////////////////////

            for (size_t i = 0U; ; i++) {
                p[i] = packageName[i];

                if (p[i] == '\0') {
                    p += i;
                    bufLength += i;
                    break;
                }
            }

            ///////////////////////////////

            p[0] = '"';
            p[1] = ' ';
            p[2] = '-';
            p[3] = '>';
            p[4] = ' ';
            p[5] = '{';
            p += 6;
            bufLength += 6;
        }

        ////////////////////////////////////////////////////////////////

        while (q[0] != '\0') {
            if (q[0] == ' ' || q[0] == '\n') {
                q++;
                continue;
            }

            bool b = false;

            for (i = 0U; q[i] != '\0'; i++) {
                if (q[i] == ' ') {
                    q[i] = '\0';
                    b = true;
                    break;
                }
            }

            if (strcmp(q, packageName) == 0) {
                fprintf(stderr, "package '%s' depends itself.\n", packageName);
                ret = XCPKG_ERROR;
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            if (needGenerateDotScript) {
                p[0] = ' ';
                p[1] = '"';
                p += 2;
                bufLength += 2;

                for (size_t j = 0U; j < i; j++) {
                    p[j] = q[j];
                }

                p += i;
                bufLength += i;

                p[0] = '"';
                p++;
                bufLength++;
            } else {
                p[0] = '"';
                p++;
                bufLength++;

                for (size_t j = 0U; ; j++) {
                    p[j] = packageName[j];

                    if (p[j] == '\0') {
                        p += j;
                        bufLength += j;
                        break;
                    }
                }

                p[0] = '"';
                p[1] = ' ';
                p[2] = '-';
                p[3] = '>';
                p[4] = ' ';
                p[5] = '"';
                p += 6;
                bufLength += 6;

                for (size_t j = 0U; j < i; j++) {
                    p[j] = q[j];
                }

                p += i;
                bufLength += i;

                p[0] = '"';
                p[1] = '\n';
                p += 2;
                bufLength += 2;
            }

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** q = (char**)realloc(packageNameStack, (packageNameStackCapacity + 10U) * sizeof(char*));

                if (q == NULL) {
                    ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                memset(q + packageNameStackCapacity, 0, 10);

                packageNameStack = q;
                packageNameStackCapacity += 10;
            }

            char * x = (char*)malloc(i + 1);

            if (x == NULL) {
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            for (size_t j = 0U; j < i; j++) {
                x[j] = q[j];
            }

            x[i] = '\0';

            packageNameStack[packageNameStackSize] = x;
            packageNameStackSize++;

            if (b) {
                q += i + 1;
            } else {
                q += i;
            }
        }

        if (needGenerateDotScript) {
            p[0] = ' ';
            p[1] = '}';
            p[2] = '\n';
            p[3] = '\0';
            p += 3;
            bufLength += 3;
        }
    }

    if (needGenerateDotScript) {
        p[0] = '}';
        p[1] = '\n';
        p[2] = '\0';
        bufLength += 2;
    }

finalize:
    for (size_t i = 0U; i < packageNameStackSize; i++) {
        free(packageNameStack[i]);
        packageNameStack[i] = NULL;
    }

    free(packageNameStack);
    packageNameStack = NULL;

    //////////////////////////////////////////////////

    for (size_t i = 0U; i < packageNameArraySize; i++) {
        free(packageNameArray[i]);
        packageNameArray[i] = NULL;
    }

    free(packageNameArray);
    packageNameArray = NULL;

    //////////////////////////////////////////////////

    if (bufLength == 0U) {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////

    switch (outputType) {
        case XCPKGDependsOutputType_D2 : return xcpkg_depends_output_d2 (packageName, outputPath, buf, bufLength);
        case XCPKGDependsOutputType_DOT: return xcpkg_depends_output_dot(packageName, outputPath, buf, bufLength);
        case XCPKGDependsOutputType_BOX: return xcpkg_depends_output_box(packageName, outputPath, buf, bufLength);
        default:
            if (engine == XCPKGDependsOutputDiagramEngine_DOT) {
                return xcpkg_depends_output_via_dot(packageName, outputType, outputPath, buf, bufLength);
            } else {
                return xcpkg_depends_output_via_d2 (packageName, outputType, outputPath, buf, bufLength);
            }
    }
}
