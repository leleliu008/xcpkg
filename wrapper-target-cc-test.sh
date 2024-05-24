#!/bin/sh

set -ex

NATIVE_OS_VERS="$(sw_vers -productVersion)"
NATIVE_OS_ARCH="$(uname -m)"

SYSROOT="$(xcrun --sdk macosx --show-sdk-path)"

export XCPKG_COMPILER_C="$(xcrun --sdk macosx --find clang)"
export XCPKG_COMPILER_ARGS="-isysroot $SYSROOT     -mmacosx-version-min=$NATIVE_OS_VERS -arch $NATIVE_OS_ARCH -Qunused-arguments    -ldl"
export XCPKG_VERBOSE=1

clang -flto -Os -o wrapper-target-cc wrapper-target-cc.c

./wrapper-target-cc "$@"
