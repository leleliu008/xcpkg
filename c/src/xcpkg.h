#ifndef XCPKG_H
#define XCPKG_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "core/tar.h"

#include "config.h"


#define XCPKG_PACKAGE_NAME_PATTERN "^[A-Za-z0-9+-._@]{1,50}$"

#define XCPKG_METADATA_DIR_PATH_RELATIVE_TO_INSTALLED_ROOT "/.xcpkg"
#define XCPKG_MANIFEST_FILEPATH_RELATIVE_TO_INSTALLED_ROOT "/.xcpkg/MANIFEST.txt"
#define XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_INSTALLED_ROOT "/.xcpkg/RECEIPT.yml"
#define XCPKG_RECEIPT_FILEPATH_RELATIVE_TO_METADATA_DIR "/RECEIPT.yml"

#define XCPKG_FORMULA_REPO_CONFIG_FILPATH_RELATIVE_TO_REPO_ROOT "/.xcpkg-formula-repo.yml"


 
#define XCPKG_OK                     0
#define XCPKG_ERROR                  1

#define XCPKG_ERROR_ARG_IS_NULL      2
#define XCPKG_ERROR_ARG_IS_EMPTY     3
#define XCPKG_ERROR_ARG_IS_INVALID   4
#define XCPKG_ERROR_ARG_IS_UNKNOWN   5

#define XCPKG_ERROR_MEMORY_ALLOCATE  6

#define XCPKG_ERROR_SHA256_MISMATCH  7

#define XCPKG_ERROR_ENV_HOME_NOT_SET 8
#define XCPKG_ERROR_ENV_PATH_NOT_SET 9

#define XCPKG_ERROR_NOT_FOUND    10
#define XCPKG_ERROR_NOT_MATCH    11

#define XCPKG_ERROR_INVALID_URL  12

#define XCPKG_ERROR_PLATFORM_SPEC_IS_INVALID 14

#define XCPKG_ERROR_PACKAGE_SPEC_IS_INVALID 15

#define XCPKG_ERROR_PACKAGE_NAME_IS_NULL    15
#define XCPKG_ERROR_PACKAGE_NAME_IS_EMPTY   16
#define XCPKG_ERROR_PACKAGE_NAME_IS_TOOLONG 17
#define XCPKG_ERROR_PACKAGE_NAME_IS_INVALID 18

#define XCPKG_ERROR_PACKAGE_NOT_AVAILABLE 20
#define XCPKG_ERROR_PACKAGE_NOT_INSTALLED 21
#define XCPKG_ERROR_PACKAGE_NOT_OUTDATED  22
#define XCPKG_ERROR_PACKAGE_IS_BROKEN     23

#define XCPKG_ERROR_FORMULA_REPO_NOT_FOUND 30
#define XCPKG_ERROR_FORMULA_REPO_HAS_EXIST 31
#define XCPKG_ERROR_FORMULA_REPO_IS_BROKEN 32
#define XCPKG_ERROR_FORMULA_REPO_CONFIG_SYNTAX 34
#define XCPKG_ERROR_FORMULA_REPO_CONFIG_SCHEME 35

#define XCPKG_ERROR_FORMULA_SYNTAX     40
#define XCPKG_ERROR_FORMULA_SCHEME     41

#define XCPKG_ERROR_RECEIPT_SYNTAX     45
#define XCPKG_ERROR_RECEIPT_SCHEME     46

#define XCPKG_ERROR_URL_TRANSFORM_ENV_NOT_SET           50
#define XCPKG_ERROR_URL_TRANSFORM_ENV_VALUE_IS_EMPTY    51
#define XCPKG_ERROR_URL_TRANSFORM_ENV_POINT_TO_PATH_NOT_EXIST 52
#define XCPKG_ERROR_URL_TRANSFORM_RUN_NO_RESULT         53

// libgit's error [-35, -1]
#define XCPKG_ERROR_LIBGIT2_BASE    70

// libarchive's error [-30, 1]
#define XCPKG_ERROR_ARCHIVE_BASE    110

// libcurl's error [1, 99]
#define XCPKG_ERROR_NETWORK_BASE    150

static const char * supportedTargetPlatformNames[] = {
    "MacOSX",
    "DriverKit",
    "WatchOS",
    "WatchSimulator",
    "iPhoneOS",
    "iPhoneSimulator",
    "AppleTVOS",
    "AppleTVSimulator",
    "XROS",
    "XRSimulator",
    NULL
};

