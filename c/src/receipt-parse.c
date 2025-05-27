#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <limits.h>

#include <unistd.h>
#include <yaml.h>

#include "xcpkg.h"

typedef enum {
    XCPKGReceiptKeyCode_unknown,

    XCPKGReceiptKeyCode_pkgtype,
    XCPKGReceiptKeyCode_summary,
    XCPKGReceiptKeyCode_version,
    XCPKGReceiptKeyCode_license,

    XCPKGReceiptKeyCode_web_url,

    XCPKGReceiptKeyCode_git_url,
    XCPKGReceiptKeyCode_git_sha,
    XCPKGReceiptKeyCode_git_ref,
    XCPKGReceiptKeyCode_git_nth,

    XCPKGReceiptKeyCode_src_url,
    XCPKGReceiptKeyCode_src_uri,
    XCPKGReceiptKeyCode_src_sha,

    XCPKGReceiptKeyCode_fix_url,
    XCPKGReceiptKeyCode_fix_uri,
    XCPKGReceiptKeyCode_fix_sha,
    XCPKGReceiptKeyCode_fix_opt,

    XCPKGReceiptKeyCode_res_url,
    XCPKGReceiptKeyCode_res_uri,
    XCPKGReceiptKeyCode_res_sha,

    XCPKGReceiptKeyCode_dep_pkg,
    XCPKGReceiptKeyCode_dep_upp,
    XCPKGReceiptKeyCode_dep_pym,
    XCPKGReceiptKeyCode_dep_plm,

    XCPKGReceiptKeyCode_bsystem,
    XCPKGReceiptKeyCode_bscript,
    XCPKGReceiptKeyCode_binbstd,

    XCPKGReceiptKeyCode_dofetch,
    XCPKGReceiptKeyCode_do12345,
    XCPKGReceiptKeyCode_dopatch,
    XCPKGReceiptKeyCode_prepare,
    XCPKGReceiptKeyCode_install,
    XCPKGReceiptKeyCode_dotweak,
    XCPKGReceiptKeyCode_bindenv,
    XCPKGReceiptKeyCode_caveats,

    XCPKGReceiptKeyCode_symlink,
    XCPKGReceiptKeyCode_movable,
    XCPKGReceiptKeyCode_ltoable,
    XCPKGReceiptKeyCode_mslable,

    XCPKGReceiptKeyCode_ppflags,
    XCPKGReceiptKeyCode_ccflags,
    XCPKGReceiptKeyCode_xxflags,
    XCPKGReceiptKeyCode_ldflags,

    XCPKGReceiptKeyCode_patches,
    XCPKGReceiptKeyCode_reslist,

    XCPKGReceiptKeyCode_parallel,

    XCPKGReceiptKeyCode_builtby,
    XCPKGReceiptKeyCode_builtat,
    XCPKGReceiptKeyCode_builtfor,
} XCPKGReceiptKeyCode;

