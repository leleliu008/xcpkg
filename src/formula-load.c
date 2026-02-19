#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include <yaml.h>

#include "core/regex/regex.h"

#include "xcpkg.h"

static inline __attribute__((always_inline)) void string_buffer_append(char buf[], size_t * bufLengthP, const char * s) {
    size_t n = (*bufLengthP);

    char * p = buf + n;

    if (n != 0U) {
        n++;
        p[0] = ' ';
        p++;
    }

    for (size_t i = 0U; ; i++) {
        p[i] = s[i];

        if (p[i] == '\0') {
            (*bufLengthP) = n + i;
            break;
        }
    }
}

typedef enum {
    FORMULA_KEY_CODE_unknown,

    FORMULA_KEY_CODE_pkgtype,
    FORMULA_KEY_CODE_summary,
    FORMULA_KEY_CODE_version,
    FORMULA_KEY_CODE_license,

    FORMULA_KEY_CODE_web_url,

    FORMULA_KEY_CODE_git_url,
    FORMULA_KEY_CODE_git_uri,
    FORMULA_KEY_CODE_git_sha,
    FORMULA_KEY_CODE_git_ref,
    FORMULA_KEY_CODE_git_nth,

    FORMULA_KEY_CODE_src_url,
    FORMULA_KEY_CODE_src_uri,
    FORMULA_KEY_CODE_src_sha,

    FORMULA_KEY_CODE_fix_url,
    FORMULA_KEY_CODE_fix_uri,
    FORMULA_KEY_CODE_fix_sha,
    FORMULA_KEY_CODE_fix_opt,

    FORMULA_KEY_CODE_res_url,
    FORMULA_KEY_CODE_res_uri,
    FORMULA_KEY_CODE_res_sha,

    FORMULA_KEY_CODE_dep_pkg,
    FORMULA_KEY_CODE_dep_lib,
    FORMULA_KEY_CODE_dep_upp,
    FORMULA_KEY_CODE_dep_pip,
    FORMULA_KEY_CODE_dep_plm,

    FORMULA_KEY_CODE_bsystem,
    FORMULA_KEY_CODE_bscript,
    FORMULA_KEY_CODE_binbstd,

    FORMULA_KEY_CODE_dofetch,
    FORMULA_KEY_CODE_do12345,
    FORMULA_KEY_CODE_dopatch,
    FORMULA_KEY_CODE_prepare,
    FORMULA_KEY_CODE_install,
    FORMULA_KEY_CODE_dotweak,
    FORMULA_KEY_CODE_bindenv,
    FORMULA_KEY_CODE_caveats,

    FORMULA_KEY_CODE_symlink,
    FORMULA_KEY_CODE_movable,
    FORMULA_KEY_CODE_ltoable,
    FORMULA_KEY_CODE_mslable,

    FORMULA_KEY_CODE_ppflags,
    FORMULA_KEY_CODE_ccflags,
    FORMULA_KEY_CODE_xxflags,
    FORMULA_KEY_CODE_ldflags,

    FORMULA_KEY_CODE_patches,
    FORMULA_KEY_CODE_reslist,

    FORMULA_KEY_CODE_parallel,
} XCPKGFormulaKeyCode;

void xcpkg_formula_dump(XCPKGFormula * formula) {
    if (formula == NULL) {
        return;
    }

    switch (formula->pkgtype) {
        case XCPKGPkgType_exe: printf("pkgtype: exe\n"); break;
        case XCPKGPkgType_lib: printf("pkgtype: lib\n"); break;
    }

    printf("summary: %s\n", formula->summary);
    printf("version: %s\n", formula->version);
    printf("license: %s\n", formula->license);

    printf("web-url: %s\n", formula->web_url);

    printf("git_url: %s\n", formula->git_url);
    printf("git_uri: %s\n", formula->git_uri);
    printf("git_sha: %s\n", formula->git_sha);
    printf("git_ref: %s\n", formula->git_ref);
    printf("git_nth: %zu\n", formula->git_nth);

    printf("src_url: %s\n", formula->src_url);
    printf("src_uri: %s\n", formula->src_uri);
    printf("src_sha: %s\n", formula->src_sha);

    printf("fix_url: %s\n", formula->fix_url);
    printf("fix_uri: %s\n", formula->fix_uri);
    printf("fix_sha: %s\n", formula->fix_sha);
    printf("fix_opt: %s\n", formula->fix_opt);

    printf("res_url: %s\n", formula->res_url);
    printf("res_uri: %s\n", formula->res_uri);
    printf("res_sha: %s\n", formula->res_sha);

    printf("dep_pkg: %s\n", formula->dep_pkg);
    printf("dep_lib: %s\n", formula->dep_lib);
    printf("dep_upp: %s\n", formula->dep_upp);
    printf("dep_pip: %s\n", formula->dep_pip);
    printf("dep_plm: %s\n", formula->dep_plm);

    printf("bsystem: %s\n", formula->bsystem);
    printf("bscript: %s\n", formula->bscript);
    printf("binbstd: %d\n", formula->binbstd);

    printf("dofetch: %s\n", formula->dofetch);
    printf("do12345: %s\n", formula->do12345);
    printf("dopatch: %s\n", formula->dopatch);
    printf("prepare: %s\n", formula->prepare);
    printf("install: %s\n", formula->install);
    printf("dotweak: %s\n", formula->dotweak);

    printf("bindenv: %s\n", formula->bindenv);
    printf("caveats: %s\n", formula->caveats);

    printf("symlink: %d\n", formula->symlink);
    printf("ltoable: %d\n", formula->ltoable);
    printf("movable: %d\n", formula->movable);
    printf("mslable: %d\n", formula->support_create_mostly_statically_linked_executable);
    printf("parallel: %d\n", formula->support_build_in_parallel);

    printf("patches: %s\n", formula->patches);
    printf("reslist: %s\n", formula->reslist);

    printf("path:    %s\n", formula->path);

    printf("useBuildSystemCmake:    %d\n", formula->useBuildSystemCmake);
    printf("useBuildSystemGmake:    %d\n", formula->useBuildSystemGmake);
    printf("useBuildSystemXmake:    %d\n", formula->useBuildSystemXmake);
    printf("useBuildSystemMeson:    %d\n", formula->useBuildSystemMeson);
    printf("useBuildSystemNinja:    %d\n", formula->useBuildSystemNinja);
    printf("useBuildSystemCargo:    %d\n", formula->useBuildSystemCargo);
    printf("useBuildSystemGolang:   %d\n", formula->useBuildSystemGolang);
    printf("useBuildSystemGN:       %d\n", formula->useBuildSystemGN);
    printf("useBuildSystemZIG:      %d\n", formula->useBuildSystemZIG);
    printf("useBuildSystemWAF:      %d\n", formula->useBuildSystemWAF);
    printf("useBuildSystemNetsurf:  %d\n", formula->useBuildSystemNetsurf);
    printf("useBuildSystemConfigure:%d\n", formula->useBuildSystemConfigure);
    printf("useBuildSystemAutotools:%d\n", formula->useBuildSystemAutotools);
    printf("useBuildSystemAutogen:  %d\n", formula->useBuildSystemAutogen);
}