typedef enum {
    XCPKGPlatformID_MacOSX,
    XCPKGPlatformID_DriverKit,
    XCPKGPlatformID_WatchOS,
    XCPKGPlatformID_WatchSimulator,
    XCPKGPlatformID_iPhoneOS,
    XCPKGPlatformID_iPhoneSimulator,
    XCPKGPlatformID_AppleTVOS,
    XCPKGPlatformID_AppleTVSimulator,
    XCPKGPlatformID_XROS,
    XCPKGPlatformID_XRSimulator
} XCPKGPlatformID;

typedef enum {
    XCPKGPkgType_exe,
    XCPKGPkgType_lib
} XCPKGPkgType;

typedef struct {
    char * path;

    char * version;

    char * summary;

    char * license;

    char * web_url;
    char * web_uri;

    char * git_url;
    char * git_uri;
    char * git_sha;
    char * git_ref;
    size_t git_nth;

    char * src_url;
    char * src_uri;
    char * src_sha;
    bool   src_is_dir;

    char * fix_url;
    char * fix_uri;
    char * fix_sha;
    char * fix_opt;
    char * patches;

    char * res_url;
    char * res_uri;
    char * res_sha;
    char * reslist;

    char * dep_pkg;
    char * dep_upp;
    char * dep_pym;
    char * dep_plm;
    char * dep_lib;

    char * bsystem;
    char * bscript;

    bool   binbstd;

    bool   symlink;

    bool   ltoable;
    bool   movable;

    bool   support_build_in_parallel;

    bool   support_create_mostly_statically_linked_executable;

    char * ppflags;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * dofetch;
    char * do12345;
    char * dopatch;
    char * prepare;
    char * install;
    char * dotweak;
    char * bindenv;
    char * caveats;

    bool web_url_is_calculated;
    bool version_is_calculated;
    bool bsystem_is_calculated;

    bool useBuildSystemAutogen;
    bool useBuildSystemAutotools;
    bool useBuildSystemConfigure;
    bool useBuildSystemCmake;
    bool useBuildSystemXmake;
    bool useBuildSystemGmake;
    bool useBuildSystemMeson;
    bool useBuildSystemNinja;
    bool useBuildSystemCargo;
    bool useBuildSystemGolang;
    bool useBuildSystemGN;

    XCPKGPkgType pkgtype;
} XCPKGFormula;

int  xcpkg_formula_path(const char * packageName, const char * targetPlatformName, char formulaFilePath[]);
int  xcpkg_formula_load(const char * packageName, const char * targetPlatformName, const char * formulaFilePath, XCPKGFormula * * out);
int  xcpkg_formula_edit(const char * packageName, const char * targetPlatformName, const char * editor);
int  xcpkg_formula_view(const char * packageName, const char * targetPlatformName, const bool raw);
int  xcpkg_formula_cat (const char * packageName, const char * targetPlatformName);
int  xcpkg_formula_bat (const char * packageName, const char * targetPlatformName);

void xcpkg_formula_free(XCPKGFormula * formula);
void xcpkg_formula_dump(XCPKGFormula * formula);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * name;
    char * url;
    char * branch;
    char * path;
    char * createdAt;
    char * updatedAt;
    bool   pinned;
    bool   enabled;
} XCPKGFormulaRepo ;

typedef struct {
    XCPKGFormulaRepo * * repos;
    size_t size;
} XCPKGFormulaRepoList ;

