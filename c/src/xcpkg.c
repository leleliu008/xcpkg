#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/stat.h>

#include "core/log.h"

#include "xcpkg.h"

int xcpkg_available_info2(const XCPKGFormula * formula, const char * packageName, const char * targetPlatformName, const char * key);

int xcpkg_main(int argc, char* argv[]) {
    if (argc == 1) {
        xcpkg_help();
        return XCPKG_OK;
    }

    if ((strcmp(argv[1], "-h") == 0) || (strcmp(argv[1], "--help") == 0)) {
        xcpkg_help();
        return XCPKG_OK;
    }

    if ((strcmp(argv[1], "-V") == 0) || (strcmp(argv[1], "--version") == 0)) {
        printf("%s\n", XCPKG_VERSION_STRING);
        return XCPKG_OK;
    }

    if (strcmp(argv[1], "sysinfo") == 0) {
        return xcpkg_sysinfo();
    }

    if (strcmp(argv[1], "about") == 0) {
        bool verbose = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
                break;
            }
        }

        int ret = xcpkg_about(verbose);

        if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    int ret = xcpkg_setenv_SSL_CERT_FILE();

    if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (strcmp(argv[1], "setup") == 0) {
        bool verbose = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
                break;
            }
        }

        return xcpkg_setup(verbose);
    }

    if (strcmp(argv[1], "integrate") == 0) {
        bool verbose = false;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
                break;
            }
        }

        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s integrate <zsh|bash|fish>\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_NULL;
        } else if (strcmp(argv[2], "zsh") == 0) {
            return xcpkg_integrate_zsh_completion (NULL, verbose);
        } else if (strcmp(argv[2], "bash") == 0) {
            return xcpkg_integrate_bash_completion(NULL, verbose);
        } else if (strcmp(argv[2], "fish") == 0) {
            return xcpkg_integrate_fish_completion(NULL, verbose);
        } else {
            LOG_ERROR2("unrecognized argument: ", argv[2]);
            return XCPKG_ERROR_ARG_IS_INVALID;
        }
    }

    if (strcmp(argv[1], "upgrade-self") == 0) {
        bool verbose = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
                break;
            }
        }

        return xcpkg_upgrade_self(verbose);
    }

    if (strcmp(argv[1], "cleanup") == 0) {
        bool verbose = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
                break;
            }
        }

        return xcpkg_cleanup(verbose);
    }

    if (strcmp(argv[1], "gen-url-transform-sample") == 0) {
        return xcpkg_generate_url_transform_sample();
    }

    if (strcmp(argv[1], "update") == 0) {
        int ret = xcpkg_formula_repo_list_update();

        if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "search") == 0) {
        const char * targetPlatformName = NULL;

        char verbose = false;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "-p") == 0) {
                targetPlatformName = argv[++i];

                if (targetPlatformName == NULL) {
                    fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
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
                    LOG_ERROR2("unsupported target platform name: ", targetPlatformName);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        int ret = xcpkg_search(argv[2], targetPlatformName, verbose);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s search <KEYWORD>, <KEYWORD> is not given.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s search <KEYWORD>, <KEYWORD> is empty string.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "info-available") == 0) {
        const char * targetPlatformName = NULL;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-p") == 0) {
                targetPlatformName = argv[++i];

                if (targetPlatformName == NULL) {
                    fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
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
                    LOG_ERROR2("unsupported target platform name: ", targetPlatformName);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            }
        }

        int ret = xcpkg_available_info(argv[2], targetPlatformName, argv[3]);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not given.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is empty string.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_ARG_IS_UNKNOWN) {
            fprintf(stderr, "Usage: %s info <PACKAGE-NAME> [KEY], unrecognized KEY: %s\n", argv[0], argv[3]);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package '%s' is not available for target '%s'\n", argv[2], targetPlatformName);
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

    if (strcmp(argv[1], "info-installed") == 0) {
        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        }

        if (platformSpec == NULL) {
            platformSpec = buf;
        }

        ret = xcpkg_installed_info(packageName, platformSpec, argv[3]);

        if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package '%s' is not available for target '%s'\n", argv[2], platformSpec);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package '%s' is not installed.\n", argv[2]);
        } else if (ret == XCPKG_ERROR_ARG_IS_UNKNOWN) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME> [KEY], unrecognized KEY: %s\n", argv[0], argv[3]);
        } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "depends") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s depends <PACKAGE-NAME>, <PACKAGE-NAME> is unspecified.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_NULL;
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
                    fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
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
                    LOG_ERROR2("unsupported target platform name: ", targetPlatformName);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            } else if (strcmp(argv[i], "-t") == 0) {
                const char * type = argv[i + 1];

                if (type == NULL) {
                    fprintf(stderr, "-t <OUTPUT-TYPE>, <OUTPUT-TYPE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }

                if (type[0] == '\0') {
                    fprintf(stderr, "-o <OUTPUT-TYPE>, <OUTPUT-TYPE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }

                if (strcmp(type, "d2") == 0) {
                    outputType = XCPKGDependsOutputType_D2;
                    i++;
                } else if (strcmp(type, "dot") == 0) {
                    outputType = XCPKGDependsOutputType_DOT;
                    i++;
                } else if (strcmp(type, "box") == 0) {
                    outputType = XCPKGDependsOutputType_BOX;
                    i++;
                } else if (strcmp(type, "svg") == 0) {
                    outputType = XCPKGDependsOutputType_SVG;
                    i++;
                } else if (strcmp(type, "png") == 0) {
                    outputType = XCPKGDependsOutputType_PNG;
                    i++;
                } else {
                    LOG_ERROR2("unsupported type: ", type);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
             } else if (strcmp(argv[i], "-o") == 0) {
                outputPath = argv[i + 1];

                if (outputPath == NULL) {
                    fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }

                if (outputPath[0] == '\0') {
                    fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }

                i++;
            } else if (strncmp(argv[i], "--engine=", 9) == 0) {
                const char * engineName = &argv[i][9];

                if (engineName == NULL) {
                    fprintf(stderr, "--engine=<ENGINE>, <ENGINE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }

                if (engineName[0] == '\0') {
                    fprintf(stderr, "--engine=<ENGINE>, <ENGINE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }

                if (strcmp(engineName, "d2") == 0) {
                    engine = XCPKGDependsOutputDiagramEngine_D2;
                } else if (strcmp(engineName, "dot") == 0) {
                    engine = XCPKGDependsOutputDiagramEngine_DOT;
                } else {
                    LOG_ERROR2("unsupported diagram engine: ", engineName);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        int ret = xcpkg_depends(argv[2], targetPlatformName, outputType, outputPath, engine);

        if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
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


    if (strcmp(argv[1], "fetch") == 0) {
        const char * targetPlatformName = NULL;

        bool verbose = false;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "-p") == 0) {
                targetPlatformName = argv[++i];

                if (targetPlatformName == NULL) {
                    fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
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
                    LOG_ERROR2("unsupported target platform name: ", targetPlatformName);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        int ret = xcpkg_fetch(argv[2], targetPlatformName, verbose);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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

    if (strcmp(argv[1], "install") == 0) {
        XCPKGInstallOptions installOptions = {0};

        installOptions.logLevel = XCPKGLogLevel_normal;

        int packageIndexArray[argc];
        int packageIndexArraySize = 0;

        char * targetPlatformSpec = NULL;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-x") == 0) {
                installOptions.xtrace = true;
            } else if (strcmp(argv[i], "-q") == 0) {
                installOptions.logLevel = XCPKGLogLevel_silent;
            } else if (strcmp(argv[i], "-v") == 0) {
                installOptions.logLevel = XCPKGLogLevel_verbose;
                installOptions.verbose_net = true;
                installOptions.verbose_env = true;
                installOptions.verbose_cc  = true;
                installOptions.verbose_ld  = true;
            } else if (strcmp(argv[i], "-v:net") == 0) {
                installOptions.logLevel = XCPKGLogLevel_normal;
                installOptions.verbose_net = true;
            } else if (strcmp(argv[i], "-v:env") == 0) {
                installOptions.logLevel = XCPKGLogLevel_normal;
                installOptions.verbose_env = true;
            } else if (strcmp(argv[i], "-v:cc") == 0) {
                installOptions.logLevel = XCPKGLogLevel_normal;
                installOptions.verbose_cc = true;
            } else if (strcmp(argv[i], "-v:ld") == 0) {
                installOptions.logLevel = XCPKGLogLevel_normal;
                installOptions.verbose_ld = true;
            } else if (strcmp(argv[i], "--dry-run") == 0) {
                installOptions.dryrun = true;
            } else if (strcmp(argv[i], "-K") == 0) {
                installOptions.keepSessionDIR = true;
            } else if (strcmp(argv[i], "--export-compile-commands-json") == 0) {
                installOptions.exportCompileCommandsJson = true;
            } else if (strcmp(argv[i], "--enable-ccache") == 0) {
                installOptions.enableCcache = true;
            } else if (strcmp(argv[i], "--enable-bear") == 0) {
                installOptions.enableBear = true;
            } else if (strcmp(argv[i], "--profile=debug") == 0) {
                installOptions.buildType = XCPKGBuildProfile_debug;
            } else if (strcmp(argv[i], "--profile=release") == 0) {
                installOptions.buildType = XCPKGBuildProfile_release;
            } else if (strcmp(argv[i], "--prefer-static") == 0) {
                installOptions.createMostlyStaticallyLinkedExecutables = true;
            } else if (strncmp(argv[i], "--jobs=", 7) == 0) {
                char * jobsStr = &argv[i][7];

                if (jobsStr[0] == '\0') {
                    LOG_ERROR1("--jobs=<N> , <N> should be a non-empty string");
                    return XCPKG_ERROR;
                } else {
                    int j = 0;

                    for (;;) {
                        char c = jobsStr[j];

                        if (c == '\0') {
                            break;
                        }

                        if ((c >= '0') && (c <= '9')) {
                            j++;
                        } else {
                            LOG_ERROR1("--jobs=<N> , <N> should be a integer.");
                            return XCPKG_ERROR;
                        }
                    }
                }

                installOptions.parallelJobsCount = atoi(jobsStr);
            } else if (strncmp(argv[i], "--target=", 9) == 0) {
                targetPlatformSpec = &argv[i][9];

                if (targetPlatformSpec[0] == '\0') {
                    fprintf(stderr, "--target=<TARGET-PLATFORM-SPEC>, <TARGET-PLATFORM-SPEC> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }
            } else if (argv[i][0] == '-') {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            } else {
            puts(argv[i]);
                packageIndexArray[packageIndexArraySize] = i;
                packageIndexArraySize++;
            }
        }

        if (packageIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        for (int i = 0; i < packageIndexArraySize; i++) {
            const char * package = argv[packageIndexArray[i]];

            const char * packageName = NULL;

            const char * platformSpec = NULL;

            char buf[51];

            int ret = xcpkg_inspect_package(package, NULL, &packageName, &platformSpec, buf);

            printf("packageName=%s\nplatformSpec=%s\nbuf=%s\n", packageName, platformSpec, buf);
            if (ret == XCPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0], argv[1]);
            } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0], argv[1]);
            } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
            }

            if (ret != XCPKG_OK) {
                return ret;
            }

            if (platformSpec == NULL) {
                platformSpec = buf;
            }

            ret = xcpkg_install(packageName, platformSpec, &installOptions);

            if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                fprintf(stderr, "package '%s' is not available.\n", packageName);
            } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                fprintf(stderr, "package '%s' is not installed.\n", packageName);
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

    if (strcmp(argv[1], "reinstall") == 0) {
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
            } else if (strcmp(argv[i], "--export-compile-commands-json") == 0) {
                installOptions.exportCompileCommandsJson = true;
            } else if (strcmp(argv[i], "--enable-ccache") == 0) {
                installOptions.enableCcache = true;
            } else if (strcmp(argv[i], "--enable-bear") == 0) {
                installOptions.enableBear = true;
            } else if (strcmp(argv[i], "--profile=debug") == 0) {
                installOptions.buildType = XCPKGBuildProfile_debug;
            } else if (strcmp(argv[i], "--profile=release") == 0) {
                installOptions.buildType = XCPKGBuildProfile_release;
            } else if (strcmp(argv[i], "--prefer-static") == 0) {
                installOptions.createMostlyStaticallyLinkedExecutables = true;
            } else if (strncmp(argv[i], "--jobs=", 7) == 0) {
                char * jobsStr = &argv[i][7];

                if (jobsStr[0] == '\0') {
                    LOG_ERROR1("--jobs=<N> , <N> should be a non-empty string");
                    return XCPKG_ERROR;
                } else {
                    int j = 0;

                    for (;;) {
                        char c = jobsStr[j];

                        if (c == '\0') {
                            break;
                        }

                        if ((c >= '0') && (c <= '9')) {
                            j++;
                        } else {
                            LOG_ERROR1("--jobs=<N> , <N> should be a integer.");
                            return XCPKG_ERROR;
                        }
                    }
                }

                installOptions.parallelJobsCount = atoi(jobsStr);
            } else if (strncmp(argv[i], "--target=", 9) == 0) {
                targetPlatformSpec = &argv[i][9];

                if (targetPlatformSpec[0] == '\0') {
                    fprintf(stderr, "--target=<TARGET-PLATFORM-SPEC>, <TARGET-PLATFORM-SPEC> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }
            } else if (argv[i][0] == '-') {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            } else {
                packageIndexArray[packageIndexArraySize] = i;
                packageIndexArraySize++;
            }
        }

        if (packageIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        for (int i = 0; i < packageIndexArraySize; i++) {
            const char * package = argv[packageIndexArray[i]];

            const char * packageName = NULL;

            const char * platformSpec = NULL;

            char buf[51];

            int ret = xcpkg_inspect_package(package, NULL, &packageName, &platformSpec, buf);

            if (ret == XCPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
            } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
            } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
            }

            if (platformSpec == NULL) {
                platformSpec = buf;
            }

            ret = xcpkg_reinstall(packageName, platformSpec, &installOptions);

            if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
                fprintf(stderr, "package '%s' is not available.\n", packageName);
            } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                fprintf(stderr, "package '%s' is not installed.\n", packageName);
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

    if (strcmp(argv[1], "upgrade") == 0) {
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
            } else if (strcmp(argv[i], "--export-compile-commands-json") == 0) {
                installOptions.exportCompileCommandsJson = true;
            } else if (strcmp(argv[i], "--enable-ccache") == 0) {
                installOptions.enableCcache = true;
            } else if (strcmp(argv[i], "--enable-bear") == 0) {
                installOptions.enableBear = true;
            } else if (strcmp(argv[i], "--profile=debug") == 0) {
                installOptions.buildType = XCPKGBuildProfile_debug;
            } else if (strcmp(argv[i], "--profile=release") == 0) {
                installOptions.buildType = XCPKGBuildProfile_release;
            } else if (strcmp(argv[i], "--prefer-static") == 0) {
                installOptions.createMostlyStaticallyLinkedExecutables = true;
            } else if (strncmp(argv[i], "--jobs=", 7) == 0) {
                char * jobsStr = &argv[i][7];

                if (jobsStr[0] == '\0') {
                    LOG_ERROR1("--jobs=<N> , <N> should be a non-empty string");
                    return XCPKG_ERROR;
                } else {
                    int j = 0;

                    for (;;) {
                        char c = jobsStr[j];

                        if (c == '\0') {
                            break;
                        }

                        if ((c >= '0') && (c <= '9')) {
                            j++;
                        } else {
                            LOG_ERROR1("--jobs=<N> , <N> should be a integer.");
                            return XCPKG_ERROR;
                        }
                    }
                }

                installOptions.parallelJobsCount = atoi(jobsStr);
            } else if (strncmp(argv[i], "--target=", 9) == 0) {
                targetPlatformSpec = &argv[i][9];

                if (targetPlatformSpec[0] == '\0') {
                    fprintf(stderr, "--target=<TARGET-PLATFORM-SPEC>, <TARGET-PLATFORM-SPEC> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }
            } else if (argv[i][0] == '-') {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
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

            int ret = xcpkg_inspect_package(package, NULL, &packageName, &platformSpec, buf);

            if (ret == XCPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
            } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
            } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
            }

            if (platformSpec == NULL) {
                platformSpec = buf;
            }

            ret = xcpkg_upgrade(packageName, platformSpec, &installOptions);

            if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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

    if (strcmp(argv[1], "uninstall") == 0) {
        int packageIndexArray[argc];
        int packageIndexArraySize = 0;

        bool verbose = false;

        char * targetPlatformSpec = NULL;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strncmp(argv[i], "--target=", 9) == 0) {
                targetPlatformSpec = &argv[i][9];

                if (targetPlatformSpec[0] == '\0') {
                    fprintf(stderr, "--target=<TARGET-PLATFORM-SPEC>, <TARGET-PLATFORM-SPEC> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }
            } else {
                packageIndexArray[packageIndexArraySize] = i;
                packageIndexArraySize++;
            }
        }

        if (packageIndexArraySize == 0) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>..., <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        for (int i = 0; i < packageIndexArraySize; i++) {
            const char * package = argv[packageIndexArray[i]];

            const char * packageName = NULL;

            const char * platformSpec = NULL;

            char buf[51];

            int ret = xcpkg_inspect_package(package, NULL, &packageName, &platformSpec, buf);

            if (ret == XCPKG_ERROR_ARG_IS_NULL) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
            } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
            } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
                fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
            }

            if (platformSpec == NULL) {
                platformSpec = buf;
            }

            ret = xcpkg_uninstall(packageName, platformSpec, verbose);

            if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
                fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
            } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
                fprintf(stderr, "package '%s' is not installed.\n", packageName);
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

    if (strcmp(argv[1], "ls-available") == 0) {
        const char * targetPlatformName = NULL;

        bool verbose = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "-p") == 0) {
                targetPlatformName = argv[++i];

                if (targetPlatformName == NULL) {
                    fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
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
                    LOG_ERROR2("unsupported target platform name: ", targetPlatformName);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        int ret = xcpkg_show_the_available_packages(targetPlatformName, verbose);

        if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "ls-installed") == 0) {
        bool verbose = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        int ret = xcpkg_list_the_installed_packages(NULL, verbose);

        if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "ls-outdated") == 0) {
        bool verbose = false;

        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        int ret = xcpkg_list_the__outdated_packages(NULL, verbose);

        if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "is-available") == 0) {
        const char * targetPlatformName = NULL;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-p") == 0) {
                targetPlatformName = argv[++i];

                if (targetPlatformName == NULL) {
                    fprintf(stderr, "-p <TARGET-PLATFORM-NAME>, <TARGET-PLATFORM-NAME> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
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
                    LOG_ERROR2("unsupported target platform name: ", targetPlatformName);
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        int ret = xcpkg_check_if_the_given_package_is_available(argv[2], targetPlatformName);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not given.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is empty string.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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

    if (strcmp(argv[1], "is-installed") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s is-installed <PACKAGE-SPEC>, <PACKAGE-SPEC> is unspecified.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_NULL;
        } else if (argv[2][0] == '\0') {
            fprintf(stderr, "Usage: %s is-installed <PACKAGE-SPEC>, <PACKAGE-SPEC> must be a non-empty string.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_EMPTY;
        }

        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        }

        if (platformSpec == NULL) {
            platformSpec = buf;
        }

        ret = xcpkg_check_if_the_given_package_is_installed(packageName, platformSpec);

        if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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

    if (strcmp(argv[1], "is-outdated") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s is-installed <PACKAGE-SPEC>, <PACKAGE-SPEC> is unspecified.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_NULL;
        } else if (argv[2][0] == '\0') {
            fprintf(stderr, "Usage: %s is-installed <PACKAGE-SPEC>, <PACKAGE-SPEC> must be a non-empty string.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_EMPTY;
        }

        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s info-installed <PACKAGE-NAME|PACKAGE-SPEC> [KEY], <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        }

        if (platformSpec == NULL) {
            platformSpec = buf;
        }

        ret = xcpkg_check_if_the_given_package_is_outdated(packageName, platformSpec);

        if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_AVAILABLE) {
            fprintf(stderr, "package [%s] is not available.\n", argv[2]);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package [%s] is not installed.\n", argv[2]);
        } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "formula-repo-list") == 0) {
        return xcpkg_formula_repo_list_printf();
    }

    if (strcmp(argv[1], "formula-repo-add") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        int pinned  = 0;
        int enabled = 1;

        const char * branch = NULL;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "--pin") == 0) {
                pinned = 1;
            } else if (strcmp(argv[i], "--unpin") == 0) {
                pinned = 0;
            } else if (strcmp(argv[i], "--enable") == 0) {
                enabled = 1;
            } else if (strcmp(argv[i], "--disable") == 0) {
                enabled = 0;
            } else if (strncmp(argv[i], "--branch=", 9) == 0) {
                if (argv[i][9] == '\0') {
                    fprintf(stderr, "--branch=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                } else {
                    branch = &argv[i][9];
                }
            } else {
                fprintf(stderr, "unrecognized option: %s\n", argv[i]);
                return XCPKG_ERROR_ARG_IS_UNKNOWN;
            }
        }

        return xcpkg_formula_repo_add(argv[2], argv[3], branch, pinned, enabled);
    }

    if (strcmp(argv[1], "formula-repo-init") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> <FORMULA-REPO-URL> [--branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        int pinned  = 1;
        int enabled = 1;

        char * branch = NULL;

        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "--pin") == 0) {
                pinned = 1;
            } else if (strcmp(argv[i], "--unpin") == 0) {
                pinned = 0;
            } else if (strcmp(argv[i], "--enable") == 0) {
                enabled = 1;
            } else if (strcmp(argv[i], "--disable") == 0) {
                enabled = 0;
            } else if (strncmp(argv[i], "--branch=", 9) == 0) {
                if (argv[i][9] == '\0') {
                    fprintf(stderr, "--branch=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                } else {
                    branch = &argv[i][9];
                }
            } else {
                fprintf(stderr, "unrecognized option: %s\n", argv[i]);
                return XCPKG_ERROR_ARG_IS_UNKNOWN;
            }
        }

        return xcpkg_formula_repo_create(argv[2], argv[3], branch, pinned, enabled);
    }

    if (strcmp(argv[1], "formula-repo-del") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        return xcpkg_formula_repo_remove(argv[2]);
    }

    if (strcmp(argv[1], "formula-repo-sync") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        return xcpkg_formula_repo_sync_(argv[2]);
    }

    if (strcmp(argv[1], "formula-repo-conf") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> [--url=VALUE --branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[3] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME> [--url=VALUE --branch=VALUE --pin/--unpin --enable/--disable]\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        int pinned  = -1;
        int enabled = -1;

        const char * branch = NULL;
        const char * url = NULL;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "--pin") == 0) {
                pinned = 1;
            } else if (strcmp(argv[i], "--unpin") == 0) {
                pinned = 0;
            } else if (strcmp(argv[i], "--enable") == 0) {
                enabled = 1;
            } else if (strcmp(argv[i], "--disable") == 0) {
                enabled = 0;
            } else if (strncmp(argv[i], "--url=", 6) == 0) {
                if (argv[i][6] == '\0') {
                    fprintf(stderr, "--url=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                } else {
                    url = &argv[i][6];
                }
            } else if (strncmp(argv[i], "--branch=", 9) == 0) {
                if (argv[i][9] == '\0') {
                    fprintf(stderr, "--branch=<VALUE>, <VALUE> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                } else {
                    branch = &argv[i][9];
                }
            } else {
                fprintf(stderr, "unrecognized option: %s\n", argv[i]);
                return XCPKG_ERROR_ARG_IS_UNKNOWN;
            }
        }

        return xcpkg_formula_repo_config(argv[2], url, branch, pinned, enabled);
    }

    if (strcmp(argv[1], "formula-repo-info") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-REPO-NAME>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        return xcpkg_formula_repo_info_(argv[2]);
    }

    if (strcmp(argv[1], "formula-view") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[2][0] == '\0') {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_EMPTY;
        }

        bool raw = false;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "--raw") == 0) {
                raw = true;
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        return xcpkg_formula_view(argv[2], NULL, raw);
    }

    if (strcmp(argv[1], "formula-edit") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[2][0] == '\0') {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_EMPTY;
        }

        const char * editor = NULL;

        for (int i = 3; i < argc; i++) {
            if (strncmp(argv[i], "--editor=", 9) == 0) {
                editor = &argv[i][9];
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        return xcpkg_formula_edit(argv[2], NULL, editor);
    }

    if (strcmp(argv[1], "formula-parse") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s %s <FORMULA-FILEPATH>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_NULL;
        }

        if (argv[2][0] == '\0') {
            fprintf(stderr, "Usage: %s %s <FORMULA-FILEPATH>\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_EMPTY;
        }

        int slashIndex = -1;

        for (int i = 0; argv[2][i] != '\0' ; i++) {
            if (argv[2][i] == '/') {
                slashIndex = i;
            }
        }

        const char * p = argv[2] + slashIndex + 1;

        int dotIndex = -1;

        for (int i = 0; p[i] != '\0' ; i++) {
            if (p[i] == '.') {
                dotIndex = i;
            }
        }

        if (dotIndex == -1) {
            fprintf(stderr, "Usage: %s %s <FORMULA-FILEPATH>, <FORMULA-FILEPATH> must ends with .yml\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_INVALID;
        }

        if (strcmp(p + dotIndex + 1, "yml") != 0) {
            fprintf(stderr, "Usage: %s %s <FORMULA-FILEPATH>, <FORMULA-FILEPATH> must ends with .yml\n", argv[0], argv[1]);
            return XCPKG_ERROR_ARG_IS_INVALID;
        }

        char packageName[dotIndex];

        strncpy(packageName, p, dotIndex);
        packageName[dotIndex] = '\0';

        XCPKGFormula * formula = NULL;

        int ret = xcpkg_formula_load(packageName, NULL, argv[2], &formula);

        if (ret != XCPKG_OK) {
            return ret;
        }

        return xcpkg_available_info2(formula, packageName, NULL, argv[3]);
    }

    if (strcmp(argv[1], "tree") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-SPEC> [KEY], <PACKAGE-SPEC> is unspecified.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_NULL;
        } else if (argv[2][0] == '\0') {
            fprintf(stderr, "Usage: %s tree <PACKAGE-SPEC> [KEY], <PACKAGE-SPEC> must be a non-empty string.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_EMPTY;
        }

        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

        printf("packageName=%s\nplatformSpec=%s\nbuf=%s\n", packageName, platformSpec, buf);
        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        }

        if (ret != XCPKG_OK) {
            return ret;
        }

        if (platformSpec == NULL) {
            platformSpec = buf;
        }

        ret = xcpkg_tree(packageName, platformSpec, argc - 3, &argv[3]);

        if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s tree <PACKAGE-NAME> [KEY], <PACKAGE-NAME> is not match pattern %s\n", argv[0], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PACKAGE_NOT_INSTALLED) {
            fprintf(stderr, "package '%s' is not installed.\n", packageName);
        } else if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
            fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR_ENV_PATH_NOT_SET) {
            fprintf(stderr, "%s\n", "PATH environment variable is not set.\n");
        } else if (ret == XCPKG_ERROR) {
            fprintf(stderr, "occurs error.\n");
        }

        return ret;
    }

    if (strcmp(argv[1], "logs") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s logs <PACKAGE-SPEC>, <PACKAGE-SPEC> is unspecified.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_NULL;
        } else if (argv[2][0] == '\0') {
            fprintf(stderr, "Usage: %s logs <PACKAGE-SPEC>, <PACKAGE-SPEC> must be a non-empty string.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_EMPTY;
        }

        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

        printf("packageName=%s\nplatformSpec=%s\nbuf=%s\n", packageName, platformSpec, buf);
        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        }

        if (ret != XCPKG_OK) {
            return ret;
        }

        if (platformSpec == NULL) {
            platformSpec = buf;
        }

        ret = xcpkg_logs(packageName, platformSpec);

        if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME>, <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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

    if (strcmp(argv[1], "bundle") == 0) {
        if (argv[2] == NULL) {
            fprintf(stderr, "Usage: %s bundle <PACKAGE-SPEC> <BUNDLE-TYPE>, <PACKAGE-SPEC> is unspecified.\n", argv[0]);
            return XCPKG_ERROR_ARG_IS_NULL;
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

        if (strcmp(argv[3], "zip") == 0) {
            outputType = ArchiveType_zip;
        } else if (strcmp(argv[3], "tar.gz") == 0) {
            outputType = ArchiveType_tar_gz;
        } else if (strcmp(argv[3], "tar.lz") == 0) {
            outputType = ArchiveType_tar_lz;
        } else if (strcmp(argv[3], "tar.xz") == 0) {
            outputType = ArchiveType_tar_xz;
        } else if (strcmp(argv[3], "tar.bz2") == 0) {
            outputType = ArchiveType_tar_bz2;
        } else {
            LOG_ERROR2("unsupported bundle type: ", argv[3]);
            return XCPKG_ERROR_ARG_IS_INVALID;
        }

        char * outputPath = NULL;

        bool verbose = false;

        for (int i = 3; i < argc; i++) {
            if (strcmp(argv[i], "-v") == 0) {
                verbose = true;
            } else if (strcmp(argv[i], "-o") == 0) {
                outputPath = argv[i + 1];

                if (outputPath == NULL) {
                    fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_INVALID;
                }

                if (outputPath[0] == '\0') {
                    fprintf(stderr, "-o <OUTPUT-PATH>, <OUTPUT-PATH> should be a non-empty string.\n");
                    return XCPKG_ERROR_ARG_IS_EMPTY;
                }

                i++;
            } else {
                LOG_ERROR2("unrecognized argument: ", argv[i]);
                return XCPKG_ERROR_ARG_IS_INVALID;
            }
        }

        const char * packageName = NULL;

        const char * platformSpec = NULL;

        char buf[51];

        int ret = xcpkg_inspect_package(argv[2], NULL, &packageName, &platformSpec, buf);

        printf("packageName=%s\nplatformSpec=%s\nbuf=%s\n", packageName, platformSpec, buf);
        if (ret == XCPKG_ERROR_ARG_IS_NULL) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not given.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_EMPTY) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is empty string.\n", argv[0], argv[1]);
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
        } else if (ret == XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME|PACKAGE-SPEC>, <PACKAGE-NAME|PACKAGE-SPEC> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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
        } else if (ret == XCPKG_ERROR_ARG_IS_INVALID) {
            fprintf(stderr, "Usage: %s %s <PACKAGE-NAME> [-t tar.gz|tar.xz|tar.bz2|zip], <PACKAGE-NAME> is not match pattern %s\n", argv[0], argv[1], XCPKG_PACKAGE_NAME_PATTERN);
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

    if (strcmp(argv[1], "xcinfo") == 0) {
        XCPKGToolChain toolchain = {0};

        ret = xcpkg_toolchain_find(&toolchain);

        if (ret != XCPKG_OK) {
            return ret;
        }

        xcpkg_toolchain_dump(&toolchain);
        xcpkg_toolchain_free(&toolchain);

        return XCPKG_OK;
    }

    if (strcmp(argv[1], "util") == 0) {
        return xcpkg_util(argc, argv);
    }

    LOG_ERROR2("unrecognized action: ", argv[1]);
    return XCPKG_ERROR_ARG_IS_UNKNOWN;
}

int main(int argc, char* argv[]) {
    return xcpkg_main(argc, argv);
}