void xcpkg_receipt_dump(XCPKGReceipt * receipt) {
    if (receipt == NULL) {
        return;
    }

    switch (receipt->pkgtype) {
        case XCPKGPkgType_exe: printf("pkgtype: exe\n"); break;
        case XCPKGPkgType_lib: printf("pkgtype: lib\n"); break;
    }

    printf("summary: %s\n", receipt->summary);
    printf("version: %s\n", receipt->version);
    printf("license: %s\n", receipt->license);

    printf("web-url: %s\n", receipt->web_url);

    printf("git_url: %s\n", receipt->git_url);
    printf("git_sha: %s\n", receipt->git_sha);
    printf("git_ref: %s\n", receipt->git_ref);
    printf("git_nth: %zu\n", receipt->git_nth);

    printf("src_url: %s\n", receipt->src_url);
    printf("src_uri: %s\n", receipt->src_uri);
    printf("src_sha: %s\n", receipt->src_sha);

    printf("fix_url: %s\n", receipt->fix_url);
    printf("fix_uri: %s\n", receipt->fix_uri);
    printf("fix_sha: %s\n", receipt->fix_sha);
    printf("fix_opt: %s\n", receipt->fix_opt);

    printf("res_url: %s\n", receipt->res_url);
    printf("res_uri: %s\n", receipt->res_uri);
    printf("res_sha: %s\n", receipt->res_sha);

    printf("dep_pkg: %s\n", receipt->dep_pkg);
    printf("dep_upp: %s\n", receipt->dep_upp);
    printf("dep_pym: %s\n", receipt->dep_pym);
    printf("dep_plm: %s\n", receipt->dep_plm);

    printf("bsystem: %s\n", receipt->bsystem);
    printf("bscript: %s\n", receipt->bscript);
    printf("binbstd: %d\n", receipt->binbstd);

    printf("dofetch: %s\n", receipt->dofetch);
    printf("do12345: %s\n", receipt->do12345);
    printf("dopatch: %s\n", receipt->dopatch);
    printf("prepare: %s\n", receipt->prepare);
    printf("install: %s\n", receipt->install);
    printf("dotweak: %s\n", receipt->dotweak);
    printf("bindenv: %s\n", receipt->bindenv);
    printf("symlink: %d\n", receipt->symlink);
    printf("ltoable: %d\n", receipt->ltoable);
    printf("movable: %d\n", receipt->movable);
    printf("mslable: %d\n", receipt->support_create_mostly_statically_linked_executable);
    printf("parallel: %d\n", receipt->support_build_in_parallel);

    printf("caveats: %s\n", receipt->caveats);

    printf("patches: %s\n", receipt->patches);
    printf("reslist: %s\n", receipt->reslist);

    printf("builtby: %s\n", receipt->builtBy);
    printf("builtat: %s\n", receipt->builtAt);
    printf("builtfor: %s\n", receipt->builtFor);

    printf("path:    %s\n", receipt->path);
}

