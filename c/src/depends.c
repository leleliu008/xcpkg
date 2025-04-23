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

static inline int string_append(char ** outP, size_t * outSize, size_t * outCapacity, const char * buf, size_t bufLength) {
    size_t  oldCapacity = (*outCapacity);
    size_t needCapacity = oldCapacity + bufLength + 1U;

    if (needCapacity > oldCapacity) {
        size_t newCapacity = needCapacity + 256U;

        char * p = (char*)realloc(*outP, newCapacity * sizeof(char));

        if (p == NULL) {
            free(*outP);
            return XCPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            (*outP) = p;
            (*outCapacity) = newCapacity;
            memset(&p[*outSize], 0, 256U + bufLength + 1U);
        }
    }

    strncat(*outP, buf, bufLength);
        
    (*outSize) += bufLength;

    return XCPKG_OK;
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

    char * p = NULL;
    size_t pSize = 0;
    size_t pCapacity = 0;

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

        size_t packageNameLength = strlen(packageName);

        if (needGenerateDotScript) {
            size_t bufCapacity = packageNameLength + 12U;
            char   buf[bufCapacity];

            ret = snprintf(buf, bufCapacity, "    \"%s\" -> {", packageName);

            if (ret < 0) {
                perror(NULL);
                xcpkg_formula_free(formula);
                return XCPKG_ERROR;
            }

            ret = string_append(&p, &pSize, &pCapacity, buf, ret);

            if (ret != XCPKG_OK) {
                xcpkg_formula_free(formula);
                goto finalize;
            }
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
                size_t bufLength = strlen(depPackageName) + 4U;
                char   buf[bufLength];

                ret = snprintf(buf, bufLength, " \"%s\"", depPackageName);

                if (ret < 0) {
                    perror(NULL);
                    xcpkg_formula_free(formula);
                    return XCPKG_ERROR;
                }

                ret = string_append(&p, &pSize, &pCapacity, buf, ret);

                if (ret != XCPKG_OK) {
                    xcpkg_formula_free(formula);
                    goto finalize;
                }
            } else {
                size_t bufCapacity = packageNameLength + strlen(depPackageName) + 10U;
                char   buf[bufCapacity];

                ret = snprintf(buf, bufCapacity, "\"%s\" -> \"%s\"\n", packageName, depPackageName);

                if (ret < 0) {
                    perror(NULL);
                    xcpkg_formula_free(formula);
                    return XCPKG_ERROR;
                }

                ret = string_append(&p, &pSize, &pCapacity, buf, ret);

                if (ret != XCPKG_OK) {
                    xcpkg_formula_free(formula);
                    goto finalize;
                }
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
            ret = string_append(&p, &pSize, &pCapacity, " }\n", 3);
        }

        xcpkg_formula_free(formula);
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

    if (ret != XCPKG_OK) {
        free(p);
        return ret;
    }

    if (pSize == 0U) {
        return XCPKG_OK;
    }

    //////////////////////////////////////////////////

    size_t stringBufferLength;
    size_t stringBufferCapacity = pSize + 14U;
    char   stringBuffer[stringBufferCapacity];

    if (needGenerateDotScript) {
        int ret = snprintf(stringBuffer, stringBufferCapacity, "digraph G {\n%s}", p);

        if (ret < 0) {
            perror(NULL);
            free(p);
            return XCPKG_ERROR;
        }

        stringBufferLength = ret;
    } else {
        strncpy(stringBuffer, p, pSize);
        stringBuffer[pSize] = '\0';
        stringBufferLength = pSize;
    }

    free(p);

    //////////////////////////////////////////////////

    switch (outputType) {
        case XCPKGDependsOutputType_D2 : return xcpkg_depends_output_d2 (packageName, outputPath, stringBuffer, stringBufferLength);
        case XCPKGDependsOutputType_DOT: return xcpkg_depends_output_dot(packageName, outputPath, stringBuffer, stringBufferLength);
        case XCPKGDependsOutputType_BOX: return xcpkg_depends_output_box(packageName, outputPath, stringBuffer, stringBufferLength);
        default:
            if (engine == XCPKGDependsOutputDiagramEngine_DOT) {
                return xcpkg_depends_output_via_dot(packageName, outputType, outputPath, stringBuffer, stringBufferLength);
            } else {
                return xcpkg_depends_output_via_d2 (packageName, outputType, outputPath, stringBuffer, stringBufferLength);
            }
    }
}
