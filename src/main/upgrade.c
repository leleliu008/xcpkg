#include <stdio.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

int xcpkg_main_upgrade(int argc, char* argv[]) {
    XCPKGInstallOptions installOptions = {0};

    installOptions.logLevel = XCPKGLogLevel_normal;

    int packageIndexArray[argc];
    int packageIndexArraySize = 0;

    char * targetPlatformSpec = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-q") == 0) {
            installOptions.logLevel = XCPKGLogLevel_silent;
        } else if (strcmp(argv[i], "-v") == 0) {
            installOptions.logLevel = XCPKGLogLevel_verbose;
        } else if (strcmp(argv[i], "-vv") == 0) {
            installOptions.logLevel = XCPKGLogLevel_very_verbose;
            installOptions.verbose_net = true;
            installOptions.verbose_env = true;
            installOptions.verbose_cc  = true;
            installOptions.verbose_ld  = true;
        } else if (strcmp(argv[i], "-v:net") == 0) {
            installOptions.logLevel = XCPKGLogLevel_verbose;
            installOptions.verbose_net = true;
        } else if (strcmp(argv[i], "-v:env") == 0) {
            installOptions.logLevel = XCPKGLogLevel_verbose;
            installOptions.verbose_env = true;
        } else if (strcmp(argv[i], "-v:cc") == 0) {
            installOptions.logLevel = XCPKGLogLevel_verbose;
            installOptions.verbose_cc = true;
        } else if (strcmp(argv[i], "-v:ld") == 0) {
            installOptions.logLevel = XCPKGLogLevel_verbose;
            installOptions.verbose_ld = true;
        } else if (strcmp(argv[i], "--dry-run") == 0) {
            installOptions.dryrun = true;
        } else if (strcmp(argv[i], "-K") == 0) {
            installOptions.keepSessionDIR = true;
        } else if (strcmp(argv[i], "-E") == 0) {
            installOptions.exportCompileCommandsJson = true;
        } else if (strcmp(argv[i], "--enable-ccache") == 0) {
            installOptions.enableCcache = true;
        } else if (strcmp(argv[i], "--enable-bear") == 0) {
            installOptions.enableBear = true;
        } else if (strcmp(argv[i], "--prefer-shared") == 0) {
            installOptions.linkSharedLibs = true;
        } else if (strcmp(argv[i], "-j") == 0) {
            const char * p = argv[++i];

            if (p == NULL) {
                LOG_ERROR1("-j <N> , <N> is unspecified.");
                return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
            }

            if (p[0] == '\0') {
                LOG_ERROR1("-j <N> , <N> should be a non-empty string.");
                return XCPKG_ERROR;
            }

            for (int j = 0; ; j++) {
                if (p[j] == '\0') {
                    break;
                }

                if ((p[j] < '0') || (p[j] > '9')) {
                    LOG_ERROR1("-j <N> , <N> should be an integer.");
                    return XCPKG_ERROR;
                }
            }

            installOptions.parallelJobsCount = atoi(p);
        } else if (strncmp(argv[i], "--target=", 9) == 0) {
            targetPlatformSpec = &argv[i][9];

            if (targetPlatformSpec[0] == '\0') {
                fprintf(stderr, "--target=<TARGET-PLATFORM-SPEC>, <TARGET-PLATFORM-SPEC> should be a non-empty string.\n");
                return XCPKG_ERROR;
            }
        } else if (strncmp(argv[i], "--profile=", 10) == 0) {
            const char * p = &argv[i][10];

            if (p[0] == '\0') {
                fprintf(stderr, "--profile=<PROFILE>, <PROFILE> should be a non-empty string.\n");
                return XCPKG_ERROR;
            }

            if (strcmp(p, "debug") == 0) {
                installOptions.profile = XCPKGBuildProfile_debug;
            } else if (strcmp(p, "release") == 0) {
                installOptions.profile = XCPKGBuildProfile_release;
            } else {
                fprintf(stderr, "--profile=<PROFILE>, <PROFILE> should be either debug or release.\n");
                return XCPKG_ERROR;
            }
        } else if (argv[i][0] == '-') {
            LOG_ERROR2("unknown argument: ", argv[i]);
            return XCPKG_ERROR_ARG_IS_UNKNOWN;
        } else {
            packageIndexArray[packageIndexArraySize] = i;
            packageIndexArraySize++;
        }
    }

    for (int i = 0; i < packageIndexArraySize; i++) {
        const char * package = argv[packageIndexArray[i]];

        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(package, targetPlatformSpec, &packageName, &platformSpec, buf);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s upgrade <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s upgrade <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
            fprintf(stderr, "Usage: %s upgrade <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> does not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s upgrade <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <TARGET-SPEC> does not match pattern A-B-C\n", argv[0]);
        }

        if (platformSpec == NULL) {
            platformSpec = buf;
        }

        ret = xcpkg_upgrade(packageName, platformSpec, &installOptions);

        if (ret == XCPKG_ERROR_PACKAGE_NAME_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> does not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package '%s' is not available.\n", packageName);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package '%s' is not installed.\n", packageName);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_OUTDATED) {
            fprintf(stderr, "package '%s' is not outdated.\n", packageName);
        } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    return XCPKG_OK;
}
