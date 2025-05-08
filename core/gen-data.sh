#!/bin/sh

set -ex

[ -z "$1" ] && {
    printf 'Usage: %s <OUTPUT-FILEPATH>, <OUTPUT-FILEPATH> is unspecified.\n' "$0"
    exit 1
}

CWD="$(dirname "$0")"

cd "$CWD"

cc -std=c99 -Os -flto -Wl,-S -o file2c file2c.c

cat > "$1" <<EOF
#include<stdlib.h>

EOF

{
./file2c wrapper-native-cc.c   XCPKG_WRAPPER_NATIVE_CC_C_SOURCE_STRING
./file2c wrapper-native-c++.c  XCPKG_WRAPPER_NATIVE_CXX_C_SOURCE_STRING
./file2c wrapper-native-objc.c XCPKG_WRAPPER_NATIVE_OBJC_C_SOURCE_STRING

./file2c wrapper-target-cc.c   XCPKG_WRAPPER_TARGET_CC_C_SOURCE_STRING
./file2c wrapper-target-c++.c  XCPKG_WRAPPER_TARGET_CXX_C_SOURCE_STRING
./file2c wrapper-target-objc.c XCPKG_WRAPPER_TARGET_OBJC_C_SOURCE_STRING

./file2c xcpkg-install         XCPKG_INSTALL_SHELL_SCRIPT_STRING
./file2c ../xcpkg-help.txt     XCPKG_HELP_STRING
./file2c ../xcpkg-zsh-completion XCPKG_ZSH_COMPLETION_SCRIPT_STRING
} >> "$1"
