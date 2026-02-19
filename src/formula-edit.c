#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <limits.h>

#include "xcpkg.h"

int xcpkg_formula_edit(const char * packageName, const char * targetPlatformName, const char * editor) {
    char formulaFilePath[PATH_MAX];

    int ret = xcpkg_formula_path(packageName, targetPlatformName, formulaFilePath);

    if (ret != XCPKG_OK) {
        return ret;
    }

    if (editor == NULL || editor[0] == '\0') {
        editor = "vim";
    }

    if (editor[0] == '/') {
        execl (editor, editor, formulaFilePath, NULL);
    } else {
        execlp(editor, editor, formulaFilePath, NULL);
    }

    perror(editor);

    return XCPKG_ERROR;
}
