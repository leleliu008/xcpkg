#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include "core/http.h"
#include "core/url.h"

#include "xcpkg.h"

static int string_append(char ** outP, size_t * outSize, size_t * outCapacity, const char * buf, size_t bufLength) {
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

static int get_output_file_path(char outputFilePath[PATH_MAX], const char * packageName, XCPKGDependsOutputType outputType, const char * outputPath) {
    const char * outputFileNameSuffix;

    switch (outputType) {
        case XCPKGDependsOutputType_D2 : outputFileNameSuffix = "d2" ; break;
        case XCPKGDependsOutputType_DOT: outputFileNameSuffix = "dot"; break;
        case XCPKGDependsOutputType_BOX: outputFileNameSuffix = "box"; break;
        case XCPKGDependsOutputType_SVG: outputFileNameSuffix = "svg"; break;
        case XCPKGDependsOutputType_PNG: outputFileNameSuffix = "png"; break;
    }

    int ret;

    if (outputPath == NULL || outputPath[0] == '\0') {
        ret = snprintf(outputFilePath, strlen(packageName) + 20U, "%s-dependencies.%s", packageName, outputFileNameSuffix);
    } else {
        size_t outputPathLength = strlen(outputPath);

        if (strcmp(outputPath, ".") == 0) {
           ret = snprintf(outputFilePath, strlen(packageName) + 20U, "%s-dependencies.%s", packageName, outputFileNameSuffix);
        } else if (strcmp(outputPath, "..") == 0) {
           ret = snprintf(outputFilePath, strlen(packageName) + 23U, "../%s-dependencies.%s", packageName, outputFileNameSuffix);
        } else if (outputPath[outputPathLength - 1] == '/') {
           ret = snprintf(outputFilePath, strlen(outputPath) + strlen(packageName) + 20U, "%s%s-dependencies.%s", outputPath, packageName, outputFileNameSuffix);
        } else {
            ret = 0;
            size_t n = strlen(outputPath);
            strncpy(outputFilePath, outputPath, n);
            outputFilePath[n] = '\0';
        }
    }

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    return XCPKG_OK;
}

static int xcpkg_depends_output_d2(const char * packageName, const char * outputPath, const char * d2ScriptStr, const size_t d2ScriptStrLength) {
    if (outputPath == NULL) {
        printf("%s\n", d2ScriptStr);
        return XCPKG_OK;
    } else {
        char   sessionDIR[PATH_MAX];
        size_t sessionDIRLength;

        int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        size_t tmpFilePathCapacity = sessionDIRLength + 18U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.d2", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        const int fd = open(tmpFilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

        if (fd == -1) {
            perror(tmpFilePath);
            return XCPKG_ERROR;
        }

        ret = write(fd, d2ScriptStr, d2ScriptStrLength);

        if (ret == -1) {
            perror(tmpFilePath);
            close(fd);
            return XCPKG_ERROR;
        }

        close(fd);

        if ((size_t)ret != d2ScriptStrLength) {
            fprintf(stderr, "not fully written: %s\n", tmpFilePath);
            return XCPKG_ERROR;
        }

        ////////////////////////////////////////////////////////////////

        char outputFilePath[PATH_MAX];

        ret = get_output_file_path(outputFilePath, packageName, XCPKGDependsOutputType_D2, outputPath);

        if (ret != XCPKG_OK) {
            return ret;
        }

        return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
    }
}


static int xcpkg_depends_output_dot(const char * packageName, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    if (outputPath == NULL) {
        printf("%s\n", dotScriptStr);
        return XCPKG_OK;
    } else {
        char   sessionDIR[PATH_MAX];
        size_t sessionDIRLength;

        int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ////////////////////////////////////////////////////////////////

        size_t tmpFilePathCapacity = sessionDIRLength + 18U;
        char   tmpFilePath[tmpFilePathCapacity];

        ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.dot", sessionDIR);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        int fd = open(tmpFilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

        if (fd == -1) {
            perror(tmpFilePath);
            return XCPKG_ERROR;
        }

        ret = write(fd, dotScriptStr, dotScriptStrLength);

        if (ret == -1) {
            perror(tmpFilePath);
            close(fd);
            return XCPKG_ERROR;
        }

        close(fd);

        if ((size_t)ret != dotScriptStrLength) {
            fprintf(stderr, "not fully written: %s\n", tmpFilePath);
            return XCPKG_ERROR;
        }

        ////////////////////////////////////////////////////////////////

        char outputFilePath[PATH_MAX];

        ret = get_output_file_path(outputFilePath, packageName, XCPKGDependsOutputType_DOT, outputPath);

        if (ret != XCPKG_OK) {
            return ret;
        }

        return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
    }
}

static int xcpkg_depends_output_box(const char * packageName, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
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

            ret = get_output_file_path(outputFilePath, packageName, XCPKGDependsOutputType_BOX, outputPath);

            if (ret != XCPKG_OK) {
                return ret;
            }

            return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
        }
    }
}