void xcpkg_receipt_free(XCPKGReceipt * receipt) {
    if (receipt == NULL) {
        return;
    }

    if (receipt->summary != NULL) {
        free(receipt->summary);
        receipt->summary = NULL;
    }

    if (receipt->version != NULL) {
        free(receipt->version);
        receipt->version = NULL;
    }

    if (receipt->license != NULL) {
        free(receipt->license);
        receipt->license = NULL;
    }

    if (receipt->web_url != NULL) {
        free(receipt->web_url);
        receipt->web_url = NULL;
    }

    ///////////////////////////////

    if (receipt->git_url != NULL) {
        free(receipt->git_url);
        receipt->git_url = NULL;
    }

    if (receipt->git_sha != NULL) {
        free(receipt->git_sha);
        receipt->git_sha = NULL;
    }

    if (receipt->git_ref != NULL) {
        free(receipt->git_ref);
        receipt->git_ref = NULL;
    }

    ///////////////////////////////

    if (receipt->src_url != NULL) {
        free(receipt->src_url);
        receipt->src_url = NULL;
    }

    if (receipt->src_uri != NULL) {
        free(receipt->src_uri);
        receipt->src_uri = NULL;
    }

    if (receipt->src_sha != NULL) {
        free(receipt->src_sha);
        receipt->src_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->fix_url != NULL) {
        free(receipt->fix_url);
        receipt->fix_url = NULL;
    }

    if (receipt->fix_uri != NULL) {
        free(receipt->fix_uri);
        receipt->fix_uri = NULL;
    }

    if (receipt->fix_sha != NULL) {
        free(receipt->fix_sha);
        receipt->fix_sha = NULL;
    }

    if (receipt->fix_opt != NULL) {
        free(receipt->fix_opt);
        receipt->fix_opt = NULL;
    }

    ///////////////////////////////

    if (receipt->res_url != NULL) {
        free(receipt->res_url);
        receipt->res_url = NULL;
    }

    if (receipt->res_uri != NULL) {
        free(receipt->res_uri);
        receipt->res_uri = NULL;
    }

    if (receipt->res_sha != NULL) {
        free(receipt->res_sha);
        receipt->res_sha = NULL;
    }

    ///////////////////////////////

    if (receipt->dep_pkg != NULL) {
        free(receipt->dep_pkg);
        receipt->dep_pkg = NULL;
    }

    if (receipt->dep_upp != NULL) {
        free(receipt->dep_upp);
        receipt->dep_upp = NULL;
    }

    if (receipt->dep_pym != NULL) {
        free(receipt->dep_pym);
        receipt->dep_pym = NULL;
    }

    if (receipt->dep_plm != NULL) {
        free(receipt->dep_plm);
        receipt->dep_plm = NULL;
    }

    ///////////////////////////////

    if (receipt->bsystem != NULL) {
        free(receipt->bsystem);
        receipt->bsystem = NULL;
    }

    if (receipt->bscript != NULL) {
        free(receipt->bscript);
        receipt->bscript = NULL;
    }

    ///////////////////////////////

    if (receipt->ppflags != NULL) {
        free(receipt->ppflags);
        receipt->ppflags = NULL;
    }

    if (receipt->ccflags != NULL) {
        free(receipt->ccflags);
        receipt->ccflags = NULL;
    }

    if (receipt->xxflags != NULL) {
        free(receipt->xxflags);
        receipt->xxflags = NULL;
    }

    if (receipt->ldflags != NULL) {
        free(receipt->ldflags);
        receipt->ldflags = NULL;
    }

    ///////////////////////////////

    if (receipt->dofetch != NULL) {
        free(receipt->dofetch);
        receipt->dofetch = NULL;
    }

    if (receipt->do12345 != NULL) {
        free(receipt->do12345);
        receipt->do12345 = NULL;
    }

    if (receipt->dopatch != NULL) {
        free(receipt->dopatch);
        receipt->dopatch = NULL;
    }

    if (receipt->prepare != NULL) {
        free(receipt->prepare);
        receipt->prepare = NULL;
    }

    if (receipt->install != NULL) {
        free(receipt->install);
        receipt->install = NULL;
    }

    if (receipt->dotweak != NULL) {
        free(receipt->dotweak);
        receipt->dotweak = NULL;
    }

    if (receipt->bindenv != NULL) {
        free(receipt->bindenv);
        receipt->bindenv = NULL;
    }

    if (receipt->caveats != NULL) {
        free(receipt->caveats);
        receipt->caveats = NULL;
    }

    if (receipt->patches != NULL) {
        free(receipt->patches);
        receipt->patches = NULL;
    }

    if (receipt->reslist != NULL) {
        free(receipt->reslist);
        receipt->reslist = NULL;
    }

    ///////////////////////////////

    if (receipt->path != NULL) {
        free(receipt->path);
        receipt->path = NULL;
    }

    ///////////////////////////////

    if (receipt->builtBy != NULL) {
        free(receipt->builtBy);
        receipt->builtBy = NULL;
    }

    if (receipt->builtAt != NULL) {
        free(receipt->builtAt);
        receipt->builtAt = NULL;
    }

    if (receipt->builtFor != NULL) {
        free(receipt->builtFor);
        receipt->builtAt = NULL;
    }

    free(receipt);
}

