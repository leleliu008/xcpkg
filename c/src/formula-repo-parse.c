#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <yaml.h>

#include "xcpkg.h"

typedef enum {
    XCPKGFormulaRepoKeyCode_unknown,
    XCPKGFormulaRepoKeyCode_url,
    XCPKGFormulaRepoKeyCode_branch,
    XCPKGFormulaRepoKeyCode_pinned,
    XCPKGFormulaRepoKeyCode_enabled,
    XCPKGFormulaRepoKeyCode_createdAt,
    XCPKGFormulaRepoKeyCode_updatedAt
} XCPKGFormulaRepoKeyCode;

void xcpkg_formula_repo_dump(XCPKGFormulaRepo * formulaRepo) {
    if (formulaRepo == NULL) {
        return;
    }

    printf("name: %s\n", formulaRepo->name);
    printf("path: %s\n", formulaRepo->path);
    printf("url:  %s\n", formulaRepo->url);
    printf("branch: %s\n", formulaRepo->branch);
    printf("pinned: %s\n", formulaRepo->pinned ? "yes" : "no");
    printf("enabled: %s\n", formulaRepo->enabled ? "yes" : "no");
    printf("created: %s\n", formulaRepo->createdAt);
    printf("updated: %s\n", formulaRepo->updatedAt);
}

void xcpkg_formula_repo_free(XCPKGFormulaRepo * formulaRepo) {
    if (formulaRepo == NULL) {
        return;
    }

    if (formulaRepo->name != NULL) {
        free(formulaRepo->name);
        formulaRepo->name = NULL;
    }

    if (formulaRepo->path != NULL) {
        free(formulaRepo->path);
        formulaRepo->path = NULL;
    }

    if (formulaRepo->url != NULL) {
        free(formulaRepo->url);
        formulaRepo->url = NULL;
    }

    if (formulaRepo->branch != NULL) {
        free(formulaRepo->branch);
        formulaRepo->branch = NULL;
    }

    if (formulaRepo->createdAt != NULL) {
        free(formulaRepo->createdAt);
        formulaRepo->createdAt = NULL;
    }

    if (formulaRepo->updatedAt != NULL) {
        free(formulaRepo->updatedAt);
        formulaRepo->updatedAt = NULL;
    }

    free(formulaRepo);
}

static XCPKGFormulaRepoKeyCode xcpkg_formula_repo_key_code_from_key_name(char * key) {
           if (strcmp(key, "url") == 0) {
        return XCPKGFormulaRepoKeyCode_url;
    } else if (strcmp(key, "branch") == 0) {
        return XCPKGFormulaRepoKeyCode_branch;
    } else if (strcmp(key, "pinned") == 0) {
        return XCPKGFormulaRepoKeyCode_pinned;
    } else if (strcmp(key, "enabled") == 0) {
        return XCPKGFormulaRepoKeyCode_enabled;
    } else if (strcmp(key, "created") == 0) {
        return XCPKGFormulaRepoKeyCode_createdAt;
    } else if (strcmp(key, "updated") == 0) {
        return XCPKGFormulaRepoKeyCode_updatedAt;
    } else {
        return XCPKGFormulaRepoKeyCode_unknown;
    }
}

static int xcpkg_formula_repo_set_value(XCPKGFormulaRepoKeyCode keyCode, char * value, XCPKGFormulaRepo * formulaRepo) {
    if (keyCode == XCPKGFormulaRepoKeyCode_unknown) {
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
        case XCPKGFormulaRepoKeyCode_url:
            if (formulaRepo->url != NULL) {
                free(formulaRepo->url);
            }

            formulaRepo->url = strdup(value);

            return formulaRepo->url == NULL ? XCPKG_ERROR_MEMORY_ALLOCATE : XCPKG_OK;
        case XCPKGFormulaRepoKeyCode_branch:
            if (formulaRepo->branch != NULL) {
                free(formulaRepo->branch);
            }

            formulaRepo->branch = strdup(value);

            return formulaRepo->branch == NULL ? XCPKG_ERROR_MEMORY_ALLOCATE : XCPKG_OK;
        case XCPKGFormulaRepoKeyCode_createdAt:
            if (formulaRepo->createdAt != NULL) {
                free(formulaRepo->createdAt);
            }

            formulaRepo->createdAt = strdup(value);

            return formulaRepo->createdAt == NULL ? XCPKG_ERROR_MEMORY_ALLOCATE : XCPKG_OK;
        case XCPKGFormulaRepoKeyCode_updatedAt:
            free(formulaRepo->updatedAt);
            formulaRepo->updatedAt = strdup(value);

            return formulaRepo->updatedAt == NULL ? XCPKG_ERROR_MEMORY_ALLOCATE : XCPKG_OK;
        case XCPKGFormulaRepoKeyCode_pinned:
            if (strcmp(value, "1") == 0) {
                formulaRepo->pinned = 1;
                return XCPKG_OK;
            } else if (strcmp(value, "0") == 0) {
                formulaRepo->pinned = 0;
                return XCPKG_OK;
            } else {
                return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
            }
        case XCPKGFormulaRepoKeyCode_enabled:
            if (strcmp(value, "1") == 0) {
                formulaRepo->enabled = 1;
                return XCPKG_OK;
            } else if (strcmp(value, "0") == 0) {
                formulaRepo->enabled = 0;
                return XCPKG_OK;
            } else {
                return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
            }
        case XCPKGFormulaRepoKeyCode_unknown:
        default:
            return XCPKG_OK;
    }
}