int  xcpkg_formula_repo_create(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  xcpkg_formula_repo_add   (const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  xcpkg_formula_repo_remove(const char * formulaRepoName);
int  xcpkg_formula_repo_sync_ (const char * formulaRepoName);
int  xcpkg_formula_repo_info_ (const char * formulaRepoName);
int  xcpkg_formula_repo_config(const char * formulaRepoName, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled);
int  xcpkg_formula_repo_config_write(const char * formulaRepoDIRPath, const char * formulaRepoUrl, const char * branchName, int pinned, int enabled, const char * createdAt, const char * updatedAt);
int  xcpkg_formula_repo_lookup(const char * formulaRepoName, XCPKGFormulaRepo * * formulaRepo);
int  xcpkg_formula_repo_parse (const char * formulaRepoConfigFilePath, XCPKGFormulaRepo * * formulaRepo);

void xcpkg_formula_repo_free(XCPKGFormulaRepo * formulaRepo);
void xcpkg_formula_repo_dump(XCPKGFormulaRepo * formulaRepo);
int  xcpkg_formula_repo_info(XCPKGFormulaRepo * formulaRepo);
int  xcpkg_formula_repo_sync(XCPKGFormulaRepo * formulaRepo);

int  xcpkg_formula_repo_list     (XCPKGFormulaRepoList * * p);
void xcpkg_formula_repo_list_free(XCPKGFormulaRepoList   * p);

int  xcpkg_formula_repo_list_printf();
int  xcpkg_formula_repo_list_update();


//////////////////////////////////////////////////////////////////////

int xcpkg_get_native_platform_spec(char buf[], size_t bufSize, size_t * len);

int xcpkg_inspect_target_platform_spec(const char * targetPlatformSpec, int * index1, int * index2);

int xcpkg_inspect_package(const char * package, const char * userSpecifiedTargetPlatformSpec, const char ** packageName, const char ** targetPlatformSpecPointer, char targetPlatformSpec[]);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * summary;
    char * version;
    char * license;

    char * web_url;

    char * git_url;
    char * git_sha;
    char * git_ref;
    size_t git_nth;

    char * src_url;
    char * src_uri;
    char * src_sha;

    char * fix_url;
    char * fix_uri;
    char * fix_sha;

    char * res_url;
    char * res_uri;
    char * res_sha;

    char * dep_pkg;
    char * dep_upp;
    char * dep_pym;
    char * dep_plm;

    char * bsystem;
    char * bscript;

    bool   binbstd;
    bool   parallel;

    bool   symlink;

    char * ppflags;
    char * ccflags;
    char * xxflags;
    char * ldflags;

    char * do12345;
    char * dopatch;
    char * install;

    char * path;

    char * builtBy;
    char * builtAt;
    char * builtFor;
} XCPKGReceipt;

int  xcpkg_receipt_parse(const char * packageName, const char * targetPlatformSpec, XCPKGReceipt * * receipt);
void xcpkg_receipt_free(XCPKGReceipt * receipt);
void xcpkg_receipt_dump(XCPKGReceipt * receipt);

//////////////////////////////////////////////////////////////////////

typedef struct {
    char * cc;
    char * cxx;
    char * swift;
    char * as;
    char * ar;
    char * ranlib;
    char * ld;
    char * nm;
    char * strip;
    char * size;
    char * strings;
    char * objdump;
} XCPKGToolChain;

int  xcpkg_toolchain_find(XCPKGToolChain * toolchain);
void xcpkg_toolchain_free(XCPKGToolChain * toolchain);
void xcpkg_toolchain_dump(XCPKGToolChain * toolchain);

int  xcpkg_sdk_path(const char * sdk, char buf[]);

//////////////////////////////////////////////////////////////////////

int xcpkg_main(int argc, char* argv[]);

int xcpkg_util(int argc, char* argv[]);

int xcpkg_help();

int xcpkg_sysinfo();

int xcpkg_buildinfo();

int xcpkg_about(const bool verbose);

/** get the xcpkg home directory absolute path
 *
 *  the capacity of buf must be PATH_MAX
 *
 *  len can be null if you do not want to known the length of filled string
 *
 *  on success, 0 is returned and buf will be filled with a null-terminated string
 *
 *  on error, none-zero value will be returned and buf remains unchanged.
 */
int xcpkg_home_dir(char buf[], size_t * len);

/** get the session directory absolute path
 *
 *  the capacity of buf must be PATH_MAX
 *
 *  len can be null if you do not want to known the length of filled string
 *
 *  on success, 0 is returned and buf will be filled with a null-terminated string
 *
 *  on error, none-zero value will be returned and buf remains unchanged.
 */
int xcpkg_session_dir(char buf[], size_t * len);

int xcpkg_search(const char * regPattern, const char * targetPlatformName, const bool verbose);

int xcpkg_available_info(const char * packageName, const char * targetPlatformName, const char * key);

int xcpkg_installed_info(const char * packageName, const char * targetPlatformSpec, const char * key);

int xcpkg_tree(const char * packageName, const char * targetPlatformSpec, size_t argc, char* argv[]);

int xcpkg_logs(const char * packageName, const char * targetPlatformSpec);

int xcpkg_bundle(const char * packageName, const char * targetPlatformSpec, ArchiveType outputType, const char * outputPath, const bool verbose);

typedef enum {
    XCPKGDependsOutputType_D2,
    XCPKGDependsOutputType_DOT,
    XCPKGDependsOutputType_BOX,
    XCPKGDependsOutputType_SVG,
    XCPKGDependsOutputType_PNG,
} XCPKGDependsOutputType;

typedef enum {
    XCPKGDependsOutputDiagramEngine_DOT,
    XCPKGDependsOutputDiagramEngine_D2
} XCPKGDependsOutputDiagramEngine;

int xcpkg_depends(const char * packageName, const char * targetPlatformName, XCPKGDependsOutputType outputType, const char * outputPath, XCPKGDependsOutputDiagramEngine engine);

int xcpkg_fetch(const char * packageName, const char * targetPlatformName, const bool verbose);

//////////////////////////////////////////////////////////////////////

typedef enum {
    XCPKGLogLevel_silent,
    XCPKGLogLevel_normal,
    XCPKGLogLevel_verbose,
    XCPKGLogLevel_very_verbose
} XCPKGLogLevel;

typedef enum {
    XCPKGBuildProfile_release,
    XCPKGBuildProfile_debug
} XCPKGBuildProfile;

typedef struct {
    bool exportCompileCommandsJson;
    bool keepSessionDIR;
    bool enableCcache;
    bool enableBear;
    bool dryrun;
    bool force;
    bool xtrace;
    bool createMostlyStaticallyLinkedExecutables;

    bool verbose_net;
    bool verbose_env;
    bool verbose_cc;
    bool verbose_ld;

    size_t parallelJobsCount;

    XCPKGLogLevel logLevel;
    XCPKGBuildProfile buildType;
} XCPKGInstallOptions;

int xcpkg_install  (const char * packageName, const char * targetPlatformSpec, const XCPKGInstallOptions * options);

int xcpkg_upgrade  (const char * packageName, const char * targetPlatformSpec, const XCPKGInstallOptions * options);

int xcpkg_reinstall(const char * packageName, const char * targetPlatformSpec, const XCPKGInstallOptions * options);

int xcpkg_uninstall(const char * packageName, const char * targetPlatformSpec, const bool verbose);

int xcpkg_upgrade_self(const bool verbose);

int xcpkg_integrate_zsh_completion (const char * outputDIR, const bool verbose);
int xcpkg_integrate_bash_completion(const char * outputDIR, const bool verbose);
int xcpkg_integrate_fish_completion(const char * outputDIR, const bool verbose);

int xcpkg_setup(const bool verbose);

int xcpkg_cleanup(const bool verbose);

int xcpkg_check_if_the_given_argument_matches_package_name_pattern(const char * p);
int xcpkg_check_if_the_given_argument_matches_platform_spec_pattern(const char * p);

int xcpkg_check_if_the_given_package_is_available(const char * packageName, const char * targetPlatformName);
int xcpkg_check_if_the_given_package_is_installed(const char * packageName, const char * targetPlatformSpec);
int xcpkg_check_if_the_given_package_is_outdated (const char * packageName, const char * targetPlatformSpec);

typedef int (*XCPKGPackageNameFilter)(const char * packageName, const char * targetPlatformName, const bool verbose, const size_t index, const void * payload);

int xcpkg_show_the_available_packages(const char * targetPlatformName, const bool verbose);

int xcpkg_list_the_available_packages(const char * targetPlatformName, const bool verbose, XCPKGPackageNameFilter packageNameFilter, const void * payload);
int xcpkg_list_the_installed_packages(const char * targetPlatformName, const bool verbose);
int xcpkg_list_the__outdated_packages(const char * targetPlatformName, const bool verbose);

int xcpkg_git_sync(const char * gitRepositoryDIRPath, const char * remoteUrl, const char * remoteRef, const char * remoteTrackingRef, const char * checkoutToBranchName, const size_t fetchDepth);

int xcpkg_generate_url_transform_sample();

int xcpkg_extract_filetype_from_url(const char * url, char buf[], const size_t bufSize);

int xcpkg_extract_filename_from_url(const char * url, char buf[], const size_t bufSize);

int xcpkg_extract_version_from_src_url(const char * url, char versionBuf[], size_t versionBufCapacity);

int xcpkg_extract_version_from_git_ref(const char * ref, char versionBuf[], size_t versionBufCapacity);

int xcpkg_http_fetch_to_file(const char * url, const char * outputFilePath, const bool verbose, const bool showProgress);

int xcpkg_http_fetch_to_stream(const char * url, FILE * stream, const bool verbose, const bool showProgress);

int xcpkg_download(const char * url, const char * uri, const char * expectedSHA256SUM, const char * outputPath, const bool verbose);

int xcpkg_uncompress(const char * filePath, const char * unpackDIR, const size_t stripComponentsNumber, const bool verbose);

int xcpkg_rename_or_copy_file(const char * fromFilePath, const char * toFilePath);

int xcpkg_copy_file(const char * fromFilePath, const char * toFilePath);

int xcpkg_write_file(const char * fp, const char * str, size_t strLen);

int xcpkg_read_the_first_n_bytes_of_a_file(const char * fp, unsigned int n, char buf[]);

int xcpkg_mkdir_p(const char * dirPath, const bool verbose);

int xcpkg_rm_rf(const char * dirPath, const bool preserveRoot, const bool verbose);

int xcpkg_fork_exec(char * cmd);

int xcpkg_fork_exec2(const size_t n, ...);

int xcpkg_setenv_SSL_CERT_FILE();

int xcpkg_get_platform_id_by_name(const char * const platformName, XCPKGPlatformID * const platformID);

int xcpkg_get_command_path_of_uppm_package(const char * uppmPackageName, const char * cmdname, char buf[]);

#endif
