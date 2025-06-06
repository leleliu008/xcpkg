#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <limits.h>
#include <sys/stat.h>

#include <yaml.h>

#include "uppm.h"
#include "xcpkg.h"

typedef enum {
    UPPMFormulaKeyCode_unknown,
    UPPMFormulaKeyCode_summary,
    UPPMFormulaKeyCode_version,
    UPPMFormulaKeyCode_license,
    UPPMFormulaKeyCode_webpage,
    UPPMFormulaKeyCode_bin_url,
    UPPMFormulaKeyCode_bin_sha,
    UPPMFormulaKeyCode_dep_pkg,
    UPPMFormulaKeyCode_unpackd,
    UPPMFormulaKeyCode_install,
} UPPMFormulaKeyCode;

void uppm_formula_dump(UPPMFormula * formula) {
    if (formula == NULL) {
        return;
    }

    printf("summary: %s\n", formula->summary);
    printf("version: %s\n", formula->version);
    printf("license: %s\n", formula->license);
    printf("webpage: %s\n", formula->webpage);
    printf("bin-url: %s\n", formula->bin_url);
    printf("bin-sha: %s\n", formula->bin_sha);
    printf("dep-pkg: %s\n", formula->dep_pkg);
    printf("unpackd: %s\n", formula->unpackd);
    printf("install: %s\n", formula->install);
    printf("path:    %s\n", formula->path);
}

void uppm_formula_free(UPPMFormula * formula) {
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

    if (formula->webpage != NULL) {
        free(formula->webpage);
        formula->webpage = NULL;
    }

    if (formula->bin_url != NULL) {
        free(formula->bin_url);
        formula->bin_url = NULL;
    }

    if (formula->bin_sha != NULL) {
        free(formula->bin_sha);
        formula->bin_sha = NULL;
    }

    if (formula->dep_pkg != NULL) {
        free(formula->dep_pkg);
        formula->dep_pkg = NULL;
    }

    if (formula->unpackd != NULL) {
        free(formula->unpackd);
        formula->unpackd = NULL;
    }

    if (formula->install != NULL) {
        free(formula->install);
        formula->install = NULL;
    }

    if (formula->path != NULL) {
        free(formula->path);
        formula->path = NULL;
    }

    free(formula);
}

static UPPMFormulaKeyCode uppm_formula_key_code_from_key_name(char * key) {
           if (strcmp(key, "summary") == 0) {
        return UPPMFormulaKeyCode_summary;
    } else if (strcmp(key, "webpage") == 0) {
        return UPPMFormulaKeyCode_webpage;
    } else if (strcmp(key, "version") == 0) {
        return UPPMFormulaKeyCode_version;
    } else if (strcmp(key, "license") == 0) {
        return UPPMFormulaKeyCode_license;
    } else if (strcmp(key, "bin-url") == 0) {
        return UPPMFormulaKeyCode_bin_url;
    } else if (strcmp(key, "bin-sha") == 0) {
        return UPPMFormulaKeyCode_bin_sha;
    } else if (strcmp(key, "dep-pkg") == 0) {
        return UPPMFormulaKeyCode_dep_pkg;
    } else if (strcmp(key, "unpackd") == 0) {
        return UPPMFormulaKeyCode_unpackd;
    } else if (strcmp(key, "install") == 0) {
        return UPPMFormulaKeyCode_install;
    } else {
        return UPPMFormulaKeyCode_unknown;
    }
}

