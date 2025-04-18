#include <time.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <limits.h>
#include <sys/stat.h>

#include <yaml.h>

#include "core/regex/regex.h"

#include "xcpkg.h"

#define STRING_BUFFER_APPEND(strbuf,strbufLength,str) \
    size_t len = strlen(str) + 1U; \
    int ret = snprintf(&strbuf[strbufLength], len + 1U, " %s", str); \
    if (ret < 0) { \
        perror(NULL); \
        return XCPKG_ERROR; \
    } \
    if ((size_t)ret == len) { \
        strbufLength += len; \
    } else { \
        fprintf(stderr, "snprintf: the output was truncated.\n"); \
        return XCPKG_ERROR; \
    }

typedef enum {
    FORMULA_KEY_CODE_unknown,

    FORMULA_KEY_CODE_pkgtype,
    FORMULA_KEY_CODE_summary,
    FORMULA_KEY_CODE_version,
    FORMULA_KEY_CODE_license,

    FORMULA_KEY_CODE_web_url,

    FORMULA_KEY_CODE_git_url,
    FORMULA_KEY_CODE_git_sha,
    FORMULA_KEY_CODE_git_ref,
    FORMULA_KEY_CODE_git_nth,

    FORMULA_KEY_CODE_src_url,
    FORMULA_KEY_CODE_src_uri,
    FORMULA_KEY_CODE_src_sha,

    FORMULA_KEY_CODE_fix_url,
    FORMULA_KEY_CODE_fix_uri,
    FORMULA_KEY_CODE_fix_sha,

    FORMULA_KEY_CODE_res_url,
    FORMULA_KEY_CODE_res_uri,
    FORMULA_KEY_CODE_res_sha,

    FORMULA_KEY_CODE_dep_pkg,
    FORMULA_KEY_CODE_dep_upp,
    FORMULA_KEY_CODE_dep_pym,
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
    printf("git_sha: %s\n", formula->git_sha);
    printf("git_ref: %s\n", formula->git_ref);
    printf("git_nth: %zu\n", formula->git_nth);

    printf("src_url: %s\n", formula->src_url);
    printf("src_uri: %s\n", formula->src_uri);
    printf("src_sha: %s\n", formula->src_sha);

    printf("fix_url: %s\n", formula->fix_url);
    printf("fix_uri: %s\n", formula->fix_uri);
    printf("fix_sha: %s\n", formula->fix_sha);

    printf("res_url: %s\n", formula->res_url);
    printf("res_uri: %s\n", formula->res_uri);
    printf("res_sha: %s\n", formula->res_sha);

    printf("dep_pkg: %s\n", formula->dep_pkg);
    printf("dep_upp: %s\n", formula->dep_upp);
    printf("dep_pym: %s\n", formula->dep_pym);
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
    printf("symlink: %d\n", formula->symlink);
    printf("ltoable: %d\n", formula->ltoable);
    printf("movable: %d\n", formula->movable);
    printf("nslable: %d\n", formula->support_create_mostly_statically_linked_executable);
    printf("parallel: %d\n", formula->support_build_in_parallel);

    printf("caveats: %s\n", formula->caveats);

    printf("patches: %s\n", formula->patches);

    printf("path:    %s\n", formula->path);

    printf("useBuildSystemCmake:    %d\n", formula->useBuildSystemCmake);
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

    if (formula->dep_upp != NULL) {
        free(formula->dep_upp);
        formula->dep_upp = NULL;
    }

    if (formula->dep_pym != NULL) {
        free(formula->dep_pym);
        formula->dep_pym = NULL;
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

    ///////////////////////////////

    if (formula->path != NULL) {
        free(formula->path);
        formula->path = NULL;
    }

    free(formula);
}

static XCPKGFormulaKeyCode xcpkg_formula_key_code_from_key_name(char * key) {
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
    } else if (strcmp(key, "res-url") == 0) {
        return FORMULA_KEY_CODE_res_url;
    } else if (strcmp(key, "res-uri") == 0) {
        return FORMULA_KEY_CODE_res_uri;
    } else if (strcmp(key, "res-sha") == 0) {
        return FORMULA_KEY_CODE_res_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return FORMULA_KEY_CODE_dep_pkg;
    } else if (strcmp(key, "dep-upp") == 0) {
        return FORMULA_KEY_CODE_dep_upp;
    } else if (strcmp(key, "dep-pym") == 0) {
        return FORMULA_KEY_CODE_dep_pym;
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
    } else if (strcmp(key, "parallel") == 0) {
        return FORMULA_KEY_CODE_parallel;
    } else {
        return FORMULA_KEY_CODE_unknown;
    }
}

