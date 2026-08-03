#include <stdio.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg bundle <PACKAGE-SPEC> [<OUTPUT-DIR>][<OUTPUT-FILENAME-PREFIX>]<BUNDLE-TYPE> [--exclude <PATH>] [-K]
 */
int xcpkg_main_bundle(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s bundle <PACKAGE-SPEC> <BUNDLE-TYPE>, <PACKAGE-SPEC> is unspecified.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    }

    if (argv[2][0] == '\0') {
        fprintf(stderr, "Usage: %s bundle <PACKAGE-SPEC> <BUNDLE-TYPE>, <PACKAGE-SPEC> must be a non-empty string.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    if (argv[3] == NULL) {
        fprintf(stderr, "Usage: %s bundle <PACKAGE-SPEC> <BUNDLE-TYPE>, <BUNDLE-TYPE> is unspecified.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "Usage: %s bundle <PACKAGE-SPEC> <BUNDLE-TYPE>, <BUNDLE-TYPE> must be a non-empty string.\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    ArchiveType outputType = ArchiveType_tar_xz;

    if (argv[3][0] == '.') {
        if (strcmp(&argv[3][1], "zip") == 0) {
            outputType = ArchiveType_zip;
        } else if (strcmp(&argv[3][1], "7z") == 0) {
            outputType = ArchiveType_7z;
        } else if (strcmp(&argv[3][1], "tar.gz") == 0) {
            outputType = ArchiveType_tar_gz;
        } else if (strcmp(&argv[3][1], "tar.lz") == 0) {
            outputType = ArchiveType_tar_lz;
        } else if (strcmp(&argv[3][1], "tar.xz") == 0) {
            outputType = ArchiveType_tar_xz;
        } else if (strcmp(&argv[3][1], "tar.bz2") == 0) {
            outputType = ArchiveType_tar_bz2;
        } else {
            LOG_ERROR2("unknown bundle type: ", argv[3]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        }
    } else {
        LOG_ERROR2("unknown bundle type: ", argv[3]);
        return XCPKG_ERROR_ARG_IS_UNKNOWN;
    }

    char * outputPath = NULL;

    bool verbose = false;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
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

    const char * packageName = NULL;

    const char * platformSpec = NULL;

    char buf[51];

    int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> does not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <TARGET-SPEC> does not match pattern A-B-C\n", argv[0], argv[1]);
    }

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (platformSpec == NULL) {
        platformSpec = buf;
    }

    ret = xcpkg_bundle(packageName, platformSpec, outputType, outputPath, verbose);

    if (ret == XCPKG_ERROR_ARG_IS_NULL) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [-t tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [-t tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
    } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
        fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [-t tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> does not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
    } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
        fprintf(stderr, "package '%s' is not installed.\n", argv[2]);
    } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
        fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    return ret;
}
