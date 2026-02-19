#include <stdio.h>

#include "xcpkg.h"

int xcpkg_completion_zsh() {
    puts(XCPKG_ZSH_COMPLETION_SCRIPT_STRING);
    return XCPKG_OK;
}

int xcpkg_completion_bash() {
    return XCPKG_OK;
}

int xcpkg_completion_fish() {
    return XCPKG_OK;
}