static int xcpkg_depends_output_via_dot(const char * packageName, XCPKGDependsOutputType outputType, const char * outputPath, const char * dotScriptStr, const size_t dotScriptStrLength) {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    size_t dotFilePathCapacity = sessionDIRLength + 18U;
    char   dotFilePath[dotFilePathCapacity];

    ret = snprintf(dotFilePath, dotFilePathCapacity, "%s/dependencies.dot", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int fd = open(dotFilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

    if (fd == -1) {
        perror(dotFilePath);
        return XCPKG_ERROR;
    }

    ret = write(fd, dotScriptStr, dotScriptStrLength);

    if (ret == -1) {
        perror(dotFilePath);
        close(fd);
        return XCPKG_ERROR;
    }

    close(fd);

    if ((size_t)ret != dotScriptStrLength) {
        fprintf(stderr, "not fully written: %s\n", dotFilePath);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + 18U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.tmp", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    const char * const str = "/uppm/installed/dot_static/bin/dot_static";

    size_t dotCommandPathCapacity = xcpkgHomeDIRLength + strlen(str) + sizeof(char);
    char   dotCommandPath[dotCommandPathCapacity];

    ret = snprintf(dotCommandPath, dotCommandPathCapacity, "%s%s", xcpkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    ret = xcpkg_fork_exec2(5, dotCommandPath, (outputType == XCPKGDependsOutputType_PNG) ? "-Tpng" : "-Tsvg", "-o", tmpFilePath, dotFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    char outputFilePath[PATH_MAX];

    ret = get_output_file_path(outputFilePath, packageName, outputType, outputPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
}

static int xcpkg_depends_output_via_d2(const char * packageName, XCPKGDependsOutputType outputType, const char * outputPath, const char * d2ScriptStr, const size_t d2ScriptStrLength) {
    char   sessionDIR[PATH_MAX];
    size_t sessionDIRLength;

    int ret = xcpkg_session_dir(sessionDIR, &sessionDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    ////////////////////////////////////////////////////////////////

    size_t d2FilePathCapacity = sessionDIRLength + 18U;
    char   d2FilePath[d2FilePathCapacity];

    ret = snprintf(d2FilePath, d2FilePathCapacity, "%s/dependencies.d2", sessionDIR);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    int fd = open(d2FilePath, O_CREAT | O_WRONLY | O_TRUNC, 0666);

    if (fd == -1) {
        perror(d2FilePath);
        return XCPKG_ERROR;
    }

    ret = write(fd, d2ScriptStr, d2ScriptStrLength);

    if (ret == -1) {
        perror(d2FilePath);
        close(fd);
        return XCPKG_ERROR;
    }

    close(fd);

    if ((size_t)ret != d2ScriptStrLength) {
        fprintf(stderr, "not fully written: %s\n", d2FilePath);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    size_t tmpFilePathCapacity = sessionDIRLength + 18U;
    char   tmpFilePath[tmpFilePathCapacity];

    ret = snprintf(tmpFilePath, tmpFilePathCapacity, "%s/dependencies.%s", sessionDIR, (outputType == XCPKGDependsOutputType_PNG) ? "png" : "svg");

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    const char * const str = "/uppm/installed/d2/bin/d2";

    size_t d2CommandPathCapacity = xcpkgHomeDIRLength + strlen(str) + sizeof(char);
    char   d2CommandPath[d2CommandPathCapacity];

    ret = snprintf(d2CommandPath, d2CommandPathCapacity, "%s%s", xcpkgHomeDIR, str);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    ////////////////////////////////////////////////////////////////

    ret = xcpkg_fork_exec2(3, d2CommandPath, d2FilePath, tmpFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    char outputFilePath[PATH_MAX];

    ret = get_output_file_path(outputFilePath, packageName, outputType, outputPath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    return xcpkg_rename_or_copy_file(tmpFilePath, outputFilePath);
}

int xcpkg_depends(const char * packageName, const char * targetPlatformName, XCPKGDependsOutputType outputType, const char * outputPath, XCPKGDependsOutputDiagramEngine engine) {
    bool needGenerateDotScript;

    switch (outputType) {
        case XCPKGDependsOutputType_D2 : needGenerateDotScript = false;
        case XCPKGDependsOutputType_DOT: needGenerateDotScript = true;
        case XCPKGDependsOutputType_BOX: needGenerateDotScript = true;
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