static XCPKGReceiptKeyCode xcpkg_receipt_key_code_from_key_name(char * key) {
           if (strcmp(key, "summary") == 0) {
        return XCPKGReceiptKeyCode_summary;
    } else if (strcmp(key, "version") == 0) {
        return XCPKGReceiptKeyCode_version;
    } else if (strcmp(key, "license") == 0) {
        return XCPKGReceiptKeyCode_license;
    } else if (strcmp(key, "web-url") == 0) {
        return XCPKGReceiptKeyCode_web_url;
    } else if (strcmp(key, "git-url") == 0) {
        return XCPKGReceiptKeyCode_git_url;
    } else if (strcmp(key, "git-sha") == 0) {
        return XCPKGReceiptKeyCode_git_sha;
    } else if (strcmp(key, "git-ref") == 0) {
        return XCPKGReceiptKeyCode_git_ref;
    } else if (strcmp(key, "git-nth") == 0) {
        return XCPKGReceiptKeyCode_git_nth;
    } else if (strcmp(key, "src-url") == 0) {
        return XCPKGReceiptKeyCode_src_url;
    } else if (strcmp(key, "src-uri") == 0) {
        return XCPKGReceiptKeyCode_src_uri;
    } else if (strcmp(key, "src-sha") == 0) {
        return XCPKGReceiptKeyCode_src_sha;
    } else if (strcmp(key, "fix-url") == 0) {
        return XCPKGReceiptKeyCode_fix_url;
    } else if (strcmp(key, "fix-uri") == 0) {
        return XCPKGReceiptKeyCode_fix_uri;
    } else if (strcmp(key, "fix-sha") == 0) {
        return XCPKGReceiptKeyCode_fix_sha;
    } else if (strcmp(key, "fix-opt") == 0) {
        return XCPKGReceiptKeyCode_fix_opt;
    } else if (strcmp(key, "res-url") == 0) {
        return XCPKGReceiptKeyCode_res_url;
    } else if (strcmp(key, "res-uri") == 0) {
        return XCPKGReceiptKeyCode_res_uri;
    } else if (strcmp(key, "res-sha") == 0) {
        return XCPKGReceiptKeyCode_res_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return XCPKGReceiptKeyCode_dep_pkg;
    } else if (strcmp(key, "dep-upp") == 0) {
        return XCPKGReceiptKeyCode_dep_upp;
    } else if (strcmp(key, "dep-pym") == 0) {
        return XCPKGReceiptKeyCode_dep_pym;
    } else if (strcmp(key, "dep-plm") == 0) {
        return XCPKGReceiptKeyCode_dep_plm;
    } else if (strcmp(key, "ppflags") == 0) {
        return XCPKGReceiptKeyCode_ppflags;
    } else if (strcmp(key, "ccflags") == 0) {
        return XCPKGReceiptKeyCode_ccflags;
    } else if (strcmp(key, "xxflags") == 0) {
        return XCPKGReceiptKeyCode_xxflags;
    } else if (strcmp(key, "ldflags") == 0) {
        return XCPKGReceiptKeyCode_ldflags;
    } else if (strcmp(key, "dofetch") == 0) {
        return XCPKGReceiptKeyCode_dofetch;
    } else if (strcmp(key, "do12345") == 0) {
        return XCPKGReceiptKeyCode_do12345;
    } else if (strcmp(key, "dopatch") == 0) {
        return XCPKGReceiptKeyCode_dopatch;
    } else if (strcmp(key, "prepare") == 0) {
        return XCPKGReceiptKeyCode_prepare;
    } else if (strcmp(key, "install") == 0) {
        return XCPKGReceiptKeyCode_install;
    } else if (strcmp(key, "dotweak") == 0) {
        return XCPKGReceiptKeyCode_dotweak;
    } else if (strcmp(key, "bindenv") == 0) {
        return XCPKGReceiptKeyCode_bindenv;
    } else if (strcmp(key, "caveats") == 0) {
        return XCPKGReceiptKeyCode_caveats;
    } else if (strcmp(key, "symlink") == 0) {
        return XCPKGReceiptKeyCode_symlink;
    } else if (strcmp(key, "movable") == 0) {
        return XCPKGReceiptKeyCode_movable;
    } else if (strcmp(key, "ltoable") == 0) {
        return XCPKGReceiptKeyCode_ltoable;
    } else if (strcmp(key, "mslable") == 0) {
        return XCPKGReceiptKeyCode_mslable;
    } else if (strcmp(key, "bsystem") == 0) {
        return XCPKGReceiptKeyCode_bsystem;
    } else if (strcmp(key, "bscript") == 0) {
        return XCPKGReceiptKeyCode_bscript;
    } else if (strcmp(key, "binbstd") == 0) {
        return XCPKGReceiptKeyCode_binbstd;
    } else if (strcmp(key, "pkgtype") == 0) {
        return XCPKGReceiptKeyCode_pkgtype;
    } else if (strcmp(key, "patches") == 0) {
        return XCPKGReceiptKeyCode_patches;
    } else if (strcmp(key, "reslist") == 0) {
        return XCPKGReceiptKeyCode_reslist;
    } else if (strcmp(key, "parallel") == 0) {
        return XCPKGReceiptKeyCode_parallel;
    } else if (strcmp(key, "builtby") == 0) {
        return XCPKGReceiptKeyCode_builtby;
    } else if (strcmp(key, "builtat") == 0) {
        return XCPKGReceiptKeyCode_builtat;
    } else if (strcmp(key, "builtfor") == 0) {
        return XCPKGReceiptKeyCode_builtfor;
    } else {
        return XCPKGReceiptKeyCode_unknown;
    }
}