void xcpkg_formula_free(XCPKGFormula * formula) {
    if (formula == NULL) {
        return;
    }

    if (formula->summary != NULL) {
        free(formula->summary);
        formula->summary = NULL;
    }

    if (formula->version != NULL) {
        free(formula->version);
        formula->version = NULL;
    }

    if (formula->license != NULL) {
        free(formula->license);
        formula->license = NULL;
    }

    if (formula->web_url != NULL) {
        free(formula->web_url);
        formula->web_url = NULL;
    }

    ///////////////////////////////

    if (formula->git_url != NULL) {
        free(formula->git_url);
        formula->git_url = NULL;
    }

    if (formula->git_uri != NULL) {
        free(formula->git_uri);
        formula->git_uri = NULL;
    }

    if (formula->git_sha != NULL) {
        free(formula->git_sha);
        formula->git_sha = NULL;
    }

    if (formula->git_ref != NULL) {
        free(formula->git_ref);
        formula->git_ref = NULL;
    }

    ///////////////////////////////

    if (formula->src_url != NULL) {
        free(formula->src_url);
        formula->src_url = NULL;
    }

    if (formula->src_uri != NULL) {
        free(formula->src_uri);
        formula->src_uri = NULL;
    }

    if (formula->src_sha != NULL) {
        free(formula->src_sha);
        formula->src_sha = NULL;
    }

    ///////////////////////////////

    if (formula->fix_url != NULL) {
        free(formula->fix_url);
        formula->fix_url = NULL;
    }

    if (formula->fix_uri != NULL) {
        free(formula->fix_uri);
        formula->fix_uri = NULL;
    }

    if (formula->fix_sha != NULL) {
        free(formula->fix_sha);
        formula->fix_sha = NULL;
    }

    if (formula->fix_opt != NULL) {
        free(formula->fix_opt);
        formula->fix_opt = NULL;
    }

    ///////////////////////////////

    if (formula->res_url != NULL) {
        free(formula->res_url);
        formula->res_url = NULL;
    }

    if (formula->res_uri != NULL) {
        free(formula->res_uri);
        formula->res_uri = NULL;
    }

    if (formula->res_sha != NULL) {
        free(formula->res_sha);
        formula->res_sha = NULL;
    }

    ///////////////////////////////

    if (formula->dep_pkg != NULL) {
        free(formula->dep_pkg);
        formula->dep_pkg = NULL;
    }

    if (formula->dep_lib != NULL) {
        free(formula->dep_lib);
        formula->dep_lib = NULL;
    }

    if (formula->dep_upp != NULL) {
        free(formula->dep_upp);
        formula->dep_upp = NULL;
    }

    if (formula->dep_pip != NULL) {
        free(formula->dep_pip);
        formula->dep_pip = NULL;
    }

    if (formula->dep_plm != NULL) {
        free(formula->dep_plm);
        formula->dep_plm = NULL;
    }

    ///////////////////////////////

    if (formula->bsystem != NULL) {
        free(formula->bsystem);
        formula->bsystem = NULL;
    }

    if (formula->bscript != NULL) {
        free(formula->bscript);
        formula->bscript = NULL;
    }

    ///////////////////////////////

    if (formula->ppflags != NULL) {
        free(formula->ppflags);
        formula->ppflags = NULL;
    }

    if (formula->ccflags != NULL) {
        free(formula->ccflags);
        formula->ccflags = NULL;
    }

    if (formula->xxflags != NULL) {
        free(formula->xxflags);
        formula->xxflags = NULL;
    }

    if (formula->ldflags != NULL) {
        free(formula->ldflags);
        formula->ldflags = NULL;
    }

    ///////////////////////////////

    if (formula->dofetch != NULL) {
        free(formula->dofetch);
        formula->dofetch = NULL;
    }

    if (formula->do12345 != NULL) {
        free(formula->do12345);
        formula->do12345 = NULL;
    }

    if (formula->dopatch != NULL) {
        free(formula->dopatch);
        formula->dopatch = NULL;
    }

    if (formula->prepare != NULL) {
        free(formula->prepare);
        formula->prepare = NULL;
    }

    if (formula->install != NULL) {
        free(formula->install);
        formula->install = NULL;
    }

    if (formula->dotweak != NULL) {
        free(formula->dotweak);
        formula->dotweak = NULL;
    }

    if (formula->bindenv != NULL) {
        free(formula->bindenv);
        formula->bindenv = NULL;
    }

    if (formula->caveats != NULL) {
        free(formula->caveats);
        formula->caveats = NULL;
    }

    if (formula->patches != NULL) {
        free(formula->patches);
        formula->patches = NULL;
    }

    if (formula->reslist != NULL) {
        free(formula->reslist);
        formula->reslist = NULL;
    }

    ///////////////////////////////

    if (formula->path != NULL) {
        free(formula->path);
        formula->path = NULL;
    }

    free(formula);
}

