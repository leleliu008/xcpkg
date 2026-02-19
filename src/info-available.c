#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>

#include <yaml.h>
#include <jansson.h>

#include "core/log.h"

#include "xcpkg.h"

typedef struct {
    const char * key;
    const char * value;
} KV;

typedef struct {
    const char * key;
    const bool value;
} KB;


int xcpkg_available_info2(const XCPKGFormula * formula, const char * packageName, const char * targetPlatformName, const char * key) {
    if ((key == NULL) || (key[0] == '\0') || (strcmp(key, "--yaml") == 0)) {
        if (isatty(STDOUT_FILENO)) {
            printf("pkgname: %s%s%s\n", COLOR_GREEN, packageName, COLOR_OFF);
        } else {
            printf("pkgname: %s\n", packageName);
        }

        const char * pkgtype;

        switch (formula->pkgtype) {
            case XCPKGPkgType_lib: pkgtype = "lib"; break;
            case XCPKGPkgType_exe: pkgtype = "exe"; break;
        }

        printf("pkgtype: %s\n", pkgtype);

        yaml_emitter_t emitter;
        yaml_event_t event;

        if (!yaml_emitter_initialize(&emitter)) {
            fprintf(stderr, "libyaml init failed.");
            return XCPKG_ERROR;
        }

        yaml_emitter_set_output_file(&emitter, stdout);

        if (!yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING)) goto error;
        if (!yaml_emitter_emit(&emitter, &event)) goto error;

        if (!yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 1)) goto error;
        if (!yaml_emitter_emit(&emitter, &event)) goto error;

        if (!yaml_mapping_start_event_initialize(&event, NULL, (yaml_char_t *)YAML_MAP_TAG, 1, YAML_ANY_MAPPING_STYLE)) goto error;
        if (!yaml_emitter_emit(&emitter, &event)) goto error;

        ///////////////////////////////////////////////////////////////

        KV kvs[] = {
            {"version", formula->version},
            {"license", formula->license},
            {"summary", formula->summary},
            {"web-url", formula->web_url},

            {"git-url", formula->git_url},
            {"git-uri", formula->git_uri},
            {"git-sha", formula->git_sha},
            {"git-ref", formula->git_ref},

            {"src-url", formula->src_url},
            {"src-uri", formula->src_uri},
            {"src-sha", formula->src_sha},

            {"fix-url", formula->fix_url},
            {"fix-uri", formula->fix_uri},
            {"fix-sha", formula->fix_sha},
            {"fix-opt", formula->fix_opt},

            {"res-url", formula->res_url},
            {"res-uri", formula->res_uri},
            {"res-sha", formula->res_sha},

            {"dep-pkg", formula->dep_pkg},
            {"dep-lib", formula->dep_lib},
            {"dep-upp", formula->dep_upp},
            {"dep-pip", formula->dep_pip},
            {"dep-plm", formula->dep_plm},

            {"ccflags", formula->ccflags},
            {"xxflags", formula->xxflags},
            {"ppflags", formula->ppflags},
            {"ldflags", formula->ldflags},

            {"bsystem", formula->bsystem},
            {"bscript", formula->bscript},

            {"dofetch", formula->dofetch},
            {"do12345", formula->do12345},
            {"dopatch", formula->dopatch},
            {"prepare", formula->prepare},
            {"install", formula->install},
            {"dotweak", formula->dotweak},

            {"bindenv", formula->bindenv},
            {"caveats", formula->caveats},

            {"patches", formula->patches},
            {"reslist", formula->reslist},

            {NULL, NULL}
        };

        for (int i = 0; ; i++) {
            const char * key = kvs[i].key;
            const char * value = kvs[i].value;

            if (key == NULL) {
                break;
            }

            if (value != NULL) {
                if (!yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG, (yaml_char_t *)key, strlen(key), 1, 0, YAML_PLAIN_SCALAR_STYLE)) goto error;
                if (!yaml_emitter_emit(&emitter, &event)) goto error;

                yaml_scalar_style_t style = YAML_PLAIN_SCALAR_STYLE;

                for (int j = 0; ; j++) {
                    if (value[j] == '\0') {
                        break;
                    }

                    if (value[j] == '\n') {
                        style = YAML_LITERAL_SCALAR_STYLE;
                        break;
                    }
                }

                if (!yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG, (yaml_char_t *)value, strlen(value), 1, 0, style)) goto error;
                if (!yaml_emitter_emit(&emitter, &event)) goto error;
            }
        }

        ///////////////////////////////////////////////////////////////

        KB kbs[] = {
            {"binbstd", formula->binbstd},
            {"ltoable", formula->ltoable},
            {"movable", formula->movable},
            {"mslable", formula->support_create_mostly_statically_linked_executable},
            {"symlink", formula->symlink},
            {"parallel", formula->support_build_in_parallel},
            {NULL, false}
        };

        for (int i = 0; ; i++) {
            const char * key = kbs[i].key;
            const bool value = kbs[i].value;

            if (key == NULL) {
                break;
            }

            const char * value2 = value ? "true" : "false";

            if (!yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG, (yaml_char_t *)key, strlen(key), 1, 0, YAML_PLAIN_SCALAR_STYLE)) goto error;
            if (!yaml_emitter_emit(&emitter, &event)) goto error;

            if (!yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG, (yaml_char_t *)value2, strlen(value2), 1, 0, YAML_ANY_SCALAR_STYLE)) goto error;
            if (!yaml_emitter_emit(&emitter, &event)) goto error;
        }

        ///////////////////////////////////////////////////////////////

        if (!yaml_mapping_end_event_initialize(&event)) goto error;
        if (!yaml_emitter_emit(&emitter, &event)) goto error;

        if (!yaml_document_end_event_initialize(&event, 1)) goto error;
        if (!yaml_emitter_emit(&emitter, &event)) goto error;

        if (!yaml_stream_end_event_initialize(&event)) goto error;
        if (!yaml_emitter_emit(&emitter, &event)) goto error;

        yaml_emitter_delete(&emitter);
        return XCPKG_OK;

        error:
        fprintf(stderr, "Failed to emit event %d: %s\n", event.type, emitter.problem);
        yaml_emitter_delete(&emitter);
        return XCPKG_ERROR;
    } else if (strcmp(key, "--json") == 0) {
        const char * pkgtype;

        switch (formula->pkgtype) {
            case XCPKGPkgType_lib: pkgtype = "lib"; break;
            case XCPKGPkgType_exe: pkgtype = "exe"; break;
        }

        json_t * root = json_object();

        json_object_set_new(root, "pkgname", json_string(packageName));
        json_object_set_new(root, "pkgtype", json_string(pkgtype));

        json_object_set_new(root, "summary", json_string(formula->summary));
        json_object_set_new(root, "version", json_string(formula->version));
        json_object_set_new(root, "license", json_string(formula->license));

        json_object_set_new(root, "web-url", json_string(formula->web_url));

        json_object_set_new(root, "git-url", json_string(formula->git_url));
        json_object_set_new(root, "git-uri", json_string(formula->git_uri));
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
        json_object_set_new(root, "dep-lib", json_string(formula->dep_lib));
        json_object_set_new(root, "dep-upp", json_string(formula->dep_upp));
        json_object_set_new(root, "dep-pip", json_string(formula->dep_pip));
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
        json_object_set_new(root, "reslist", json_string(formula->reslist));

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
    } else if (strcmp(key, "pkgtype") == 0) {
        const char * pkgtype;

        switch (formula->pkgtype) {
            case XCPKGPkgType_lib: pkgtype = "lib"; break;
            case XCPKGPkgType_exe: pkgtype = "exe"; break;
        }

        printf("%s\n", pkgtype);
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
    } else if (strcmp(key, "git-uri") == 0) {
        if (formula->git_uri != NULL) {
            printf("%s\n", formula->git_uri);
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
    } else if (strcmp(key, "src-uri") == 0) {
        if (formula->src_uri != NULL) {
            printf("%s\n", formula->src_uri);
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
    } else if (strcmp(key, "fix-uri") == 0) {
        if (formula->fix_uri != NULL) {
            printf("%s\n", formula->fix_uri);
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
    } else if (strcmp(key, "res-uri") == 0) {
        if (formula->res_uri != NULL) {
            printf("%s\n", formula->res_uri);
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
    } else if (strcmp(key, "dep-lib") == 0) {
        if (formula->dep_lib != NULL) {
            printf("%s\n", formula->dep_lib);
        }
    } else if (strcmp(key, "dep-upp") == 0) {
        if (formula->dep_upp != NULL) {
            printf("%s\n", formula->dep_upp);
        }
    } else if (strcmp(key, "dep-pip") == 0) {
        if (formula->dep_pip != NULL) {
            printf("%s\n", formula->dep_pip);
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
    } else if (strcmp(key, "ltoable") == 0) {
        printf("%d\n", formula->ltoable);
    } else if (strcmp(key, "movable") == 0) {
        printf("%d\n", formula->movable);
    } else if (strcmp(key, "mslable") == 0) {
        printf("%d\n", formula->support_create_mostly_statically_linked_executable);
    } else if (strcmp(key, "symlink") == 0) {
        printf("%d\n", formula->symlink);
    } else if (strcmp(key, "parallel") == 0) {
        printf("%d\n", formula->support_build_in_parallel);
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
    } else if (strcmp(key, "reslist") == 0) {
        if (formula->reslist != NULL) {
            printf("%s\n", formula->reslist);
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