static int xcpkg_receipt_set_value(XCPKGReceiptKeyCode keyCode, char * value, XCPKGReceipt * receipt, int * pkgtype, int * binbstd, int * symlink, int * ltoable, int * mslable, int * movable, int * parallel) {
    if (keyCode == XCPKGReceiptKeyCode_unknown) {
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
        case XCPKGReceiptKeyCode_summary: if (receipt->summary != NULL) free(receipt->summary); receipt->summary = strdup(value); break;
        case XCPKGReceiptKeyCode_version: if (receipt->version != NULL) free(receipt->version); receipt->version = strdup(value); break;
        case XCPKGReceiptKeyCode_license: if (receipt->license != NULL) free(receipt->license); receipt->license = strdup(value); break;

        case XCPKGReceiptKeyCode_web_url: if (receipt->web_url != NULL) free(receipt->web_url); receipt->web_url = strdup(value); break;

        case XCPKGReceiptKeyCode_git_url: if (receipt->git_url != NULL) free(receipt->git_url); receipt->git_url = strdup(value); break;
        case XCPKGReceiptKeyCode_git_sha: if (receipt->git_sha != NULL) free(receipt->git_sha); receipt->git_sha = strdup(value); break;
        case XCPKGReceiptKeyCode_git_ref: if (receipt->git_ref != NULL) free(receipt->git_ref); receipt->git_ref = strdup(value); break;

        case XCPKGReceiptKeyCode_src_url: if (receipt->src_url != NULL) free(receipt->src_url); receipt->src_url = strdup(value); break;
        case XCPKGReceiptKeyCode_src_uri: if (receipt->src_uri != NULL) free(receipt->src_uri); receipt->src_uri = strdup(value); break;
        case XCPKGReceiptKeyCode_src_sha: if (receipt->src_sha != NULL) free(receipt->src_sha); receipt->src_sha = strdup(value); break;

        case XCPKGReceiptKeyCode_fix_url: if (receipt->fix_url != NULL) free(receipt->fix_url); receipt->fix_url = strdup(value); break;
        case XCPKGReceiptKeyCode_fix_uri: if (receipt->fix_uri != NULL) free(receipt->fix_uri); receipt->fix_uri = strdup(value); break;
        case XCPKGReceiptKeyCode_fix_sha: if (receipt->fix_sha != NULL) free(receipt->fix_sha); receipt->fix_sha = strdup(value); break;
        case XCPKGReceiptKeyCode_fix_opt: if (receipt->fix_opt != NULL) free(receipt->fix_opt); receipt->fix_opt = strdup(value); break;

        case XCPKGReceiptKeyCode_res_url: if (receipt->res_url != NULL) free(receipt->res_url); receipt->res_url = strdup(value); break;
        case XCPKGReceiptKeyCode_res_uri: if (receipt->res_uri != NULL) free(receipt->res_uri); receipt->res_uri = strdup(value); break;
        case XCPKGReceiptKeyCode_res_sha: if (receipt->res_sha != NULL) free(receipt->res_sha); receipt->res_sha = strdup(value); break;

        case XCPKGReceiptKeyCode_dep_pkg: if (receipt->dep_pkg != NULL) free(receipt->dep_pkg); receipt->dep_pkg = strdup(value); break;
        case XCPKGReceiptKeyCode_dep_upp: if (receipt->dep_upp != NULL) free(receipt->dep_upp); receipt->dep_upp = strdup(value); break;
        case XCPKGReceiptKeyCode_dep_pym: if (receipt->dep_pym != NULL) free(receipt->dep_pym); receipt->dep_pym = strdup(value); break;
        case XCPKGReceiptKeyCode_dep_plm: if (receipt->dep_plm != NULL) free(receipt->dep_plm); receipt->dep_plm = strdup(value); break;

        case XCPKGReceiptKeyCode_ppflags: if (receipt->ppflags != NULL) free(receipt->ppflags); receipt->ppflags = strdup(value); break;
        case XCPKGReceiptKeyCode_ccflags: if (receipt->ccflags != NULL) free(receipt->ccflags); receipt->ccflags = strdup(value); break;
        case XCPKGReceiptKeyCode_xxflags: if (receipt->xxflags != NULL) free(receipt->xxflags); receipt->xxflags = strdup(value); break;
        case XCPKGReceiptKeyCode_ldflags: if (receipt->ldflags != NULL) free(receipt->ldflags); receipt->ldflags = strdup(value); break;

        case XCPKGReceiptKeyCode_do12345: if (receipt->do12345 != NULL) free(receipt->do12345); receipt->do12345 = strdup(value); break;
        case XCPKGReceiptKeyCode_dofetch: if (receipt->dofetch != NULL) free(receipt->dofetch); receipt->dofetch = strdup(value); break;
        case XCPKGReceiptKeyCode_dopatch: if (receipt->dopatch != NULL) free(receipt->dopatch); receipt->dopatch = strdup(value); break;
        case XCPKGReceiptKeyCode_prepare: if (receipt->prepare != NULL) free(receipt->prepare); receipt->prepare = strdup(value); break;
        case XCPKGReceiptKeyCode_install: if (receipt->install != NULL) free(receipt->install); receipt->install = strdup(value); break;
        case XCPKGReceiptKeyCode_dotweak: if (receipt->dotweak != NULL) free(receipt->dotweak); receipt->dotweak = strdup(value); break;
        case XCPKGReceiptKeyCode_bindenv: if (receipt->bindenv != NULL) free(receipt->bindenv); receipt->bindenv = strdup(value); break;
        case XCPKGReceiptKeyCode_caveats: if (receipt->caveats != NULL) free(receipt->caveats); receipt->caveats = strdup(value); break;
        case XCPKGReceiptKeyCode_patches: if (receipt->patches != NULL) free(receipt->patches); receipt->patches = strdup(value); break;
        case XCPKGReceiptKeyCode_reslist: if (receipt->reslist != NULL) free(receipt->reslist); receipt->reslist = strdup(value); break;

        case XCPKGReceiptKeyCode_bsystem: if (receipt->bsystem != NULL) free(receipt->bsystem); receipt->bsystem = strdup(value); break;
        case XCPKGReceiptKeyCode_bscript: if (receipt->bscript != NULL) free(receipt->bscript); receipt->bscript = strdup(value); break;

        case XCPKGReceiptKeyCode_builtby: if (receipt->builtBy != NULL) free(receipt->builtBy); receipt->builtBy = strdup(value); break;
        case XCPKGReceiptKeyCode_builtat: if (receipt->builtAt != NULL) free(receipt->builtAt); receipt->builtAt = strdup(value); break;
        case XCPKGReceiptKeyCode_builtfor: if (receipt->builtFor != NULL) free(receipt->builtFor); receipt->builtFor = strdup(value); break;

        case XCPKGReceiptKeyCode_git_nth:
            for (int i = 0; ; i++) {
                if (value[i] == '\0') {
                    break;
                }
                if (value[i] < '0' || value[i] > '9') {
                    return XCPKG_ERROR_FORMULA_SCHEME;
                }
            }
            receipt->git_nth = atoi(value);
            break;
        case XCPKGReceiptKeyCode_binbstd:
            if (strcmp(value, "1") == 0) {
                *binbstd = 1;
            } else if (strcmp(value, "0") == 0) {
                *binbstd = 0;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case XCPKGReceiptKeyCode_symlink:
            if (strcmp(value, "1") == 0) {
                *symlink = true;
            } else if (strcmp(value, "0") == 0) {
                *symlink = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case XCPKGReceiptKeyCode_movable:
            if (strcmp(value, "1") == 0) {
                *movable = true;
            } else if (strcmp(value, "0") == 0) {
                *movable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;
        case XCPKGReceiptKeyCode_pkgtype:
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

        case XCPKGReceiptKeyCode_ltoable:
            if (strcmp(value, "1") == 0) {
                *ltoable = true;
            } else if (strcmp(value, "0") == 0) {
                *ltoable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case XCPKGReceiptKeyCode_mslable:
            if (strcmp(value, "1") == 0) {
                *mslable = true;
            } else if (strcmp(value, "0") == 0) {
                *mslable = false;
            } else {
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
            break;

        case XCPKGReceiptKeyCode_parallel:
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

static int xcpkg_receipt_check(XCPKGReceipt * receipt, const char * receiptFilePath) {
    //xcpkg_receipt_dump(receipt);
    if (receipt->summary == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : summary mapping not found.\n", receiptFilePath);
        return XCPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->version == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : version mapping not found.\n", receiptFilePath);
        return XCPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->web_url == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : web-url mapping not found.\n", receiptFilePath);
        return XCPKG_ERROR_FORMULA_SCHEME;
    }

    if (receipt->src_url == NULL) {
        if (receipt->git_url == NULL) {
            fprintf(stderr, "scheme error in receipt file: %s : neither src-url nor git-url mapping not found.\n", receiptFilePath);
            return XCPKG_ERROR_FORMULA_SCHEME;
        }
    } else {
        if (strncmp(receipt->src_url, "dir://", 6) == 0) {
            ;
        } else {
            if (receipt->src_sha == NULL) {
                fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping not found.\n", receiptFilePath);
                return XCPKG_ERROR_FORMULA_SCHEME;
            }

            if (strlen(receipt->src_sha) != 64) {
                fprintf(stderr, "scheme error in receipt file: %s : src-sha mapping's value's length must be 64.\n", receiptFilePath);
                return XCPKG_ERROR_FORMULA_SCHEME;
            }
        }
    }

    if (receipt->builtBy == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : builtby mapping not found.\n", receiptFilePath);
        return XCPKG_ERROR_RECEIPT_SCHEME;
    }

    if (receipt->builtFor == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : builtfor mapping not found.\n", receiptFilePath);
        return XCPKG_ERROR_RECEIPT_SCHEME;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (receipt->builtAt == NULL) {
        fprintf(stderr, "scheme error in receipt file: %s : builtat mapping not found.\n", receiptFilePath);
        return XCPKG_ERROR_RECEIPT_SCHEME;
    }

    size_t i = 0;

    for (;; i++) {
        char c = receipt->builtAt[i];

        if (c == '\0') {
            break;
        }

        if ((c < '0') || (c > '9')) {
            fprintf(stderr, "scheme error in receipt file: %s : builtAt mapping's value should only contains non-numeric characters.\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SCHEME;
        }
    }

    if (i != 10U) {
        fprintf(stderr, "scheme error in receipt file: %s : builtAt mapping's value's length must be 10.\n", receiptFilePath);
        return XCPKG_ERROR_RECEIPT_SCHEME;
    }

    return XCPKG_OK;
}

int xcpkg_receipt_parse(const char * packageName, const char * targetPlatformSpec, XCPKGReceipt * * out) {
    int ret = xcpkg_check_if_the_given_argument_matches_package_name_pattern(packageName);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (targetPlatformSpec == NULL) {
        return XCPKG_ERROR_ARG_IS_NULL;
    }

    char   xcpkgHomeDIR[PATH_MAX];
    size_t xcpkgHomeDIRLength;

    ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

    if (ret != XCPKG_OK) {
        return ret;
    }

    size_t receiptFilePathLength = xcpkgHomeDIRLength + strlen(targetPlatformSpec) + strlen(packageName) + sizeof(XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT) + 14U;
    char * receiptFilePath = (char*)calloc(receiptFilePathLength, sizeof(char));

    if (receiptFilePath == NULL) {
        return XCPKG_ERROR_MEMORY_ALLOCATE;
    }

    if (snprintf(receiptFilePath, receiptFilePathLength, "%s/installed/%s/%s%s", xcpkgHomeDIR, targetPlatformSpec, packageName, XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT) < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    FILE * file = fopen(receiptFilePath, "r");

    if (file == NULL) {
        perror(receiptFilePath);
        free(receiptFilePath);
        return XCPKG_ERROR_PACKAGE_NOT_INSTALLED;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        free(receiptFilePath);
        return XCPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, file);

    XCPKGReceiptKeyCode receiptKeyCode = XCPKGReceiptKeyCode_unknown;

    XCPKGReceipt * receipt = NULL;

    int lastTokenType = 0;

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
            fprintf(stderr, "syntax error in receipt file: %s\n", receiptFilePath);
            ret = XCPKG_ERROR_RECEIPT_SYNTAX;
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
                    receiptKeyCode = xcpkg_receipt_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (receipt == NULL) {
                        receipt = (XCPKGReceipt*)calloc(1, sizeof(XCPKGReceipt));

                        if (receipt == NULL) {
                            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        receipt->path = receiptFilePath;
                    }

                    ret = xcpkg_receipt_set_value(receiptKeyCode, (char*)token.data.scalar.value, receipt, &pkgtype, &binbstd, &symlink, &ltoable, &mslable, &movable, &parallel);

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

    // xcpkg_receipt_dump(receipt);

    if (ret == XCPKG_OK) {
        if (pkgtype == -1) {
            fprintf(stderr, "no pkgtype mapping in receipt file: %s\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SYNTAX;
        }

        if (binbstd == -1) {
            fprintf(stderr, "no binbstd mapping in receipt file: %s\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SYNTAX;
        }

        if (symlink == -1) {
            fprintf(stderr, "no symlink mapping in receipt file: %s\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SYNTAX;
        }

        if (ltoable == -1) {
            fprintf(stderr, "no ltoable mapping in receipt file: %s\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SYNTAX;
        }

        if (mslable == -1) {
            fprintf(stderr, "no mslable mapping in receipt file: %s\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SYNTAX;
        }

        if (movable == -1) {
            fprintf(stderr, "no movable mapping in receipt file: %s\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SYNTAX;
        }

        if (parallel == -1) {
            fprintf(stderr, "no parallel mapping in receipt file: %s\n", receiptFilePath);
            return XCPKG_ERROR_RECEIPT_SYNTAX;
        }

        receipt->pkgtype = pkgtype;
        receipt->binbstd = binbstd;
        receipt->symlink = symlink;
        receipt->ltoable = ltoable;
        receipt->support_create_mostly_statically_linked_executable = mslable;
        receipt->movable = movable;
        receipt->support_build_in_parallel = parallel;

        ret = xcpkg_receipt_check(receipt, receiptFilePath);

        if (ret == XCPKG_OK) {
            (*out) = receipt;
            return XCPKG_OK;
        }
    }

    if (receipt == NULL) {
        free(receiptFilePath);
    } else {
        xcpkg_receipt_free(receipt);
    }

    return ret;
}
