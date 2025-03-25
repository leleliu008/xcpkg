#!/bin/sh

set -ex

NATIVE_PLATFORM_VERS="$(sw_vers -productVersion)"
NATIVE_PLATFORM_ARCH="$(uname -m)"

SYSROOT="$(xcrun --sdk macosx --show-sdk-path)"

export XCPKG_COMPILER_C="$(xcrun --sdk macosx --find clang)"
export XCPKG_COMPILER_ARGS="-isysroot $SYSROOT     -mmacosx-version-min=$NATIVE_PLATFORM_VERS -arch $NATIVE_PLATFORM_ARCH -Qunused-arguments    -ldl"
export XCPKG_VERBOSE=1

clang -flto -Os -o wrapper-target-cc wrapper-target-cc.c

./wrapper-target-cc "$@"
