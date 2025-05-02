#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include <jansson.h>

#include "core/log.h"

#include "xcpkg.h"

static const char * trim(const char * p) {
    if (p != NULL) {
        while (p[0] <= 32) {
            p++;
        }
    }

    return p;
}

int xcpkg_available_info2(const XCPKGFormula * formula, const char * packageName, const char * targetPlatformName, const char * key) {
    if ((key == NULL) || (key[0] == '\0') || (strcmp(key, "--yaml") == 0)) {
        char formulaFilePath[PATH_MAX];

        int ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }

        FILE * formulaFile = fopen(formulaFilePath, "r");

        if (formulaFile == NULL) {
            perror(formulaFilePath);
            return XCPKG_ERROR;
        }

        if (isatty(STDOUT_FILENO)) {
            printf("pkgname: %s%s%s\n", COLOR_GREEN, packageName, COLOR_OFF);
        } else {
            printf("pkgname: %s\n", packageName);
        }

        char   buff[1024];
        size_t size;

        for (;;) {
            size = fread(buff, 1, 1024, formulaFile);

            if (ferror(formulaFile)) {
                perror(formulaFilePath);
                fclose(formulaFile);
                return XCPKG_ERROR;
            }

            if (size > 0) {
                if (fwrite(buff, 1, size, stdout) != size || ferror(stdout)) {
                    perror(NULL);
                    fclose(formulaFile);
                    return XCPKG_ERROR;
                }
            }

            if (feof(formulaFile)) {
                fclose(formulaFile);
                break;
            }
        }

        printf("formula: %s\n", formulaFilePath);
    } else if (strcmp(key, "--json") == 0) {
        json_t * root = json_object();

        const char * pkgtype;

        switch (formula->pkgtype) {
            case XCPKGPkgType_lib: pkgtype = "lib"; break;
            case XCPKGPkgType_exe: pkgtype = "exe"; break;
        }

        json_object_set_new(root, "pkgtype", json_string(pkgtype));
        json_object_set_new(root, "pkgname", json_string(packageName));

        json_object_set_new(root, "summary", json_string(formula->summary));
        json_object_set_new(root, "version", json_string(formula->version));
        json_object_set_new(root, "license", json_string(formula->license));

        json_object_set_new(root, "web-url", json_string(formula->web_url));

        json_object_set_new(root, "git-url", json_string(formula->git_url));
        json_object_set_new(root, "git-sha", json_string(formula->git_sha));
        json_object_set_new(root, "git-ref", json_string(formula->git_ref));
        json_object_set_new(root, "git-nth", json_integer(formula->git_nth));

        json_object_set_new(root, "src-url", json_string(formula->src_url));
        json_object_set_new(root, "src-uri", json_string(formula->src_uri));
        json_object_set_new(root, "src-sha", json_string(formula->src_sha));

        json_object_set_new(root, "fix-url", json_string(formula->fix_url));
        json_object_set_new(root, "fix-uri", json_string(formula->fix_uri));
        json_object_set_new(root, "fix-sha", json_string(formula->fix_sha));
        json_object_set_new(root, "fix-opt", json_string(formula->fix_opt));

        json_object_set_new(root, "res-url", json_string(formula->res_url));
        json_object_set_new(root, "res-uri", json_string(formula->res_uri));
        json_object_set_new(root, "res-sha", json_string(formula->res_sha));

        json_object_set_new(root, "dep-pkg", json_string(formula->dep_pkg));
        json_object_set_new(root, "dep-upp", json_string(formula->dep_upp));
        json_object_set_new(root, "dep-pym", json_string(formula->dep_pym));
        json_object_set_new(root, "dep-plm", json_string(formula->dep_plm));

        json_object_set_new(root, "ppflags", json_string(formula->ppflags));
        json_object_set_new(root, "ccflags", json_string(formula->ccflags));
        json_object_set_new(root, "xxflags", json_string(formula->xxflags));
        json_object_set_new(root, "ldfalgs", json_string(formula->ldflags));

        json_object_set_new(root, "bsystem", json_string(formula->bsystem));
        json_object_set_new(root, "bscript", json_string(formula->bscript));
        json_object_set_new(root, "binbstd", json_boolean(formula->binbstd));
        json_object_set_new(root, "ltoable", json_boolean(formula->ltoable));
        json_object_set_new(root, "movable", json_boolean(formula->movable));
        json_object_set_new(root, "mslable", json_boolean(formula->support_create_mostly_statically_linked_executable));
        json_object_set_new(root, "symlink", json_boolean(formula->symlink));
        json_object_set_new(root, "parallel", json_boolean(formula->support_build_in_parallel));

        json_object_set_new(root, "dofetch", json_string(formula->dofetch));
        json_object_set_new(root, "do12345", json_string(formula->do12345));
        json_object_set_new(root, "dopatch", json_string(formula->dopatch));
        json_object_set_new(root, "prepare", json_string(formula->prepare));
        json_object_set_new(root, "install", json_string(formula->install));
        json_object_set_new(root, "dotweak", json_string(formula->dotweak));

        json_object_set_new(root, "bindenv", json_string(formula->bindenv));
        json_object_set_new(root, "caveats", json_string(formula->caveats));
        json_object_set_new(root, "patches", json_string(formula->patches));

        char * jsonStr = json_dumps(root, 0);

        int ret = XCPKG_OK;

        if (jsonStr == NULL) {
            ret = XCPKG_ERROR;
        } else {
            printf("%s\n", jsonStr);
            free(jsonStr);
        }

        json_decref(root);

        return ret;
    } else if (strcmp(key, "summary") == 0) {
        if (formula->summary != NULL) {
            printf("%s\n", formula->summary);
        }
    } else if (strcmp(key, "version") == 0) {
        if (formula->version != NULL) {
            printf("%s\n", formula->version);
        }
    } else if (strcmp(key, "license") == 0) {
        if (formula->license != NULL) {
            printf("%s\n", formula->license);
        }
    } else if (strcmp(key, "web-url") == 0) {
        if (formula->web_url != NULL) {
            printf("%s\n", formula->web_url);
        }
    } else if (strcmp(key, "git-url") == 0) {
        if (formula->git_url != NULL) {
            printf("%s\n", formula->git_url);
        }
    } else if (strcmp(key, "git-sha") == 0) {
        if (formula->git_sha != NULL) {
            printf("%s\n", formula->git_sha);
        }
    } else if (strcmp(key, "git-ref") == 0) {
        if (formula->git_ref != NULL) {
            printf("%s\n", formula->git_ref);
        }
    } else if (strcmp(key, "git-nth") == 0) {
        printf("%zu\n", formula->git_nth);
    } else if (strcmp(key, "src-url") == 0) {
        if (formula->src_url != NULL) {
            printf("%s\n", formula->src_url);
        }
    } else if (strcmp(key, "src-sha") == 0) {
        if (formula->src_sha != NULL) {
            printf("%s\n", formula->src_sha);
        }
    } else if (strcmp(key, "src-ft") == 0) {
        char fileNameExtension[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

        int ret = xcpkg_extract_filetype_from_url(formula->src_url, fileNameExtension, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }

        printf("%s\n", fileNameExtension);
    } else if (strcmp(key, "src-fp") == 0) {
        char   xcpkgHomeDIR[PATH_MAX];
        size_t xcpkgHomeDIRLength;

        int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        char fileNameExtension[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

        ret = xcpkg_extract_filetype_from_url(formula->src_url, fileNameExtension, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }

        printf("%s/downloads/%s%s\n", xcpkgHomeDIR, formula->src_sha, fileNameExtension);
    } else if (strcmp(key, "fix-url") == 0) {
        if (formula->fix_url != NULL) {
            printf("%s\n", formula->fix_url);
        }
    } else if (strcmp(key, "fix-sha") == 0) {
        if (formula->fix_sha != NULL) {
            printf("%s\n", formula->fix_sha);
        }
    } else if (strcmp(key, "fix-ft") == 0) {
        char fileNameExtension[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

        int ret = xcpkg_extract_filetype_from_url(formula->fix_url, fileNameExtension, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }

        printf("%s\n", fileNameExtension);
    } else if (strcmp(key, "fix-fp") == 0) {
        char   xcpkgHomeDIR[PATH_MAX];
        size_t xcpkgHomeDIRLength;

        int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        char fileNameExtension[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

        ret = xcpkg_extract_filetype_from_url(formula->fix_url, fileNameExtension, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }

        printf("%s/downloads/%s%s\n", xcpkgHomeDIR, formula->fix_sha, fileNameExtension);
    } else if (strcmp(key, "res-url") == 0) {
        if (formula->res_url != NULL) {
            printf("%s\n", formula->res_url);
        }
    } else if (strcmp(key, "res-sha") == 0) {
        if (formula->res_sha != NULL) {
            printf("%s\n", formula->res_sha);
        }
    } else if (strcmp(key, "res-ft") == 0) {
        char fileNameExtension[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

        int ret = xcpkg_extract_filetype_from_url(formula->res_url, fileNameExtension, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }

        printf("%s\n", fileNameExtension);
    } else if (strcmp(key, "res-fp") == 0) {
        char   xcpkgHomeDIR[PATH_MAX];
        size_t xcpkgHomeDIRLength;

        int ret = xcpkg_home_dir(xcpkgHomeDIR, &xcpkgHomeDIRLength);

        if (ret != XCPKG_OK) {
            return ret;
        }

        char fileNameExtension[XCPKG_FILE_EXTENSION_MAX_CAPACITY] = {0};

        ret = xcpkg_extract_filetype_from_url(formula->res_url, fileNameExtension, XCPKG_FILE_EXTENSION_MAX_CAPACITY);

        if (ret != XCPKG_OK) {
            return ret;
        }

        printf("%s/downloads/%s%s\n", xcpkgHomeDIR, formula->res_sha, fileNameExtension);
    } else if (strcmp(key, "dep-pkg") == 0) {
        if (formula->dep_pkg != NULL) {
            printf("%s\n", formula->dep_pkg);
        }
    } else if (strcmp(key, "dep-upp") == 0) {
        if (formula->dep_upp != NULL) {
            printf("%s\n", formula->dep_upp);
        }
    } else if (strcmp(key, "dep-pym") == 0) {
        if (formula->dep_pym != NULL) {
            printf("%s\n", formula->dep_pym);
        }
    } else if (strcmp(key, "dep-plm") == 0) {
        if (formula->dep_plm != NULL) {
            printf("%s\n", formula->dep_plm);
        }
    } else if (strcmp(key, "bsystem") == 0) {
        if (formula->bsystem != NULL) {
            printf("%s\n", formula->bsystem);
        }
    } else if (strcmp(key, "bscript") == 0) {
        if (formula->bscript != NULL) {
            printf("%s\n", formula->bscript);
        }
    } else if (strcmp(key, "binbstd") == 0) {
        printf("%d\n", formula->binbstd);
    } else if (strcmp(key, "movable") == 0) {
        printf("%d\n", formula->movable);
    } else if (strcmp(key, "symlink") == 0) {
        printf("%d\n", formula->symlink);
    } else if (strcmp(key, "ppflags") == 0) {
        if (formula->ppflags != NULL) {
            printf("%s\n", formula->ppflags);
        }
    } else if (strcmp(key, "ccflags") == 0) {
        if (formula->ccflags != NULL) {
            printf("%s\n", formula->ccflags);
        }
    } else if (strcmp(key, "xxflags") == 0) {
        if (formula->xxflags != NULL) {
            printf("%s\n", formula->xxflags);
        }
    } else if (strcmp(key, "ldflags") == 0) {
        if (formula->ldflags != NULL) {
            printf("%s\n", formula->ldflags);
        }
    } else if (strcmp(key, "do12345") == 0) {
        if (formula->do12345 != NULL) {
            printf("%s\n", formula->do12345);
        }
    } else if (strcmp(key, "dofetch") == 0) {
        if (formula->dofetch != NULL) {
            printf("%s\n", formula->dofetch);
        }
    } else if (strcmp(key, "dopatch") == 0) {
        if (formula->dopatch != NULL) {
            printf("%s\n", formula->dopatch);
        }
    } else if (strcmp(key, "prepare") == 0) {
        if (formula->prepare != NULL) {
            printf("%s\n", formula->prepare);
        }
    } else if (strcmp(key, "install") == 0) {
        if (formula->install != NULL) {
            printf("%s\n", formula->install);
        }
    } else if (strcmp(key, "dotweak") == 0) {
        if (formula->dotweak != NULL) {
            printf("%s\n", formula->dotweak);
        }
    } else if (strcmp(key, "patches") == 0) {
        if (formula->patches != NULL) {
            printf("%s\n", formula->patches);
        }
    } else if (strcmp(key, "caveats") == 0) {
        if (formula->caveats != NULL) {
            printf("%s\n", formula->caveats);
        }
    } else {
        return XCPKG_ERROR_ARG_IS_UNKNOWN;
    }

    return XCPKG_OK;
}

int xcpkg_available_info(const char * packageName, const char * targetPlatformName, const char * key) {
    if (key != NULL && key[0] != '\0' && strcmp(key, "formula") == 0) {
        char formulaFilePath[PATH_MAX];

        int ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

        if (ret != XCPKG_OK) {
            return ret;
        }

        printf("%s\n", formulaFilePath);

        return XCPKG_OK;
    } else {
        XCPKGFormula * formula = NULL;

        int ret = xcpkg_formula_load(packageName, targetPlatformName, NULL, &formula);

        if (ret != XCPKG_OK) {
            return ret;
        }

        ret = xcpkg_available_info2(formula, packageName, targetPlatformName, key);

        xcpkg_formula_free(formula);

        return ret;
    }
}
