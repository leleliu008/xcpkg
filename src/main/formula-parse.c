#include <stdio.h>
#include <string.h>

#include "../xcpkg.h"

/**
 *  xcpkg formula-parse <FORMULA-FILE-PATH> [--json | --yaml | <KEY>]
 */
int xcpkg_main_formula_parse(int argc, char* argv[]) {
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

    return xcpkg_print_available_info(packageName, NULL, argv[3], argv[2]);
}
