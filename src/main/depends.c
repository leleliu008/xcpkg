#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg depends <PACKAGE-NAME> [-t <OUTPUT-TYPE>] [-e <ENGINE>] [-o <OUTPUT-PATH>]
 */
int xcpkg_main_depends(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s depends <PACKAGE-NAME>, <PACKAGE-NAME> is unspecified.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    if (argv[2][0] == '\0') {
        fprintf(stderr, "Usage: %s depends <PACKAGE-NAME>, <PACKAGE-NAME> should be a non-empty string.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    XCPKGDependsOutputType outputType = XCPKGDependsOutputType_BOX;
    XCPKGDependsOutputDiagramEngine engine = XCPKGDependsOutputDiagramEngine_DOT;

    const char * targetPlatformName = NULL;

    char * outputPath = NULL;

    bool verbose = false;

    for (int i = 3; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "-p") == 0) {
            targetPlatformName = argv[++i];

            if (targetPlatformName == NULL) {
                fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> is unspecified.\n");
                return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
            }

            if (targetPlatformName[0] == '\0') {
                fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            bool isSupported = false;

            for (int j = 0; supportedTargetPlatformNames[j] != NULL; j++) {
                if (strcmp(targetPlatformName, supportedTargetPlatformNames[j]) == 0) {
                    isSupported = true;
                    break;
                }
            }

            if (!isSupported) {
                LOG_ERROR2("unknown target platform name: ", targetPlatformName);
                return XCPKG_ERROR_ARG_IS_UNKNOWN;
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            const char * p = argv[++i];

            if (p == NULL) {
                fprintf(stderr, "-t <OUTPUT-TYPE>, <OUTPUT-TYPE> is unspecified.\n");
                return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
            }

            if (p[0] == '\0') {
                fprintf(stderr, "-t <OUTPUT-TYPE>, <OUTPUT-TYPE> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            if (strcmp(p, "d2") == 0) {
                outputType = XCPKGDependsOutputType_D2;
            } else if (strcmp(p, "dot") == 0) {
                outputType = XCPKGDependsOutputType_DOT;
            } else if (strcmp(p, "box") == 0) {
                outputType = XCPKGDependsOutputType_BOX;
            } else if (strcmp(p, "svg") == 0) {
                outputType = XCPKGDependsOutputType_SVG;
            } else if (strcmp(p, "png") == 0) {
                outputType = XCPKGDependsOutputType_PNG;
            } else {
                LOG_ERROR2("unknown output type: ", p);
                return XCPKG_ERROR;
            }
        } else if (strcmp(argv[i], "-e") == 0) {
            const char * p = argv[++i];

            if (p == NULL) {
                fprintf(stderr, "-e <ENGINE>, <ENGINE> is unspecified.\n");
                return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
            }

            if (p[0] == '\0') {
                fprintf(stderr, "-e <ENGINE>, <ENGINE> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }

            if (strcmp(p, "d2") == 0) {
                engine = XCPKGDependsOutputDiagramEngine_D2;
            } else if (strcmp(p, "dot") == 0) {
                engine = XCPKGDependsOutputDiagramEngine_DOT;
            } else {
                LOG_ERROR2("unknown engine: ", p);
                return XCPKG_ERROR_ARG_IS_UNKNOWN;
            }
         } else if (strcmp(argv[i], "-o") == 0) {
            outputPath = argv[++i];

            if (outputPath == NULL) {
                fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> is unspecified.\n");
                return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
            }

            if (outputPath[0] == '\0') {
                fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> should be a non-empty string.\n");
                return XCPKG_ERROR_ARG_IS_EMPTY;
            }
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    }

    int ret = xcpkg_depends(argv[2], targetPlatformName, outputType, outputPath, engine);

    if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s depends <PACKAGE-NAME>, <PACKAGE-NAME> does not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
        fprintf(stderr, "package '%s' is not available for target '%s'\n", argv[2], targetPlatformName);
    } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    return ret;
}
