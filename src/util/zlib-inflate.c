#include <stdio.h>

#include "../core/zlib-flate.h"

#include "../util.h"

/**
 *  xcpkg util zlib-inflate < input/file/path
 */
int xcpkg_util_zlib_inflate(int argc, char* argv[]) {
    return zlib_inflate_file_to_file(stdin, stdout);
}