static int uppm_formula_set_value(UPPMFormulaKeyCode keyCode, char * value, UPPMFormula * formula) {
    if (keyCode == UPPMFormulaKeyCode_unknown) {
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
        case UPPMFormulaKeyCode_summary:
            if (formula->summary != NULL) {
                free(formula->summary);
            }

            formula->summary = strdup(value);

            if (formula->summary == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_version:
            if (formula->version != NULL) {
                free(formula->version);
            }

            formula->version = strdup(value);

            if (formula->version == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_license:
            if (formula->license != NULL) {
                free(formula->license);
            }

            formula->license = strdup(value);

            if (formula->license == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_webpage:
            if (formula->webpage != NULL) {
                free(formula->webpage);
            }

            formula->webpage = strdup(value);

            if (formula->webpage == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_bin_url:
            if (formula->bin_url != NULL) {
                free(formula->bin_url);
            }

            formula->bin_url = strdup(value);

            if (formula->bin_url == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_bin_sha:
            if (formula->bin_sha != NULL) {
                free(formula->bin_sha);
            }

            formula->bin_sha = strdup(value);

            if (formula->bin_sha == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_dep_pkg:
            if (formula->dep_pkg != NULL) {
                free(formula->dep_pkg);
            }

            formula->dep_pkg = strdup(value);

            if (formula->dep_pkg == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_unpackd:
            if (formula->unpackd != NULL) {
                free(formula->unpackd);
            }

            formula->unpackd = strdup(value);

            if (formula->unpackd == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        case UPPMFormulaKeyCode_install:
            if (formula->install != NULL) {
                free(formula->install);
            }

            formula->install = strdup(value);

            if (formula->install == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            } else {
                return XCPKG_OK;
            }
        default: return XCPKG_OK;
    }
}

static int uppm_formula_check(UPPMFormula * formula, const char * formulaFilePath) {
    if (formula->summary == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : summary mapping not found.\n", formulaFilePath);
        return UPPM_ERROR_FORMULA_SCHEME;
    }

    if (formula->webpage == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : webpage mapping not found.\n", formulaFilePath);
        return UPPM_ERROR_FORMULA_SCHEME;
    }

    if (formula->bin_url == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : bin-url mapping not found.\n", formulaFilePath);
        return UPPM_ERROR_FORMULA_SCHEME;
    }

    if (formula->bin_sha == NULL) {
        fprintf(stderr, "scheme error in formula file: %s : bin-sha mapping not found.\n", formulaFilePath);
        return UPPM_ERROR_FORMULA_SCHEME;
    }

    if (strlen(formula->bin_sha) != 64) {
        fprintf(stderr, "scheme error in formula file: %s : bin-sha mapping's value's length must be 64.\n", formulaFilePath);
        return UPPM_ERROR_FORMULA_SCHEME;
    }

    if (formula->version == NULL) {
        char version[20]; version[0] = '\0';

        int ret = xcpkg_extract_version_from_src_url(formula->bin_url, version, 20);

        if (ret != XCPKG_OK) {
            return ret;
        }

        if (version[0] == '\0') {
            fprintf(stderr, "scheme error in formula file: %s : version mapping not found.\n", formulaFilePath);
            return UPPM_ERROR_FORMULA_SCHEME;
        } else {
            formula->version = strdup(version);

            if (formula->version == NULL) {
                return XCPKG_ERROR_MEMORY_ALLOCATE;
            }
        }
    }

    return XCPKG_OK;
}

int uppm_formula_lookup(const char * uppmHomeDIR, const size_t uppmHomeDIRLength, const char * packageName, UPPMFormula * * out) {
    size_t formulaFilePathCapacity = uppmHomeDIRLength + strlen(packageName) + 41U;
    char   formulaFilePath[formulaFilePathCapacity];

    int ret = snprintf(formulaFilePath, formulaFilePathCapacity, "%s/repos.d/official-core/formula/%s.yml", uppmHomeDIR, packageName);

    if (ret < 0) {
        perror(NULL);
        return XCPKG_ERROR;
    }

    struct stat st;

    if (stat(formulaFilePath, &st) == 0) {

    } else {
        return UPPM_ERROR_PACKAGE_NOT_AVAILABLE;
    }

    FILE * formulaFile = fopen(formulaFilePath, "r");

    if (formulaFile == NULL) {
        perror(formulaFilePath);
        return XCPKG_ERROR;
    }

    yaml_parser_t parser;
    yaml_token_t  token;

    // https://libyaml.docsforge.com/master/api/yaml_parser_initialize/
    if (yaml_parser_initialize(&parser) == 0) {
        perror("Failed to initialize yaml parser");
        fclose(formulaFile);
        return XCPKG_ERROR;
    }

    yaml_parser_set_input_file(&parser, formulaFile);

    UPPMFormulaKeyCode formulaKeyCode = UPPMFormulaKeyCode_unknown;

    UPPMFormula * formula = NULL;

    int lastTokenType = 0;

    do {
        // https://libyaml.docsforge.com/master/api/yaml_parser_scan/
        if (yaml_parser_scan(&parser, &token) == 0) {
            fprintf(stderr, "syntax error in formula file: %s\n", formulaFilePath);
            ret = UPPM_ERROR_FORMULA_SYNTAX;
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
                    formulaKeyCode = uppm_formula_key_code_from_key_name((char*)token.data.scalar.value);
                } else if (lastTokenType == 2) {
                    if (formula == NULL) {
                        formula = (UPPMFormula*)calloc(1, sizeof(UPPMFormula));

                        if (formula == NULL) {
                            ret = XCPKG_ERROR_MEMORY_ALLOCATE;
                            goto finalize;
                        }

                        formula->path = formulaFilePath;
                    }

                    ret = uppm_formula_set_value(formulaKeyCode, (char*)token.data.scalar.value, formula);

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

    fclose(formulaFile);

    if (formula != NULL) {
        char * p = strdup(formulaFilePath);

        if (p == NULL) {
            uppm_formula_free(formula);
            return XCPKG_ERROR_MEMORY_ALLOCATE;
        } else {
            formula->path = p;
        }
    }

    if (ret == XCPKG_OK) {
        ret = uppm_formula_check(formula, formulaFilePath);
    }

    if (ret == XCPKG_OK) {
        (*out) = formula;
        return XCPKG_OK;
    } else {
        uppm_formula_free(formula);
        return ret;
    }
}
