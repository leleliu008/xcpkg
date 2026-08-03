#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/log.h"
#include "core/printenv.h"
#include "core/list-PATH.h"

#include "main.h"
#include "util.h"
#include "xcpkg.h"

/**
 *  xcpkg formula-repo-list
 */
int xcpkg_main_formula_repo_list(int argc, char* argv[]) {
    return xcpkg_formula_repo_list();
}

/**
 *  xcpkg util printenv
 */
int xcpkg_util_printenv(int argc, char* argv[]) {
    printenv();
    return XCPKG_OK;
}

/**
 *  xcpkg util list-PATH
 */
int xcpkg_util_list_PATH(int argc, char* argv[]) {
    return list_PATH();
}

typedef struct {
    const char * arg;
    int (*fn)(int argc, char* argv[]);
} XCPKGAction;

//invoked as 'xcpkg util <CMD> [ARGUMENT]...'
int xcpkg_main_util(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s %s <COMMAND> , <COMMAND> is unspecified.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (argv[2][0] == '\0') {
        fprintf(stderr, "Usage: %s %s <COMMAND> , <COMMAND> should be a non-empty string.\n", argv[0], argv[1]);
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    ///////////////////////////////////////////////////

    const XCPKGAction actions[] = {
        {"base16-encode",        xcpkg_util_base16_encode},
        {"base16-encode",        xcpkg_util_base16_decode},
        {"base64-encode",        xcpkg_util_base64_encode},
        {"base64-encode",        xcpkg_util_base64_decode},
        {"zlib-deflate",         xcpkg_util_zlib_deflate},
        {"zlib-inflate",         xcpkg_util_zlib_inflate},
        {"sha256sum",            xcpkg_util_sha256sum},
        {"printenv",             xcpkg_util_printenv},
        {"which",                xcpkg_util_which},
        {"list-PATH",            xcpkg_util_list_PATH},
        {"http-fetch",           xcpkg_util_http_fetch},
        {"git-sync",             xcpkg_util_git_sync},
        {"uncompress",           xcpkg_util_uncompress},
        {"mkdir-p",              xcpkg_util_mkdir_p},
        {"rm-rf",                xcpkg_util_rm_rf},
        {NULL, NULL}
    };

    for (size_t i = 0U; actions[i].arg != NULL; i++) {
        if (strcmp(argv[2], actions[i].arg) == 0) {
            return actions[i].fn(argc, argv);
        }
    }

    fprintf(stderr, "Usage: %s %s <COMMAND> , unknown <COMMAND>: %s\n", argv[0], argv[1], argv[2]);

    return XCPKG_ERROR_ARG_IS_UNKNOWN;
}

int main(int argc, char* argv[]) {
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

    ///////////////////////////////////////////////////

    int ret = xcpkg_setenv();

    if (ret == XCPKG_ERROR_ENV_HOME_NOT_SET) {
        fprintf(stderr, "%s\n", "HOME environment variable is not set.\n");
    } else if (ret == XCPKG_ERROR) {
        fprintf(stderr, "occurs error.\n");
    }

    if (ret != XCPKG_OK) {
        return ret;
    }

    ///////////////////////////////////////////////////

    const XCPKGAction actions[] = {
        {"about",        xcpkg_main_about},
        {"update",       xcpkg_main_update},
        {"search",       xcpkg_main_search},
        {"info",         xcpkg_main_info_available},
        {"show",         xcpkg_main_info_installed},
        {"depends",      xcpkg_main_depends},
        {"fetch",        xcpkg_main_fetch},
        {"install",      xcpkg_main_install},
        {"reinstall",    xcpkg_main_reinstall},
        {"uninstall",    xcpkg_main_uninstall},
        {"upgrade",      xcpkg_main_upgrade},
        {"cleanup",      xcpkg_main_cleanup},

        {"tree",         xcpkg_main_tree},
        {"logs",         xcpkg_main_logs},
        {"bundle",       xcpkg_main_bundle},
        {"xcinfo",       xcpkg_main_xcinfo},
        {"util",         xcpkg_main_util},

        {"ls-available", xcpkg_main_ls_available},
        {"ls-installed", xcpkg_main_ls_installed},
        {"ls-outdated",  xcpkg_main_ls_outdated},

        {"is-available", xcpkg_main_is_available},
        {"is-installed", xcpkg_main_is_installed},
        {"is-outdated",  xcpkg_main_is_outdated},

        {"completion",   xcpkg_main_completion},
        {"upgrade-self", xcpkg_main_upgrade_self},

        {"formula-cat",  xcpkg_main_formula_cat},
        {"formula-bat",  xcpkg_main_formula_bat},
        {"formula-set",  xcpkg_main_formula_set},
        {"formula-edit", xcpkg_main_formula_edit},
        {"formula-parse", xcpkg_main_formula_parse},

        {"formula-repo-list", xcpkg_main_formula_repo_list},
        {"formula-repo-init", xcpkg_main_formula_repo_init},
        {"formula-repo-add",  xcpkg_main_formula_repo_add},
        {"formula-repo-del",  xcpkg_main_formula_repo_del},
        {"formula-repo-sync", xcpkg_main_formula_repo_sync},
        {"formula-repo-conf", xcpkg_main_formula_repo_conf},
        {"formula-repo-info", xcpkg_main_formula_repo_info},

        {NULL, NULL}
    };

    for (size_t i = 0U; actions[i].arg != NULL; i++) {
        if (strcmp(argv[1], actions[i].arg) == 0) {
            return actions[i].fn(argc, argv);
        }
    }

    LOG_ERROR2("unknown argument: ", argv[1]);
    return XCPKG_ERROR_ARG_IS_UNKNOWN;
}