static int xcpkg_formula_repo_check(XCPKGFormulaRepo * formulaRepo, const char * formulaRepoConfigFilePath) {
    if (formulaRepo->url == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : summary mapping not found.\n", formulaRepoConfigFilePath);
        return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    if (formulaRepo->branch == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : summary mapping not found.\n", formulaRepoConfigFilePath);
        return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepo->createdAt == NULL) {
        fprintf(stderr, "scheme error in formula repo config file: %s : created mapping not found.\n", formulaRepoConfigFilePath);
        return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    size_t i = 0;
    char   c;

    for (;; i++) {
        c = formulaRepo->createdAt[i];

        if (c == '\0') {
            break;
        }

        if ((c < '0') || (c > '9')) {
            fprintf(stderr, "scheme error in formula repo config file: %s : created mapping's value should only contains non-numeric characters.\n", formulaRepoConfigFilePath);
            return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
        }
    }

    if (i != 10) {
        fprintf(stderr, "scheme error in formula repo config file: %s : created mapping's value's length must be 10.\n", formulaRepoConfigFilePath);
        return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (formulaRepo->updatedAt != NULL) {
        i = 0;

        for (;; i++) {
            c = formulaRepo->updatedAt[i];

            if (c == '\0') {
                break;
            }

            if ((c < '0') || (c > '9')) {
                fprintf(stderr, "scheme error in formula repo config file: %s : updated mapping's value should only contains non-numeric characters.\n", formulaRepoConfigFilePath);
                return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
            }
        }

        if (i != 10) {
            fprintf(stderr, "scheme error in formula repo config file: %s : updated mapping's value's length must be 10.\n", formulaRepoConfigFilePath);
            return XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME;
        }
    }

    return XCPKG_OK;
}

int xcpkg_formula_repo_parse(const char * formulaRepoConfigFilePath, XCPKGFormulaRepo * * out) {
    FILE * file = fopen(formulaRepoConfigFilePath, "r");

    if (file == NULL) {
        perror(formulaRepoConfigFilePath);
        return XCPKG_ERROR;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        fclose(file);
        return XCPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, file);

    XCPKGFormulaRepoKeyCode formulaRepoKeyCode = XCPKGFormulaRepoKeyCode_unknown;

    XCPKGFormulaRepo * formulaRepo = NULL;

    int lastTokenType = 0;

    int ret = XCPKG_OK;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in formula repo config file: %s\n", formulaRepoConfigFilePath);
            ret = XCPKG_ERROR_FORMULA_REPO_CONFIG_SYNTAX;
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
                    formulaRepoKeyCode = xcpkg_formula_repo_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (formulaRepo == NULL) {
                        formulaRepo = (XCPKGFormulaRepo*)calloc(1, sizeof(XCPKGFormulaRepo));

                        if (formulaRepo == NULL) {
                            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        formulaRepo->enabled = 1;
                    }

                    ret = xcpkg_formula_repo_set_value(formulaRepoKeyCode, (char*)token.data.scalar.value, formulaRepo);

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
        ret = xcpkg_formula_repo_check(formulaRepo, formulaRepoConfigFilePath);
    }

    //xcpkg_formula_repo_dump(formulaRepo);

    if (ret == XCPKG_OK) {
        (*out) = formulaRepo;
        return XCPKG_OK;
    } else {
        xcpkg_formula_repo_free(formulaRepo);
        return ret;
    }
}
