#include <stdio.h>
#include <string.h>

#include "../xcpkg.h"
#include "../core/log.h"

/**
 *  xcpkg completion <zsh|bash|fish>
 */
int xcpkg_main_completion(int argc, char* argv[]) {
    if (argv[2] == NULL) {
        fprintf(stderr, "Usage: %s completion <zsh|bash|fish>\n", argv[0]);
        return XCPKG_ERROR_ARG_IS_UNSPECIFIED;
    } else if (strcmp(argv[2], "zsh") == 0) {
        return xcpkg_completion_zsh();
    } else if (strcmp(argv[2], "bash") == 0) {
        return xcpkg_completion_bash();
    } else if (strcmp(argv[2], "fish") == 0) {
        return xcpkg_completion_fish();
    } else {
        LOG_ERROR2("unknown argument: ", argv[2]);
        return XCPKG_ERROR_ARG_IS_UNKNOWN;
    }
}
