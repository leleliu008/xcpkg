#ifndef XCPKG_UTIL_H
#define XCPKG_UTIL_H

#define DECLARE_UTIL(name) int xcpkg_util_##name(int argc, char* argv[]);

DECLARE_UTIL(base16_encode)
DECLARE_UTIL(base16_decode)

DECLARE_UTIL(base64_encode)
DECLARE_UTIL(base64_decode)

DECLARE_UTIL(zlib_deflate)
DECLARE_UTIL(zlib_inflate)

DECLARE_UTIL(sha256sum)

DECLARE_UTIL(http_fetch)
DECLARE_UTIL(git_sync)

DECLARE_UTIL(uncompress)

DECLARE_UTIL(which)
DECLARE_UTIL(printenv)

DECLARE_UTIL(mkdir_p)
DECLARE_UTIL(rm_rf)

DECLARE_UTIL(list_PATH)

#endif
