#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "core/http.h"
#include "core/url.h"

#include "xcpkg.h"


static inline __attribute__((always_inline)) void string_buffer_append(char buf[], size_t * bufLengthP, const char * s) {
    size_t bufLength = (*bufLengthP);

    char * p = buf + bufLength;

    for (int i = 0; ; i++) {
        p[i] = s[i];

        if (p[i] == '\0') {
            (*bufLengthP) = bufLength + i;
            break;
        }
    }
}

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

    if (outputPath == NULL || outputPath[0] == '\0' || strcmp(outputPath, ".") == 0) {
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

    //printf("url=%s\n", url);

    if (outputPath == NULL) {
        int ret = http_fetch_to_stream(url, stdout, false, false);

        if (ret == -1) {
            return XCPKG_ERROR;
        }

        if (ret > 0) {
            return XCPKG_ERROR_NETWORK_BASE + ret;
        }

        return XCPKG_OK;
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

        ret = http_fetch_to_file(url, tmpFilePath, false, false);

        if (ret == -1) {
            perror(tmpFilePath);
            return XCPKG_ERROR;
        }

        if (ret > 0) {
            return XCPKG_ERROR_NETWORK_BASE + ret;
        } else {
            char outputFilePath[PATH_MAX];

            ret = get_output_file_path(outputFilePath, packageName, XCPKGDependsOutputType_BOX, outputPath, NULL);

            if (ret != XCPKG_OK) {
                return ret;
            }

            return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
        }
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

    if (needGenerateDotScript) {
        string_buffer_append(buf, &bufLength, "digraph G {\n");
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
            char ** p = (char**)realloc(packageNameArray, (packageNameArrayCapacity + 10U) * sizeof(char*));

            if (p == NULL) {
                free(packageName);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            memset(p + packageNameArrayCapacity, 0, 10);

            packageNameArray = p;
            packageNameArrayCapacity += 10;
        }

        packageNameArray[packageNameArraySize++] = packageName;

        ////////////////////////////////////////////////////////////////

        XCPKGFormula * formula = NULL;

        ret = xcpkg_formula_load(packageName, targetPlatformName, NULL, &formula);

        if (ret != XCPKG_OK) {
            puts(packageName);
            goto finalize;
        }

        if (formula->dep_pkg == NULL) {
            xcpkg_formula_free(formula);
            continue;
        }

        ////////////////////////////////////////////////////////////////

        if (needGenerateDotScript) {
            string_buffer_append(buf, &bufLength, "    \"");
            string_buffer_append(buf, &bufLength, packageName);
            string_buffer_append(buf, &bufLength, "\" -> {");
        }

        ////////////////////////////////////////////////////////////////

        size_t depPackageNamesLength = strlen(formula->dep_pkg);

        size_t depPackageNamesCopyLength = depPackageNamesLength + 1U;
        char   depPackageNamesCopy[depPackageNamesCopyLength];
        strncpy(depPackageNamesCopy, formula->dep_pkg, depPackageNamesCopyLength);

        char * depPackageName = strtok(depPackageNamesCopy, " ");

        while (depPackageName != NULL) {
            if (strcmp(depPackageName, packageName) == 0) {
                fprintf(stderr, "package '%s' depends itself.\n", packageName);
                xcpkg_formula_free(formula);
                ret = XCPKG_ERROR;
                goto finalize;
            }

            ////////////////////////////////////////////////////////////////

            if (needGenerateDotScript) {
                string_buffer_append(buf, &bufLength, " \"");
                string_buffer_append(buf, &bufLength, depPackageName);
                string_buffer_append(buf, &bufLength, "\"");
            } else {
                string_buffer_append(buf, &bufLength, "\"");
                string_buffer_append(buf, &bufLength, packageName);
                string_buffer_append(buf, &bufLength, "\" -> \"");
                string_buffer_append(buf, &bufLength, depPackageName);
                string_buffer_append(buf, &bufLength, "\"\n");
            }

            ////////////////////////////////////////////////////////////////

            if (packageNameStackSize == packageNameStackCapacity) {
                char ** p = (char**)realloc(packageNameStack, (packageNameStackCapacity + 10U) * sizeof(char*));

                if (p == NULL) {
                    xcpkg_formula_free(formula);
                    ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                    goto finalize;
                }

                memset(p + packageNameStackCapacity, 0, 10);

                packageNameStack = p;
                packageNameStackCapacity += 10;
            }

            char * p = strdup(depPackageName);

            if (p == NULL) {
                xcpkg_formula_free(formula);
                ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                goto finalize;
            }

            packageNameStack[packageNameStackSize] = p;
            packageNameStackSize++;

            depPackageName = strtok (NULL, " ");
        }

        if (needGenerateDotScript) {
            string_buffer_append(buf, &bufLength, " }\n");
        }

        xcpkg_formula_free(formula);
    }

    if (needGenerateDotScript) {
        string_buffer_append(buf, &bufLength, "}\n");
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
