#ifndef XCPKG_UTIL_H
#define XCPKG_UTIL_H

int xcpkg_util_base16_encode(int argc, char* argv[]);

int xcpkg_util_base16_decode(int argc, char* argv[]);

int xcpkg_util_base64_encode(int argc, char* argv[]);

int xcpkg_util_base64_decode(int argc, char* argv[]);

int xcpkg_util_zlib_deflate(int argc, char* argv[]);

int xcpkg_util_zlib_inflate(int argc, char* argv[]);

int xcpkg_util_sha256sum(int argc, char* argv[]);

int xcpkg_util_http_fetch(int argc, char* argv[]);

int xcpkg_util_git_sync(int argc, char* argv[]);

int xcpkg_util_uncompress(int argc, char* argv[]);

int xcpkg_util_mkdir_p(int argc, char* argv[]);

int xcpkg_util_rm_rf(int argc, char* argv[]);

int xcpkg_util_which(int argc, char* argv[]);

int xcpkg_util_printenv(int argc, char* argv[]);

int xcpkg_util_list_PATH(int argc, char* argv[]);

#endif