static inline XCPKGFormulaKeyCode xcpkg_formula_key_code_from_key_name(char * key) {
           if (strcmp(key, "summary") == 0) {
        return FORMULA_KEY_CODE_summary;
    } else if (strcmp(key, "version") == 0) {
        return FORMULA_KEY_CODE_version;
    } else if (strcmp(key, "license") == 0) {
        return FORMULA_KEY_CODE_license;
    } else if (strcmp(key, "web-url") == 0) {
        return FORMULA_KEY_CODE_web_url;
    } else if (strcmp(key, "git-url") == 0) {
        return FORMULA_KEY_CODE_git_url;
    } else if (strcmp(key, "git-uri") == 0) {
        return FORMULA_KEY_CODE_git_uri;
    } else if (strcmp(key, "git-sha") == 0) {
        return FORMULA_KEY_CODE_git_sha;
    } else if (strcmp(key, "git-ref") == 0) {
        return FORMULA_KEY_CODE_git_ref;
    } else if (strcmp(key, "git-nth") == 0) {
        return FORMULA_KEY_CODE_git_nth;
    } else if (strcmp(key, "src-url") == 0) {
        return FORMULA_KEY_CODE_src_url;
    } else if (strcmp(key, "src-uri") == 0) {
        return FORMULA_KEY_CODE_src_uri;
    } else if (strcmp(key, "src-sha") == 0) {
        return FORMULA_KEY_CODE_src_sha;
    } else if (strcmp(key, "fix-url") == 0) {
        return FORMULA_KEY_CODE_fix_url;
    } else if (strcmp(key, "fix-uri") == 0) {
        return FORMULA_KEY_CODE_fix_uri;
    } else if (strcmp(key, "fix-sha") == 0) {
        return FORMULA_KEY_CODE_fix_sha;
    } else if (strcmp(key, "fix-opt") == 0) {
        return FORMULA_KEY_CODE_fix_opt;
    } else if (strcmp(key, "res-url") == 0) {
        return FORMULA_KEY_CODE_res_url;
    } else if (strcmp(key, "res-uri") == 0) {
        return FORMULA_KEY_CODE_res_uri;
    } else if (strcmp(key, "res-sha") == 0) {
        return FORMULA_KEY_CODE_res_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return FORMULA_KEY_CODE_dep_pkg;
    } else if (strcmp(key, "dep-lib") == 0) {
        return FORMULA_KEY_CODE_dep_lib;
    } else if (strcmp(key, "dep-upp") == 0) {
        return FORMULA_KEY_CODE_dep_upp;
    } else if (strcmp(key, "dep-pip") == 0) {
        return FORMULA_KEY_CODE_dep_pip;
    } else if (strcmp(key, "dep-plm") == 0) {
        return FORMULA_KEY_CODE_dep_plm;
    } else if (strcmp(key, "ppflags") == 0) {
        return FORMULA_KEY_CODE_ppflags;
    } else if (strcmp(key, "ccflags") == 0) {
        return FORMULA_KEY_CODE_ccflags;
    } else if (strcmp(key, "xxflags") == 0) {
        return FORMULA_KEY_CODE_xxflags;
    } else if (strcmp(key, "ldflags") == 0) {
        return FORMULA_KEY_CODE_ldflags;
    } else if (strcmp(key, "dofetch") == 0) {
        return FORMULA_KEY_CODE_dofetch;
    } else if (strcmp(key, "do12345") == 0) {
        return FORMULA_KEY_CODE_do12345;
    } else if (strcmp(key, "dopatch") == 0) {
        return FORMULA_KEY_CODE_dopatch;
    } else if (strcmp(key, "prepare") == 0) {
        return FORMULA_KEY_CODE_prepare;
    } else if (strcmp(key, "install") == 0) {
        return FORMULA_KEY_CODE_install;
    } else if (strcmp(key, "dotweak") == 0) {
        return FORMULA_KEY_CODE_dotweak;
    } else if (strcmp(key, "bindenv") == 0) {
        return FORMULA_KEY_CODE_bindenv;
    } else if (strcmp(key, "caveats") == 0) {
        return FORMULA_KEY_CODE_caveats;
    } else if (strcmp(key, "symlink") == 0) {
        return FORMULA_KEY_CODE_symlink;
    } else if (strcmp(key, "movable") == 0) {
        return FORMULA_KEY_CODE_movable;
    } else if (strcmp(key, "ltoable") == 0) {
        return FORMULA_KEY_CODE_ltoable;
    } else if (strcmp(key, "mslable") == 0) {
        return FORMULA_KEY_CODE_mslable;
    } else if (strcmp(key, "bsystem") == 0) {
        return FORMULA_KEY_CODE_bsystem;
    } else if (strcmp(key, "bscript") == 0) {
        return FORMULA_KEY_CODE_bscript;
    } else if (strcmp(key, "binbstd") == 0) {
        return FORMULA_KEY_CODE_binbstd;
    } else if (strcmp(key, "pkgtype") == 0) {
        return FORMULA_KEY_CODE_pkgtype;
    } else if (strcmp(key, "patches") == 0) {
        return FORMULA_KEY_CODE_patches;
    } else if (strcmp(key, "reslist") == 0) {
        return FORMULA_KEY_CODE_reslist;
    } else if (strcmp(key, "parallel") == 0) {
        return FORMULA_KEY_CODE_parallel;
    } else {
        return FORMULA_KEY_CODE_unknown;
    }
}

