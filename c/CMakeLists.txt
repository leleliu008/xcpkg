cmake_minimum_required(VERSION 3.14)

project(xcpkg VERSION 0.30.0 LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

########################################################################################

message(STATUS "CMAKE_HOST_SYSTEM_NAME = ${CMAKE_HOST_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_NAME      = ${CMAKE_SYSTEM_NAME}")

message(STATUS "CMAKE_VERSION        = ${CMAKE_VERSION}")
message(STATUS "CMAKE_PREFIX_PATH    = ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_INCLUDE_PATH   = ${CMAKE_INCLUDE_PATH}")
message(STATUS "CMAKE_LIBRARY_PATH   = ${CMAKE_LIBRARY_PATH}")
message(STATUS "CMAKE_FIND_ROOT_PATH = ${CMAKE_FIND_ROOT_PATH}")
message(STATUS "ENV: PKG_CONFIG_PATH = $ENV{PKG_CONFIG_PATH}")

########################################################################################

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" "${CMAKE_MODULE_PATH}")

# https://cmake.org/cmake/help/latest/module/FindPkgConfig.html
find_package(PkgConfig REQUIRED)

# https://cmake.org/cmake/help/latest/module/FindOpenSSL.html
find_package(OpenSSL REQUIRED)

# https://cmake.org/cmake/help/latest/module/FindCURL.html
find_package(CURL REQUIRED)

# https://cmake.org/cmake/help/latest/module/FindLibArchive.html
find_package(LibArchive REQUIRED)

find_package(LibGit2 REQUIRED)

find_package(LibYAML REQUIRED)

find_package(JANSSON REQUIRED)

find_package(ZLIB REQUIRED)

########################################################################################

message(STATUS "CURL_LIBRARIES=${CURL_LIBRARIES}")
message(STATUS "OPENSSL_LIBRARIES=${OPENSSL_LIBRARIES}")
message(STATUS "LibArchive_LIBRARIES=${LibArchive_LIBRARIES}")
message(STATUS "LIBGIT2_LIBRARIES=${LIBGIT2_LIBRARIES}")
message(STATUS "LIBYAML_LIBRARY=${LIBYAML_LIBRARY}")
message(STATUS "JANSSON_LIBRARY=${JANSSON_LIBRARY}")
message(STATUS "ZLIB_LIBRARIES=${ZLIB_LIBRARIES}")

########################################################################################

if (OPENSSL_VERSION)
    message(STATUS "OPENSSL_VERSION = ${OPENSSL_VERSION}")

    string(REPLACE "." ";" OPENSSL_VERSION_PART_LIST ${OPENSSL_VERSION})
    list(GET OPENSSL_VERSION_PART_LIST 0 OPENSSL_VERSION_MAJOR)

    message(STATUS "OPENSSL_VERSION_MAJOR = ${OPENSSL_VERSION_MAJOR}")

    if (OPENSSL_VERSION_MAJOR STREQUAL 1)
        message(FATAL_ERROR "This project need openssl3, but openssl-${OPENSSL_VERSION} was found.")
    endif()
endif()

########################################################################################

aux_source_directory(src/core CORE_SRCS)
aux_source_directory(src           SRCS)

add_executable(xcpkg "${CORE_SRCS};src/core/regex/posix.c;${SRCS}")

########################################################################################

target_link_libraries(xcpkg CURL::libcurl)
target_link_libraries(xcpkg OpenSSL::SSL)
target_link_libraries(xcpkg OpenSSL::Crypto)
target_link_libraries(xcpkg LibArchive::LibArchive)
target_link_libraries(xcpkg LIBGIT2::LIBGIT2)
target_link_libraries(xcpkg LIBYAML::LIBYAML)
target_link_libraries(xcpkg JANSSON::JANSSON)
target_link_libraries(xcpkg ZLIB::ZLIB)
target_link_libraries(xcpkg -lm)

########################################################################################

string(TIMESTAMP COMPILED_TIMESTAMP "%Y-%m-%d %H:%M:%S UTC" UTC)

file(READ ../xcpkg-help.txt XCPKG_HELP HEX)
file(READ ../xcpkg-zsh-completion XCPKG_ZSH_COMPLETION HEX)
file(READ ../xcpkg-url-transform.sample XCPKG_URL_TRANSFORM_SAMPLE HEX)

configure_file(src/config.h.in "${CMAKE_CURRENT_SOURCE_DIR}/src/config.h" @ONLY)

########################################################################################

include(GNUInstallDirs)

install(TARGETS xcpkg RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