static int xcpkg_formula_set_value(XCPKGFormulaKeyCode keyCode, char * value, XCPKGFormula * formula, int * pkgtype) {
    if (keyCode == FORMULA_KEY_CODE_unknown) {
        return XCPKG_OK;
    }

    for (;;) {
        char c = value[0];

        if (c == '\0') {
            return XCPKG_OK;
        }

        // non-printable ASCII characters and space
        if (c <= 32) {
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
        case FORMULA_KEY_CODE_git_sha: if (formula->git_sha != NULL) free(formula->git_sha); formula->git_sha = strdup(value); break;
        case FORMULA_KEY_CODE_git_ref: if (formula->git_ref != NULL) free(formula->git_ref); formula->git_ref = strdup(value); break;

        case FORMULA_KEY_CODE_src_url: if (formula->src_url != NULL) free(formula->src_url); formula->src_url = strdup(value); break;
        case FORMULA_KEY_CODE_src_uri: if (formula->src_uri != NULL) free(formula->src_uri); formula->src_uri = strdup(value); break;
        case FORMULA_KEY_CODE_src_sha: if (formula->src_sha != NULL) free(formula->src_sha); formula->src_sha = strdup(value); break;

        case FORMULA_KEY_CODE_fix_url: if (formula->fix_url != NULL) free(formula->fix_url); formula->fix_url = strdup(value); break;
        case FORMULA_KEY_CODE_fix_uri: if (formula->fix_uri != NULL) free(formula->fix_uri); formula->fix_uri = strdup(value); break;
        case FORMULA_KEY_CODE_fix_sha: if (formula->fix_sha != NULL) free(formula->fix_sha); formula->fix_sha = strdup(value); break;

        case FORMULA_KEY_CODE_res_url: if (formula->res_url != NULL) free(formula->res_url); formula->res_url = strdup(value); break;
        case FORMULA_KEY_CODE_res_uri: if (formula->res_uri != NULL) free(formula->res_uri); formula->res_uri = strdup(value); break;
        case FORMULA_KEY_CODE_res_sha: if (formula->res_sha != NULL) free(formula->res_sha); formula->res_sha = strdup(value); break;

        case FORMULA_KEY_CODE_dep_pkg: if (formula->dep_pkg != NULL) free(formula->dep_pkg); formula->dep_pkg = strdup(value); break;
        case FORMULA_KEY_CODE_dep_upp: if (formula->dep_upp != NULL) free(formula->dep_upp); formula->dep_upp = strdup(value); break;
        case FORMULA_KEY_CODE_dep_pym: if (formula->dep_pym != NULL) free(formula->dep_pym); formula->dep_pym = strdup(value); break;
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

        case FORMULA_KEY_CODE_bsystem: if (formula->bsystem != NULL) free(formula->bsystem); formula->bsystem = strdup(value); break;
        case FORMULA_KEY_CODE_bscript: if (formula->bscript != NULL) free(formula->bscript); formula->bscript = strdup(value); break;

        case FORMULA_KEY_CODE_git_nth:
            formula->git_nth = atoi(value);
            break;
        case FORMULA_KEY_CODE_binbstd:
            if (strcmp(value, "1") == 0) {
                formula->binbstd = true;
            } else if (strcmp(value, "0") == 0) {
                formula->binbstd = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case FORMULA_KEY_CODE_symlink:
            if (strcmp(value, "1") == 0) {
                formula->symlink = true;
            } else if (strcmp(value, "0") == 0) {
                formula->symlink = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case FORMULA_KEY_CODE_movable:
            if (strcmp(value, "1") == 0) {
                formula->movable = true;
            } else if (strcmp(value, "0") == 0) {
                formula->movable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case FORMULA_KEY_CODE_pkgtype:
            if (strcmp(value, "lib") == 0) {
                *pkgtype = XCPKGPkgType_lib;
            } else if (strcmp(value, "exe") == 0) {
                *pkgtype = XCPKGPkgType_exe;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case FORMULA_KEY_CODE_ltoable:
            if (strcmp(value, "1") == 0) {
                formula->ltoable = true;
            } else if (strcmp(value, "0") == 0) {
                formula->ltoable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case FORMULA_KEY_CODE_mslable:
            if (strcmp(value, "1") == 0) {
                formula->support_create_mostly_statically_linked_executable = true;
            } else if (strcmp(value, "0") == 0) {
                formula->support_create_mostly_statically_linked_executable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case FORMULA_KEY_CODE_parallel:
            if (strcmp(value, "1") == 0) {
                formula->support_build_in_parallel = true;
            } else if (strcmp(value, "0") == 0) {
                formula->support_build_in_parallel = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        default:
            break;
    }

    return XCPKG_OK;
}

static int xcpkg_formula_check_bsystem(XCPKGFormula * formula) {
    if (formula->bsystem != NULL) {
        return XCPKG_OK;
    }

    if (formula->install == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : neither bsystem nor install mapping is found.\n", formula->path);
        return XCPKG_ERROR_FORMULA_SCHEME;
    }

    const char * p = formula->install;

    for (;;) {
        if (p[0] == '\0') {
            return XCPKG_OK;
        }

        if (p[0] <= 32) {
            p++;
            continue;
        }

        for (int i = 0; ; i++) {
            if (p[i] <= 32) {
                if (strncmp(p, "configure", i) == 0) {
                    formula->bsystem = strdup("configure");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "cmakew", i) == 0) {
                    formula->bsystem = strdup("cmake");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "xmakew", i) == 0) {
                    formula->bsystem = strdup("xmake");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "gmakew", i) == 0) {
                    formula->bsystem = strdup("gmake");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "mesonw", i) == 0) {
                    formula->bsystem = strdup("meson");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "cabalw", i) == 0) {
                    formula->bsystem = strdup("cabal");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "cargow", i) == 0) {
                    formula->bsystem = strdup("cargo");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "gow", i) == 0) {
                    formula->bsystem = strdup("go");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                if (strncmp(p, "gnw", i) == 0) {
                    formula->bsystem = strdup("gn");
                    formula->bsystem_is_calculated = true;
                    return XCPKG_OK;
                }

                p += i;

                for (;;) {
                    if (p[0] == '\0') {
                        return XCPKG_OK;
                    }

                    if (p[0] == '\n') {
                        p++;
                        break;
                    } else {
                        p++;
                    }
                }

                break;
            }
        }
    }

    return XCPKG_OK;
}

static int xcpkg_extract_version_from_src_url(const char * url, char versionBuf[], size_t versionBufCapacity) {
    if (url == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (url[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    int slashIndex = -1;
    int nulIndex = -1;

    for (int i = 0; ; i++) {
        if (url[i] == '\0') {
            nulIndex = i;
            break;
        }

        if (url[i] == '/') {
            slashIndex = i;
        }
    }

    if (slashIndex == -1) {
        return XCPKG_ERROR_INVALID_URL;
    }

    /////////////////////////////

    const char * p = url + slashIndex + 1;

    char buf[nulIndex - slashIndex];

    int dotIndex1 = -1;
    int dotIndex2 = -1;

    for (int i = 0; ; i++) {
        buf[i] = p[i];

        if (p[i] == '\0') {
            break;
        }

        if (p[i] == '.') {
            dotIndex1 = dotIndex2;
            dotIndex2 = i;
        }
    }

    /////////////////////////////

    /**
     * v2023.2.tar.gz
     * V0.3.0.tar.gz
     * R70.11.tar.gz
     * r2.5.tar.gz
     * mksh-R59c.tgz
     * gping-v1.16.1.tar.gz
     * expat-2.6.4.tar.lz
     * tectonic@0.15.0.tar.gz
     * mediainfo_24.05.tar.bz2
     * upx-4.2.4-src.tar.xz
     * v5.7.0-stable.tar.gz
     * v0.4.4-alpha.tar.gz
     * v0.15.0-beta.tar.gz
     * helix-25.01.1-source.tar.xz
     * llvm-project-18.1.8.src.tar.xz
     * libiberty_20230104.orig.tar.xz
     */

    bool gotFileNameExtension = false;


    if (dotIndex1 > 0) {
        const char * q = buf + dotIndex1 + 1;

        if (strcmp(q, "tar.gz") == 0 ||
            strcmp(q, "tar.xz") == 0 ||
            strcmp(q, "tar.lz") == 0 ||
            strcmp(q, "tar.bz2") == 0) {
            buf[dotIndex1] = '\0';
            nulIndex = dotIndex1;
            gotFileNameExtension = true;
        }
    }

    if (!gotFileNameExtension && dotIndex2 > 0) {
        const char * q = buf + dotIndex2 + 1;

        if (strcmp(q, "tgz") == 0 ||
            strcmp(q, "txz") == 0 ||
            strcmp(q, "tlz") == 0 ||
            strcmp(q, "tbz2") == 0 ||
            strcmp(q, "zip") == 0 ||
            strcmp(q, "crate") == 0) {
            buf[dotIndex2] = '\0';
            nulIndex = dotIndex2;
            gotFileNameExtension = true;
        }
    }

    if (!gotFileNameExtension) {
        return XCPKG_OK;
    }

    /////////////////////////////

    int dotIndex = -1;

    for (int i = nulIndex - 1; i >= 0; i--) {
        if (buf[i] == '.' || buf[i] == '-') {
            dotIndex = i;
            break;
        }
    }

    if (dotIndex > 0) {
        const char * q = buf + dotIndex + 1;

        if (strcmp(q, "orig") == 0 || strcmp(q, "src") == 0 || strcmp(q, "source") == 0 || strcmp(q, "stable") == 0 || strcmp(q, "alpha") == 0 || strcmp(q, "beta") == 0) {
            buf[dotIndex] = '\0';
        }
    }

    /////////////////////////////

    int hyphenIndex = -1;

    for (int i = 0; ; i++) {
        if (buf[i] == '\0') {
            break;
        }

        if (buf[i] == '-' || buf[i] == '_' || buf[i] == '@') {
            if (i > 0) {
                int m = i - 1;
                int n = i + 1;

                if (buf[m] >= '0' && buf[m] <= '9' && buf[n] >= '0' && buf[n] <= '9') {
                    buf[i] = '.';
                } else {
                    buf[i] = '-';
                    hyphenIndex = i;
                }
            } else {
                buf[i] = '-';
                hyphenIndex = i;
            }
        }
    }

    /////////////////////////////

    if (hyphenIndex == -1) {
        p = buf;
    } else {
        p = buf + hyphenIndex + 1;
    }

    switch (p[0]) {
        case 'v':
        case 'V':
        case 'r':
        case 'R': p++;
    }

    size_t i;

    for (i = 0U; i < versionBufCapacity; i++) {
        if (p[i] == '\0') {
            break;
        }

        versionBuf[i] = p[i];
    }
    
    versionBuf[i] = '\0';

    return XCPKG_OK;
}

static int xcpkg_extract_version_from_git_ref(const char * ref, char versionBuf[], size_t versionBufCapacity) {
    if (ref == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    if (ref[0] == '\0') {
        return XCPKG_ERROR_ARG_IS_EMPTY;
    }

    int slashIndex = -1;
    int nulIndex = -1;

    for (int i = 0; ; i++) {
        if (ref[i] == '\0') {
            nulIndex = i;
            break;
        }

        if (ref[i] == '/') {
            slashIndex = i;
        }
    }

    size_t capcity;

    const char * p;

    if (slashIndex == -1) {
        p = ref;
        capcity = nulIndex + 1;
    } else {
        p = ref + slashIndex + 1;
        capcity = nulIndex - slashIndex;
    }

    if (p[0] == 'v') {
        p++;
        capcity--;
    }

    for (int i = 0; p[i] != '\0'; i++) {
        if (p[i] == '.') {
            continue;
        }

        if (p[i] < '0' && p[i] > '9') {
            return XCPKG_OK;
        }
    }

    strncpy(versionBuf, p, (capcity > versionBufCapacity ) ? versionBufCapacity : capcity);

    return XCPKG_OK;
}

static int xcpkg_formula_check(XCPKGFormula * formula, const char * formulaFilePath) {
    if (formula->summary == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : summary mapping not found.\n", formulaFilePath);
        return XCPKG_ERROR_FORMULA_SCHEME;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->web_url == NULL) {
        if (formula->git_url == NULL) {
            if (formula->src_url == NULL) {
                fprintf(stderr, "scheme error in formula file: %s : web-url mapping not found.\n", formulaFilePath);
                return XCPKG_ERROR_FORMULA_SCHEME;
            } else {
                char * p = regex_extract(formula->src_url, "https://git(hub|lab).com/[^/]*/[^/]*/");

                if (p == NULL) {
                    if (errno == 0) {
                        fprintf(stderr, "scheme error in formula file: %s : web-url mapping not found.\n", formulaFilePath);
                        return XCPKG_ERROR_FORMULA_SCHEME;
                    } else {
                        perror(NULL);
                        return XCPKG_ERROR;
                    }
                } else {
                    formula->web_url_is_calculated = true;
                    formula->web_url = p;
                }
            }
        } else {
            formula->web_url_is_calculated = true;
            formula->web_url = strdup(formula->git_url);

            if (formula->web_url == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->src_url == NULL) {
        if (formula->git_url == NULL) {
            fprintf(stderr, "scheme error in formula file: %s : neither src-url nor git-url mapping is found.\n", formulaFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        } else {
            if ((formula->git_sha != NULL) && (strlen(formula->git_sha) != 40)) {
                fprintf(stderr, "scheme error in formula file: %s : git-sha mapping's value's length must be 40.\n", formulaFilePath);
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
        }
    } else {
        size_t i = 0;
        char   c;

        for (;;) {
            c = formula->src_url[i];

            if (c == '\0') {
                break;
            }

            if (i == 5) {
                if (strncmp(formula->src_url, "dir://", 6) == 0) {
                    struct stat st;

                    if (stat(&formula->src_url[6], &st) != 0 || !S_ISDIR(st.st_mode)) {
                        fprintf(stderr, "src-url mapping request local dir %s not exist. in formula file: %s.\n", &formula->src_url[6], formulaFilePath);
                        return XCPKG_ERROR;
                    }

                    formula->src_is_dir = true;
                }

                break;
            }

            i++;
        }

        if (!(formula->src_is_dir)) {
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

                int ret = xcpkg_extract_version_from_src_url(formula->src_url, version, 20);

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
        } else if (formula->git_url != NULL) {
            char version[20]; version[0] = '\0';

            int ret = xcpkg_extract_version_from_git_ref(formula->git_ref, version, 20);

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

        if (formula->version == NULL) {
            time_t tt = time(NULL);
            struct tm * tms = gmtime(&tt);

            formula->version = (char*)calloc(11, sizeof(char));

            if (formula->version == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            strftime(formula->version, 11, "%Y.%m.%d", tms);
        }

        formula->version_is_calculated = true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    int ret = xcpkg_formula_check_bsystem(formula);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (formula->bsystem == NULL) {
        return XCPKG_OK;
    }

    size_t  bsystemBufCapcity = strlen(formula->bsystem) + 1U;
    char    bsystemBuf[bsystemBufCapcity];
    strncpy(bsystemBuf, formula->bsystem, bsystemBufCapcity);

    char * bsystem = strtok(bsystemBuf, " ");

    while (bsystem != NULL) {
               if (strcmp(bsystem, "autogen") == 0) {
            formula->useBuildSystemAutogen = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "autotools") == 0) {
            formula->useBuildSystemAutotools = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "configure") == 0) {
            formula->useBuildSystemConfigure = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "cmake") == 0) {
            formula->useBuildSystemCmake = true;
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "cmake-ninja") == 0) {
            formula->useBuildSystemCmake = true;
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "cmake-gmake") == 0) {
            formula->useBuildSystemCmake = true;
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "xmake") == 0) {
            formula->useBuildSystemXmake = true;
        } else if (strcmp(bsystem, "gmake") == 0) {
            formula->useBuildSystemGmake = true;
        } else if (strcmp(bsystem, "ninja") == 0) {
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "meson") == 0) {
            formula->useBuildSystemMeson = true;
            formula->useBuildSystemNinja = true;
        } else if (strcmp(bsystem, "cargo") == 0) {
            formula->useBuildSystemCargo = true;
        } else if (strcmp(bsystem, "go") == 0) {
            formula->useBuildSystemGolang = true;
        } else if (strcmp(bsystem, "gn") == 0) {
            formula->useBuildSystemGN = true;
        }

        bsystem = strtok(NULL, " ");
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    char   dep_upp_extra_buf[256] = {0};
    size_t dep_upp_extra_buf_len = 0U;

    if (formula->useBuildSystemAutogen || formula->useBuildSystemAutotools) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "automake autoconf perl gm4")
    }

    if (formula->useBuildSystemCmake) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "cmake")
    }

    if (formula->useBuildSystemGmake) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "gmake")
    }

    if (formula->useBuildSystemNinja) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "ninja")
    }

    if (formula->useBuildSystemXmake) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "xmake")
    }

    if (formula->useBuildSystemGolang) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "golang")
    }

    if (formula->useBuildSystemMeson || (formula->dep_pym != NULL)) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "python3")
    }

    if (formula->dep_plm != NULL) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "perl")
    }

    if (formula->fix_url != NULL || formula->res_url != NULL) {
        STRING_BUFFER_APPEND(dep_upp_extra_buf, dep_upp_extra_buf_len, "patch")
    }

    if (formula->dep_upp == NULL) {
        char * p = strdup(dep_upp_extra_buf);

        if (p == NULL) {
            return XCPKG_ERROR_MEMORY_ALLOCATE;
        }

        formula->dep_upp = p;
    } else {
        size_t oldLength = strlen(formula->dep_upp);
        size_t newLength = oldLength + dep_upp_extra_buf_len + 2U;

        char * p = (char*)calloc(newLength, sizeof(char));

        if (p == NULL) {
            return XCPKG_ERROR_MEMORY_ALLOCATE;
        }

        ret = snprintf(p, newLength, "%s %s", formula->dep_upp, dep_upp_extra_buf);

        if (ret < 0) {
            perror(NULL);
            return XCPKG_ERROR;
        }

        free(formula->dep_upp);

        formula->dep_upp = p;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemMeson) {
        if (formula->dep_pym == NULL) {
            char * p = strdup("meson");

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            formula->dep_pym = p;
        } else {
            size_t oldLength = strlen(formula->dep_pym);
            size_t newLength = oldLength + 7U;

            char * p = (char*)calloc(newLength, sizeof(char));

            if (p == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }

            ret = snprintf(p, newLength, "%s meson", formula->dep_pym);

            if (ret < 0) {
                perror(NULL);
                return XCPKG_ERROR;
            }

            free(formula->dep_pym);

            formula->dep_pym = p;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->install == NULL) {
        const char * dobuildActions;

        if (formula->useBuildSystemAutogen) {
            dobuildActions = "configure";
        } else if (formula->useBuildSystemAutotools) {
            dobuildActions = "configure";
        } else if (formula->useBuildSystemConfigure) {
            dobuildActions = "configure";
        } else if (formula->useBuildSystemCmake) {
            dobuildActions = "cmakew";
        } else if (formula->useBuildSystemXmake) {
            dobuildActions = "xmakew";
        } else if (formula->useBuildSystemMeson) {
            dobuildActions = "mesonw";
        } else if (formula->useBuildSystemNinja) {
            dobuildActions = "ninjaw clean && ninjaw && ninjaw install";
        } else if (formula->useBuildSystemGmake) {
            dobuildActions = "gmakew clean && gmakew && gmakew install";
        } else if (formula->useBuildSystemCargo) {
            dobuildActions = "cargow install";
        } else if (formula->useBuildSystemGolang) {
            dobuildActions = "gow";
        } else {
            fprintf(stderr, "scheme error in formula file: %s : dobuild mapping not found.\n", formula->path);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }

        formula->install = strdup(dobuildActions);

        if (formula->install == NULL) {
            return XCPKG_ERROR_MEMORY_ALLOCATE;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formula->useBuildSystemXmake || formula->useBuildSystemCargo || formula->useBuildSystemGolang) {
        formula->binbstd = true;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

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

                        formula->path = strdup(formulaFilePath);

                        if (formula->path == NULL) {
                            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        formula->git_nth = 1U;
                        formula->binbstd = false;
                        formula->symlink = true;
                        formula->ltoable = true;
                        formula->movable = true;
                        formula->support_build_in_parallel = true;
                        formula->support_create_mostly_statically_linked_executable = true;
                    }

                    ret = xcpkg_formula_set_value(formulaKeyCode, (char*)token.data.scalar.value, formula, &pkgtype);

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
            if (pkgtype == -1) {
                if (strncmp(packageName, "lib", 3) == 0) {
                    pkgtype = XCPKGPkgType_lib;
                } else {
                    size_t len = strlen(packageName);

                    if (len > 3) {
                        if (strncmp(packageName + len - 4, "lib", 3) == 0) {
                            pkgtype = XCPKGPkgType_lib;
                        }
                    }

                    if (pkgtype == -1) {
                        if (len > 4) {
                            if (strncmp(packageName + len - 5, "-dev", 4) == 0) {
                                pkgtype = XCPKGPkgType_lib;
                            }
                        }
                    }

                    if (pkgtype == -1) {
                        pkgtype = XCPKGPkgType_exe;
                    }
                }
            }

            formula->pkgtype = pkgtype;
            (*out) = formula;
            return XCPKG_OK;
        }
    }

    xcpkg_formula_free(formula);
    return ret;
}