static inline int xcpkg_formula_set_value(XCPKGFormulaKeyCode keyCode, char * value, XCPKGFormula * formula, int * pkgtype, int * binbstd, int * symlink, int * ltoable, int * mslable, int * movable, int * parallel) {
    if (keyCode == FORMULA_KEY_CODE_unknown) {
        return XCPKG_OK;
    }

    for (;;) {
        if (value[0] == '\0') {
            return XCPKG_OK;
        }

        // non-printable ASCII characters and space
        if (value[0] <= 32) {
            value++;
        } else {
            break;
        }
    }

    switch (keyCode) {
        case FORMULA_KEY_CODE_summary: if (formula->summary != NULL) free(formula->summary); formula->summary = strdup(value); break;
        case FORMULA_KEY_CODE_version: if (formula->version != NULL) free(formula->version); formula->version = strdup(value); break;
        case FORMULA_KEY_CODE_license: if (formula->license != NULL) free(formula->license); formula->license = strdup(value); break;

        case FORMULA_KEY_CODE_web_url: if (formula->web_url != NULL) free(formula->web_url); formula->web_url = strdup(value); break;

        case FORMULA_KEY_CODE_git_url: if (formula->git_url != NULL) free(formula->git_url); formula->git_url = strdup(value); break;
        case FORMULA_KEY_CODE_git_uri: if (formula->git_uri != NULL) free(formula->git_uri); formula->git_uri = strdup(value); break;
        case FORMULA_KEY_CODE_git_sha: if (formula->git_sha != NULL) free(formula->git_sha); formula->git_sha = strdup(value); break;
        case FORMULA_KEY_CODE_git_ref: if (formula->git_ref != NULL) free(formula->git_ref); formula->git_ref = strdup(value); break;

        case FORMULA_KEY_CODE_src_url: if (formula->src_url != NULL) free(formula->src_url); formula->src_url = strdup(value); break;
        case FORMULA_KEY_CODE_src_uri: if (formula->src_uri != NULL) free(formula->src_uri); formula->src_uri = strdup(value); break;
        case FORMULA_KEY_CODE_src_sha: if (formula->src_sha != NULL) free(formula->src_sha); formula->src_sha = strdup(value); break;

        case FORMULA_KEY_CODE_fix_url: if (formula->fix_url != NULL) free(formula->fix_url); formula->fix_url = strdup(value); break;
        case FORMULA_KEY_CODE_fix_uri: if (formula->fix_uri != NULL) free(formula->fix_uri); formula->fix_uri = strdup(value); break;
        case FORMULA_KEY_CODE_fix_sha: if (formula->fix_sha != NULL) free(formula->fix_sha); formula->fix_sha = strdup(value); break;
        case FORMULA_KEY_CODE_fix_opt: if (formula->fix_opt != NULL) free(formula->fix_opt); formula->fix_opt = strdup(value); break;

        case FORMULA_KEY_CODE_res_url: if (formula->res_url != NULL) free(formula->res_url); formula->res_url = strdup(value); break;
        case FORMULA_KEY_CODE_res_uri: if (formula->res_uri != NULL) free(formula->res_uri); formula->res_uri = strdup(value); break;
        case FORMULA_KEY_CODE_res_sha: if (formula->res_sha != NULL) free(formula->res_sha); formula->res_sha = strdup(value); break;

        case FORMULA_KEY_CODE_dep_pkg: if (formula->dep_pkg != NULL) free(formula->dep_pkg); formula->dep_pkg = strdup(value); break;
        case FORMULA_KEY_CODE_dep_lib: if (formula->dep_lib != NULL) free(formula->dep_lib); formula->dep_lib = strdup(value); break;
        case FORMULA_KEY_CODE_dep_upp: if (formula->dep_upp != NULL) free(formula->dep_upp); formula->dep_upp = strdup(value); break;
        case FORMULA_KEY_CODE_dep_pip: if (formula->dep_pip != NULL) free(formula->dep_pip); formula->dep_pip = strdup(value); break;
        case FORMULA_KEY_CODE_dep_plm: if (formula->dep_plm != NULL) free(formula->dep_plm); formula->dep_plm = strdup(value); break;

        case FORMULA_KEY_CODE_ppflags: if (formula->ppflags != NULL) free(formula->ppflags); formula->ppflags = strdup(value); break;
        case FORMULA_KEY_CODE_ccflags: if (formula->ccflags != NULL) free(formula->ccflags); formula->ccflags = strdup(value); break;
        case FORMULA_KEY_CODE_xxflags: if (formula->xxflags != NULL) free(formula->xxflags); formula->xxflags = strdup(value); break;
        case FORMULA_KEY_CODE_ldflags: if (formula->ldflags != NULL) free(formula->ldflags); formula->ldflags = strdup(value); break;

        case FORMULA_KEY_CODE_do12345: if (formula->do12345 != NULL) free(formula->do12345); formula->do12345 = strdup(value); break;
        case FORMULA_KEY_CODE_dofetch: if (formula->dofetch != NULL) free(formula->dofetch); formula->dofetch = strdup(value); break;
        case FORMULA_KEY_CODE_dopatch: if (formula->dopatch != NULL) free(formula->dopatch); formula->dopatch = strdup(value); break;
        case FORMULA_KEY_CODE_prepare: if (formula->prepare != NULL) free(formula->prepare); formula->prepare = strdup(value); break;
        case FORMULA_KEY_CODE_install: if (formula->install != NULL) free(formula->install); formula->install = strdup(value); break;
        case FORMULA_KEY_CODE_dotweak: if (formula->dotweak != NULL) free(formula->dotweak); formula->dotweak = strdup(value); break;
        case FORMULA_KEY_CODE_bindenv: if (formula->bindenv != NULL) free(formula->bindenv); formula->bindenv = strdup(value); break;
        case FORMULA_KEY_CODE_caveats: if (formula->caveats != NULL) free(formula->caveats); formula->caveats = strdup(value); break;
        case FORMULA_KEY_CODE_patches: if (formula->patches != NULL) free(formula->patches); formula->patches = strdup(value); break;
        case FORMULA_KEY_CODE_reslist: if (formula->reslist != NULL) free(formula->reslist); formula->reslist = strdup(value); break;

        case FORMULA_KEY_CODE_bsystem: if (formula->bsystem != NULL) free(formula->bsystem); formula->bsystem = strdup(value); break;
        case FORMULA_KEY_CODE_bscript: if (formula->bscript != NULL) free(formula->bscript); formula->bscript = strdup(value); break;

        case FORMULA_KEY_CODE_git_nth:
            for (int i = 0; ; i++) {
                if (value[i] == '\0') {
                    break;
                }
                if (value[i] < '0' || value[i] > '9') {
                    return XCPKG_ERROR_FORMULA_SCHEME;
                }
            }
            formula->git_nth = atoi(value);
            break;
        case FORMULA_KEY_CODE_binbstd:
            if (strcmp(value, "1") == 0) {
                *binbstd = 1;
            } else if (strcmp(value, "0") == 0) {
                *binbstd = 0;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case FORMULA_KEY_CODE_symlink:
            if (strcmp(value, "1") == 0) {
                *symlink = true;
            } else if (strcmp(value, "0") == 0) {
                *symlink = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case FORMULA_KEY_CODE_movable:
            if (strcmp(value, "1") == 0) {
                *movable = true;
            } else if (strcmp(value, "0") == 0) {
                *movable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case FORMULA_KEY_CODE_pkgtype:
            if (strcmp(value, "lib") == 0) {
                *pkgtype = XCPKGPkgType_lib;
            } else if (strcmp(value, "exe") == 0) {
                *pkgtype = XCPKGPkgType_exe;
            } else if (strcmp(value, "exe+lib") == 0) {
                *pkgtype = XCPKGPkgType_lib;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case FORMULA_KEY_CODE_ltoable:
            if (strcmp(value, "1") == 0) {
                *ltoable = true;
            } else if (strcmp(value, "0") == 0) {
                *ltoable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case FORMULA_KEY_CODE_mslable:
            if (strcmp(value, "1") == 0) {
                *mslable = true;
            } else if (strcmp(value, "0") == 0) {
                *mslable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case FORMULA_KEY_CODE_parallel:
            if (strcmp(value, "1") == 0) {
                *parallel = true;
            } else if (strcmp(value, "0") == 0) {
                *parallel = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        default:
            break;
    }

    return XCPKG_OK;
}

/**
 * a is \0 or \n or space terminated
 * b is \0 terminated
 */
static inline __attribute__((always_inline)) bool _str_equal(const char * a, const char * b) {
    for (;;) {
        if (a[0] == ' ' || a[0] == '\n' || a[0] == '\0') {
            return (b[0] == '\0');
        }

        if (b[0] == '\0') {
            return (a[0] == ' ' || a[0] == '\n' || a[0] == '\0');
        }

        if (a[0] == b[0]) {
            a++;
            b++;
        } else {
            return false;
        }
    }
}

static inline const char * xcpkg_extract_bsystem_from_install_commands(XCPKGFormula * formula) {
    const char * p = formula->install;

loop:
    if (p[0] == '\0') return NULL;

    if (p[0] == ' ' || p[0] == '\n') {
        p++;
        goto loop;
    }

    if (_str_equal(p, "configure")) {
        formula->useBuildSystemConfigure = true;
        formula->useBuildSystemGmake = true;
        return "configure";
    }

    if (_str_equal(p, "cmakew")) {
        formula->useBuildSystemCmake = true;
        formula->useBuildSystemNinja = true;
        return "cmake";
    }

    if (_str_equal(p, "xmakew")) {
        formula->useBuildSystemXmake = true;
        return "xmake";
    }

    if (_str_equal(p, "gmakew")) {
        formula->useBuildSystemGmake = true;
        return "gmake";
    }

    if (_str_equal(p, "mesonw")) {
        formula->useBuildSystemMeson = true;
        formula->useBuildSystemNinja = true;
        return "meson";
    }

    if (_str_equal(p, "cabal_v2_install")) {
        formula->useBuildSystemCabal = true;
        formula->useBuildSystemGmake = true;
        return "cabal";
    }

    if (_str_equal(p, "cargow")) {
        formula->useBuildSystemCargo = true;
        return "cargo";
    }

    if (_str_equal(p, "gow")) {
        formula->useBuildSystemGolang = true;
        return "go";
    }

    if (_str_equal(p, "gnw")) {
        formula->useBuildSystemGN = true;
        return "gn";
    }

    if (_str_equal(p, "zig")) {
        formula->useBuildSystemZIG = true;
        return "zig";
    }

    for (;;) {
        if (p[0] == '\0') return XCPKG_OK;

        if (p[0] == '\n') {
            p++;
            goto loop;
        } else {
            p++;
        }
    }
}

static inline const char* xcpkg_determine_default_install_commands_from_bsystem(const char * p) {
    for (;;) {
        if (p[0] == '\0') return NULL;

        if (p[0] == ' ' || p[0] == '\n') {
            p++;
            continue;
        } else {
            break;
        }
    }

    if (_str_equal(p, "autogen")) {
        return "configure";
    }

    if (_str_equal(p, "autotools")) {
        return "configure";
    }

    if (_str_equal(p, "configure")) {
        return "configure";
    }

    if (_str_equal(p, "cmake")) {
        return "cmakew";
    }

    if (_str_equal(p, "cmake+ninja")) {
        return "cmakew";
    }

    if (_str_equal(p, "cmake+gmake")) {
        return "cmakew";
    }

    if (_str_equal(p, "xmake")) {
        return "xmakew";
    }

    if (_str_equal(p, "gmake")) {
        return "gmakew clean && gmakew && gmakew install";
    }

    if (_str_equal(p, "ninja")) {
        return "ninjaw clean && ninjaw && ninjaw install";
    }

    if (_str_equal(p, "meson")) {
        return "mesonw";
    }

    if (_str_equal(p, "cabal")) {
        return "cabal_v2_install";
    }

    if (_str_equal(p, "cargo")) {
        return "cargow install";
    }

    if (_str_equal(p, "go")) {
        return "gow";
    }

    if (_str_equal(p, "gn")) {
        return "gnw";
    }

    if (_str_equal(p, "waf")) {
        return "waf";
    }

    if (_str_equal(p, "zig")) {
        return "zig";
    }

    if (strncmp(p, "zig@", 4U) == 0) {
        return "zig";
    }

    if (_str_equal(p, "netsurf")) {
        return "netsurf_buildsystem";
    }

    return NULL;
}

static inline int xcpkg_parse_bsystem(XCPKGFormula * formula, char dep_upp_extra_buf[], size_t * dep_upp_extra_buf_len) {
    const char * p = formula->bsystem;

loop:
    if (p[0] == '\0') return XCPKG_OK;

    if (p[0] == ' ' || p[0] == '\n') {
        p++;
        goto loop;
    }

           if (_str_equal(p, "autogen")) {
        formula->useBuildSystemAutogen = true;
        formula->useBuildSystemGmake = true;
    } else if (_str_equal(p, "autotools")) {
        formula->useBuildSystemAutotools = true;
        formula->useBuildSystemGmake = true;
    } else if (_str_equal(p, "configure")) {
        formula->useBuildSystemConfigure = true;
        formula->useBuildSystemGmake = true;
    } else if (_str_equal(p, "cmake")) {
        formula->useBuildSystemCmake = true;
        formula->useBuildSystemNinja = true;
    } else if (_str_equal(p, "cmake+ninja")) {
        formula->useBuildSystemCmake = true;
        formula->useBuildSystemNinja = true;
    } else if (_str_equal(p, "cmake+gmake")) {
        formula->useBuildSystemCmake = true;
        formula->useBuildSystemGmake = true;
    } else if (_str_equal(p, "xmake")) {
        formula->useBuildSystemXmake = true;
    } else if (_str_equal(p, "gmake")) {
        formula->useBuildSystemGmake = true;
    } else if (_str_equal(p, "ninja")) {
        formula->useBuildSystemNinja = true;
    } else if (_str_equal(p, "meson")) {
        formula->useBuildSystemMeson = true;
        formula->useBuildSystemNinja = true;
    } else if (_str_equal(p, "cabal")) {
        formula->useBuildSystemCabal = true;
    } else if (_str_equal(p, "cargo")) {
        formula->useBuildSystemCargo = true;
    } else if (_str_equal(p, "go")) {
        formula->useBuildSystemGolang = true;
    } else if (_str_equal(p, "gn")) {
        formula->useBuildSystemGN = true;
    } else if (_str_equal(p, "netsurf")) {
        formula->useBuildSystemNetsurf = true;
        formula->useBuildSystemGmake = true;
    } else if (_str_equal(p, "waf")) {
        formula->useBuildSystemWAF = true;
    } else if (_str_equal(p, "zig")) {
        formula->useBuildSystemZIG = true;
        string_buffer_append(dep_upp_extra_buf, dep_upp_extra_buf_len, "zig");
    } else if (strncmp(p, "zig@", 4U) == 0) {
        formula->useBuildSystemZIG = true;

        char * q = dep_upp_extra_buf;

        for (size_t i = 0U; ; i++) {
            q[i] = p[i];

            if (q[i] == '\0') {
                (*dep_upp_extra_buf_len) += i;
                return XCPKG_OK;
            }

            if (q[i] == ' ' || q[i] == '\n') {
                q[i] = '\0';

                (*dep_upp_extra_buf_len) += i;

                p += i + 1;
                goto loop;
            }
        }

    }

    for (;;) {
        if (p[0] == '\0') return XCPKG_OK;

        if (p[0] == ' ' || p[0] == '\n') {
            p++;
            goto loop;
        } else {
            p++;
        }
    }
}

static inline int xcpkg_formula_check(XCPKGFormula * formula, const char * formulaFilePath) {
    if (formula->summary == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : summary mapping not found.\n", formulaFilePath);
        return XCPKG_ERROR_FORMULA_SCHEME;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->git_url == NULL) {
        if (formula->src_url != NULL) {
            char * p = regex_extract(formula->src_url, "https://git(hub|lab).com/[^/]*/[^/]*/");

            if (p == NULL) {
                if (errno != 0) {
                    perror(NULL);
                    return XCPKG_ERROR;
                }
            } else {
                formula->git_url = p;
                formula->git_url_is_calculated = true;
            }
        }
    } else {
        if ((formula->git_sha != NULL) && (strlen(formula->git_sha) != 40)) {
            fprintf(stderr, "scheme error in formula file: %s : git-sha mapping's value's length must be 40.\n", formulaFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->web_url == NULL) {
        if (formula->git_url == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : web-url mapping not found.\n", formulaFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        } else {
            formula->web_url_is_calculated = true;
            formula->web_url = strdup(formula->git_url);

            if (formula->web_url == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->src_url != NULL) {
        if (strncmp(formula->src_url, "dir://", 6) == 0) {
            struct stat st;

            if (stat(&formula->src_url[6], &st) != 0 || !S_ISDIR(st.st_mode)) {
                fprintf(stderr, "src-url mapping request local dir %s not exist. in formula file: %s.\n", &formula->src_url[6], formulaFilePath);
                return XCPKG_ERROR;
            }

            formula->src_is_dir = true;
        } else {
            if (formula->src_sha == NULL) {
                fprintf(stderr, "scheme error in formula file: %s : src-sha mapping not found.\n", formulaFilePath);
                return XCPKG_ERROR_FORMULA_SCHEME;
            }

            if (strlen(formula->src_sha) != 64) {
                fprintf(stderr, "scheme error in formula file: %s : src-sha mapping's value's length must be 64.\n", formulaFilePath);
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->fix_url != NULL) {
        if (formula->fix_sha == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : fix-sha mapping not found.\n", formulaFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }

        if (strlen(formula->fix_sha) != 64) {
            fprintf(stderr, "scheme error in formula file: %s : fix-sha mapping's value's length must be 64.\n", formulaFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->res_url != NULL) {
        if (formula->res_sha == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : res-sha mapping not found.\n", formulaFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }

        if (strlen(formula->res_sha) != 64) {
            fprintf(stderr, "scheme error in formula file: %s : res-sha mapping's value's length must be 64.\n", formulaFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->version == NULL) {
        if (formula->src_url != NULL) {
            if (!formula->src_is_dir) {
                char version[20]; version[0] = '\0';

                int ret = xcpkg_extract_version(formula->src_url, version, 20);

                if (ret != XCPKG_OK) {
                    return ret;
                }

                if (version[0] == '\0') {
                    fprintf(stderr, "Can't extract package version from src-url: '%s' in formula file: %s\n", formula->src_url, formulaFilePath);
                    return XCPKG_ERROR_FORMULA_SCHEME;
                } else {
                    formula->version = strdup(version);

                    if (formula->version == NULL) {
                        return XCPKG_ERROR_MEMORY_ALLOCATE;
                    }
                }
            }
        } else if (formula->git_url != NULL && formula->git_ref != NULL) {
            char version[20]; version[0] = '\0';

            int ret = xcpkg_extract_version(formula->git_ref, version, 20);

            if (ret != XCPKG_OK) {
                return ret;
            }

            if (version[0] != '\0') {
                formula->version = strdup(version);

                if (formula->version == NULL) {
                    return XCPKG_ERROR_MEMORY_ALLOCATE;
                }
            }
        }

        if (formula->version != NULL) {
            formula->version_is_calculated = true;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    char   dep_upp_extra_buf[256]; dep_upp_extra_buf[0] = '\0';
    size_t dep_upp_extra_buf_len = 0U;

    if (formula->bsystem == NULL) {
        if (formula->install == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : neither bsystem nor install mapping is found.\n", formula->path);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }

        const char * bsystem = xcpkg_extract_bsystem_from_install_commands(formula);

        if (bsystem != NULL) {
            char * p = strdup(bsystem);

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            formula->bsystem = p;
            formula->bsystem_is_calculated = true;

            if (formula->useBuildSystemZIG) {
                string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "zig");
            }
        }
    } else {
        if (formula->install == NULL) {
            const char * dobuildActions = xcpkg_determine_default_install_commands_from_bsystem(formula->bsystem);

            if (dobuildActions == NULL) {
                fprintf(stderr, "scheme error in formula file: %s : install mapping not found.\n", formula->path);
                return XCPKG_ERROR_FORMULA_SCHEME;
            }

            char * p = strdup(dobuildActions);

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            formula->install = p;
        }

        int ret = xcpkg_parse_bsystem(formula, dep_upp_extra_buf, &dep_upp_extra_buf_len);

        if (ret != XCPKG_OK) {
            return ret;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemNetsurf) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "netsurf_buildsystem");
    }

    if (formula->useBuildSystemAutogen || formula->useBuildSystemAutotools) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "automake autoconf perl gm4");
    }

    if (formula->useBuildSystemCmake) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "cmake");
    }

    if (formula->useBuildSystemGmake || formula->useBuildSystemCabal) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "gmake");
    }

    if (formula->useBuildSystemNinja) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "ninja");
    }

    if (formula->useBuildSystemXmake) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "xmake");
    }

    if (formula->useBuildSystemGolang) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "golang");
    }

    if (formula->useBuildSystemMeson || formula->useBuildSystemWAF || formula->dep_pip != NULL) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "python3");
    }

    if (formula->dep_plm != NULL) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "perl");
    }

    if (formula->fix_url != NULL || formula->res_url != NULL) {
        string_buffer_append(dep_upp_extra_buf, &dep_upp_extra_buf_len, "patch");
    }

    if (dep_upp_extra_buf[0] != '\0') {
        if (formula->dep_upp == NULL) {
            char * p = strdup(dep_upp_extra_buf);

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            formula->dep_upp = p;
        } else {
            size_t oldLength = strlen(formula->dep_upp);
            size_t newLength = oldLength + dep_upp_extra_buf_len + 2U;

            char * p = (char*)malloc(newLength * sizeof(char));

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            int ret = snprintf(p, newLength, "%s %s", formula->dep_upp, dep_upp_extra_buf);

            if (ret < 0) {
                perror(NULL);
                free(p);
                return XCPKG_ERROR;
            }

            free(formula->dep_upp);

            formula->dep_upp = p;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemMeson) {
        if (formula->dep_pip == NULL) {
            char * p = strdup("meson");

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            formula->dep_pip = p;
        } else {
            size_t oldLength = strlen(formula->dep_pip);
            size_t newLength = oldLength + 7U;

            char * p = (char*)malloc(newLength * sizeof(char));

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            int ret = snprintf(p, newLength, "%s meson", formula->dep_pip);

            if (ret < 0) {
                perror(NULL);
                free(p);
                return XCPKG_ERROR;
            }

            free(formula->dep_pip);

            formula->dep_pip = p;
        }
    }

    return XCPKG_OK;
}

int xcpkg_formula_load(const char * packageName, const char * targetPlatformName, const char * formulaFilePath, XCPKGFormula * * out) {
    char buf[PATH_MAX];

    if (formulaFilePath == NULL) {
        int ret = xcpkg_formula_path(packageName, targetPlatformName, buf);

        if (ret != XCPKG_OK) {
            return ret;
        }

        formulaFilePath = buf;
    }

    FILE * file = fopen(formulaFilePath, "r");

    if (file == NULL) {
        perror(formulaFilePath);
        return XCPKG_ERROR;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        return XCPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, file);

    XCPKGFormulaKeyCode formulaKeyCode = FORMULA_KEY_CODE_unknown;

    XCPKGFormula * formula = NULL;

    int lastTokenType = 0;

    int ret = XCPKG_OK;

    int pkgtype = -1;

    int binbstd = -1;

    int symlink = -1;

    int ltoable = -1;
    int mslable = -1;
    int movable = -1;

    int parallel = -1;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in formula file: %s\n", formulaFilePath);
            ret = XCPKG_ERROR_FORMULA_SYNTAX;
            goto finalize;
        }

        switch(token.type) {
            case YAML_KEY_TOKEN:
                lastTokenType = 1;
                break;
            case YAML_VALUE_TOKEN:
                lastTokenType = 2;
                break;
            case YAML_SCALAR_TOKEN:
                if (lastTokenType == 1) {
                    formulaKeyCode = xcpkg_formula_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (formula == NULL) {
                        formula = (XCPKGFormula*)calloc(1, sizeof(XCPKGFormula));

                        if (formula == NULL) {
                            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        formula->git_nth = 1;
                        formula->path = strdup(formulaFilePath);

                        if (formula->path == NULL) {
                            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }
                    }

                    ret = xcpkg_formula_set_value(formulaKeyCode, (char*)token.data.scalar.value, formula, &pkgtype, &binbstd, &symlink, &ltoable, &mslable, &movable, &parallel);

                    if (ret != XCPKG_OK) {
                        goto finalize;
                    }
                }
                break;
            default: 
                lastTokenType = 0;
                break;
        }

        if (token.type != YAML_STREAM_END_TOKEN) {
            yaml_token_delete(&token);
        }
    } while(token.type != YAML_STREAM_END_TOKEN);

finalize:
    yaml_token_delete(&token);

    yaml_parser_delete(&parser);

    fclose(file);

    if (ret == XCPKG_OK) {
        ret = xcpkg_formula_check(formula, formulaFilePath);

        if (ret == XCPKG_OK) {
            if (symlink == -1) {
                symlink = true;
                formula->symlink_is_calculated = true;
            }

            if (ltoable == -1) {
                ltoable = true;
                formula->ltoable_is_calculated = true;
            }

            if (mslable == -1) {
                mslable = true;
                formula->mslable_is_calculated = true;
            }

            if (movable == -1) {
                movable = true;
                formula->movable_is_calculated = true;
            }

            if (parallel == -1) {
                parallel = true;
                formula->parallel_is_calculated = true;
            }

            if (binbstd == -1) {
                binbstd = 0;
                formula->binbstd_is_calculated = true;

                if (formula->useBuildSystemGolang || formula->useBuildSystemCargo || formula->useBuildSystemCabal || formula->useBuildSystemXmake || formula->useBuildSystemZIG || formula->useBuildSystemWAF || formula->useBuildSystemNetsurf) {
                    binbstd = 1;
                } else {
                    const char * p = formula->bsystem;

                    if (p != NULL) {
                        if (_str_equal(p, "gmake")) {
                            binbstd = 1;
                        }
                    }
                }
            }

            formula->binbstd = binbstd;
            formula->symlink = symlink;
            formula->ltoable = ltoable;
            formula->support_create_mostly_statically_linked_executable = mslable;
            formula->movable = movable;
            formula->support_build_in_parallel = parallel;

            if (pkgtype == -1) {
                if (strncmp(packageName, "lib", 3) == 0) {
                    pkgtype = XCPKGPkgType_lib;
                } else {
                    size_t len = strlen(packageName);

                    if (len > 3) {
                        if (strncmp(packageName + len - 3, "lib", 3) == 0) {
                            pkgtype = XCPKGPkgType_lib;
                        }
                    }

                    if (pkgtype == -1) {
                        if (len > 4) {
                            if (strncmp(packageName + len - 4, "-dev", 4) == 0) {
                                pkgtype = XCPKGPkgType_lib;
                            }
                        }
                    }

                    if (pkgtype == -1) {
                        pkgtype = XCPKGPkgType_exe;
                    }
                }

                formula->pkgtype_is_calculated = true;
            }

            formula->pkgtype = pkgtype;

            if (formula->pkgtype == XCPKGPkgType_lib) {
                formula->support_create_mostly_statically_linked_executable = false;
            }

            (*out) = formula;
            return XCPKG_OK;
        }
    }

    xcpkg_formula_free(formula);
    return ret;
}
