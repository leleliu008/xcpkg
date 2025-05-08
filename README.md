# xcpkg

A package builder/manager for [Xcode](https://developer.apple.com/xcode) to build projects written in C, C++, Rust, Zig, Go, Haskell, etc.

## Caveats

- This software is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

- Please do NOT place your own files under `~/.xcpkg` directory, as `xcpkg` will change files under `~/.xcpkg` directory without notice.

- Please do NOT run `xcpkg` command in parallel so as not to generate dirty data.

## Using xcpkg via GitHub Actions

This is the recommended way of using this software.

In this way, you don't need a computer in hand, you could use GitHub mobile Apps.

In this way, you will be liberated from the rut of setting up the build environment.

In this way, you do NOT need to frequently update this software, you always use the latest version.

In this way, all you need to do is just clicking the buttons and waiting for finishing. After finishing, a url refers to a zip archive will be provided to download.

For more details please refer to <https://github.com/leleliu008/xcpkg-package-manually-build>

## Build from C source remotely via GitHub Actions

- https://github.com/leleliu008/xcpkg-package-manually-build
- https://github.com/leleliu008/ppkg-package-manually-build

## Build from C source locally dependencies

|dependency|required?|purpose|
|----|---------|-------|
|[Xcode](https://developer.apple.com/xcode/) or [LLVM+clang](https://llvm.org/)|required |for compiling C source code|
|[cmake](https://cmake.org/)|required |for generating `build.ninja`|
|[ninja](https://ninja-build.org/)|required |for doing jobs that read from `build.ninja`|
|[pkg-config>=0.18](https://www.freedesktop.org/wiki/Software/pkg-config/)|required|for finding libraries|
||||
|[jansson](https://github.com/akheron/jansson)|required|for parsing and creating JSON.|
|[libyaml](https://github.com/yaml/libyaml/)|required|for parsing formula files whose format is YAML.|
|[libgit2](https://libgit2.org/)|required|for updating formula repositories.|
|[libcurl](https://curl.se/)|required|for http requesting support.|
|[openssl](https://www.openssl.org/)|required|for https requesting support and SHA-256 sum checking support.|
|[libarchive](https://www.libarchive.org/)|required|for uncompressing .zip and .tar.* files.|
|[zlib](https://www.zlib.net/)|required|for compress and uncompress data.|

## Build from C source locally via [ppkg](https://github.com/leleliu008/ppkg)

```bash
ppkg install xcpkg
```

## Build from C source locally via `xcpkg` itself

```bash
xcpkg install xcpkg
```

## Build from C source locally using [vcpkg](https://github.com/microsoft/vcpkg)

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
export VCPKG_ROOT="$PWD/vcpkg"
export PATH="$VCPKG_ROOT:$PATH"

vcpkg install 'curl[http2,brotli,zstd,openssl]' libgit2 libarchive libyaml jansson

git clone --depth=1 https://github.com/leleliu008/xcpkg
cd xcpkg

cmake -S c -B   build.d -G Ninja -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build   build.d
cmake --install build.d
```

## Build from C source locally via [HomeBrew](https://brew.sh/)

```bash
cat > xcpkg.rb <<EOF
class Xcpkg < Formula
    desc     "A package builder/manager for Xcode to build projects written in C, C++, Rust, Zig, Go, Haskell, etc"
    homepage "https://github.com/leleliu008/xcpkg"
    head     "https://github.com/leleliu008/xcpkg.git", branch: "master"
    url      "https://github.com/leleliu008/xcpkg.git", revision: "d2757613f9f45b882df1a44d403d281f06d63462"
    version  "0.30.1"
    license  "Apache-2.0"

    depends_on "cmake" => :build
    depends_on "ninja" => :build
    depends_on "pkg-config" => :build

    depends_on "curl"
    depends_on "jansson"
    depends_on "libyaml"
    depends_on "libgit2"
    depends_on "libarchive"

    def install
      system "cmake", "-S", "c", "-B", "build", *std_cmake_args
      system "cmake", "--build",   "build"
      system "cmake", "--install", "build"
    end

    test do
      system "#{bin}/xcpkg", "--help"
    end
end
EOF

MY_FORMULA_DIR="$(brew --repository)/Library/Taps/leleliu008/homebrew-tmp/Formula"
install -d  "$MY_FORMULA_DIR/"
mv xcpkg.rb "$MY_FORMULA_DIR/"

brew install xcpkg
```

## ~/.xcpkg

**Caveats:** Please do NOT place your own files under `~/.xcpkg` directory, as xcpkg will change (remove, modify, override) files under `~/.xcpkg` directory without notice by default.

You are allowed to change this by setting `XCPKG_HOME` envionment variable.

## xcpkg command usage

- **show help of this command**

    ```bash
    xcpkg -h
    xcpkg --help
    ```

- **show version of this command**

    ```bash
    xcpkg -V
    xcpkg --version
    ```

- **show basic information about this software**

    ```bash
    xcpkg about
    ```

- **show basic information about your current running operation system**

    ```bash
    xcpkg sysinfo
    ```

- **show your current activated [Xcode](https://developer.apple.com/xcode) information**

    ```bash
    xcpkg xcinfo
    ```

- **show the given [Xcode](https://developer.apple.com/xcode) information**

    ```bash
    xcpkg xcinfo /Applications/Xcode12.app/Contents/Developer
    ```

- **save `tab-completion` script for zsh to a file**

    ```bash
    xcpkg completion zsh > _xcpkg
    sudo mv _xcpkg /usr/local/share/zsh/site-functions/
    ```

    This project provides a tab-completion script for `zsh` for `xcpkg`. when you've typed `xcpkg` then type `TAB` key, the rest of the arguments will be automatically complete for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit` in your terminal (your current running shell must be zsh).

- **update all available formula repositories**

    ```bash
    xcpkg update
    ```

- **search all available packages whose name matches the given regular expression pattern**

    ```bash
    xcpkg search curl
    xcpkg search curl -v
    xcpkg search '^lib'
    ```

- **show information of the given available package**

    ```bash
    xcpkg info-available curl
    xcpkg info-available curl --yaml
    xcpkg info-available curl --json
    xcpkg info-available curl version
    xcpkg info-available curl license
    xcpkg info-available curl summary
    xcpkg info-available curl web-url
    xcpkg info-available curl git-url
    xcpkg info-available curl git-sha
    xcpkg info-available curl git-ref
    xcpkg info-available curl src-url
    xcpkg info-available curl src-sha
    ```

- **show information of the given installed package**

    ```bash
    xcpkg info-installed iPhoneOS-12.0-arm64/curl
    xcpkg info-installed iPhoneOS-12.0-arm64/curl --prefix
    xcpkg info-installed iPhoneOS-12.0-arm64/curl --files
    xcpkg info-installed iPhoneOS-12.0-arm64/curl --yaml
    xcpkg info-installed iPhoneOS-12.0-arm64/curl --json
    xcpkg info-installed iPhoneOS-12.0-arm64/curl version
    xcpkg info-installed iPhoneOS-12.0-arm64/curl license
    xcpkg info-installed iPhoneOS-12.0-arm64/curl summary
    xcpkg info-installed iPhoneOS-12.0-arm64/curl web-url
    xcpkg info-installed iPhoneOS-12.0-arm64/curl git-url
    xcpkg info-installed iPhoneOS-12.0-arm64/curl git-sha
    xcpkg info-installed iPhoneOS-12.0-arm64/curl git-ref
    xcpkg info-installed iPhoneOS-12.0-arm64/curl src-url
    xcpkg info-installed iPhoneOS-12.0-arm64/curl src-sha
    xcpkg info-installed iPhoneOS-12.0-arm64/curl builtat
    xcpkg info-installed iPhoneOS-12.0-arm64/curl builtat-iso-8601
    xcpkg info-installed iPhoneOS-12.0-arm64/curl builtat-rfc-3339
    xcpkg info-installed iPhoneOS-12.0-arm64/curl builtat-iso-8601-utc
    xcpkg info-installed iPhoneOS-12.0-arm64/curl builtat-rfc-3339-utc
    ```

- **show packages that are depended by the given package**

    ```bash
    xcpkg depends curl

    xcpkg depends curl -t d2
    xcpkg depends curl -t dot
    xcpkg depends curl -t box
    xcpkg depends curl -t png
    xcpkg depends curl -t svg

    xcpkg depends curl -t d2  -o dependencies/
    xcpkg depends curl -t dot -o dependencies/
    xcpkg depends curl -t box -o dependencies/
    xcpkg depends curl -t png -o dependencies/
    xcpkg depends curl -t svg -o dependencies/

    xcpkg depends curl -o curl-dependencies.d2
    xcpkg depends curl -o curl-dependencies.dot
    xcpkg depends curl -o curl-dependencies.box
    xcpkg depends curl -o curl-dependencies.png
    xcpkg depends curl -o curl-dependencies.svg
    ```

- **download resources of the given package to the local cache**

    ```bash
    xcpkg fetch curl
    xcpkg fetch curl -v
    ```

- **install packages**

    ```bash
    xcpkg install curl
    xcpkg install curl -U
    xcpkg install curl --target=iPhoneOS-12.0-arm64
    xcpkg install curl --developer-dir=/Applications/Xcode12.app/Contents/Developer
    xcpkg install iPhoneOS-12.0-arm64/curl
    ```

- **reinstall packages**

    ```bash
    xcpkg reinstall curl
    xcpkg reinstall iPhoneOS-12.0-arm64/curl
    ```

- **uninstall packages**

    ```bash
    xcpkg uninstall curl
    xcpkg uninstall iPhoneOS-12.0-arm64/curl
    ```

- **upgrade the outdated packages**

    ```bash
    xcpkg upgrade
    xcpkg upgrade curl
    xcpkg upgrade iPhoneOS-12.0-arm64/curl
    ```

- **upgrade this software**

    ```bash
    xcpkg upgrade-self
    xcpkg upgrade-self -v
    ```

- **list all available formula repositories**

    ```bash
    xcpkg formula-repo-list
    ```

- **add a new formula repository**

    ```bash
    xcpkg formula-repo-add my_repo https://github.com/leleliu008/xcpkg-formula-repository-my_repo
    xcpkg formula-repo-add my_repo https://github.com/leleliu008/xcpkg-formula-repository-my_repo master
    xcpkg formula-repo-add my_repo https://github.com/leleliu008/xcpkg-formula-repository-my_repo main
    ```

- **delete a existing formula repository**

    ```bash
    xcpkg formula-repo-del my_repo
    ```

- **check if the given package is available**

    ```bash
    xcpkg is-available curl
    ```

- **check if the given package is installed**

    ```bash
    xcpkg is-installed curl
    xcpkg is-installed iPhoneOS-12.0-arm64/curl
    ```

- **check if the given package is outdated**

    ```bash
    xcpkg is-outdated  curl
    xcpkg is-outdated  iPhoneOS-12.0-arm64/curl
    ```

- **list all available packages**

    ```bash
    xcpkg ls-available
    xcpkg ls-available -v
    ```

- **list all installed packages**

    ```bash
    xcpkg ls-installed
    xcpkg ls-installed -v
    ```

- **list all outdated packages**

    ```bash
    xcpkg ls-outdated
    xcpkg ls-outdated -v
    ```

- **list installed files of the given installed package in a tree-like format**

    ```bash
    xcpkg tree curl
    xcpkg tree iPhoneOS-12.0-arm64/curl -L 3
    ```

- **show logs of the given installed package**

    ```bash
    xcpkg logs curl
    xcpkg logs iPhoneOS-12.0-arm64/curl
    ```

- **bundle the given installed package into a single archive file**

    ```bash
    xcpkg bundle iPhoneOS-12.0-arm64/curl .tar.xz
    xcpkg bundle iPhoneOS-12.0-arm64/curl .tar.gz
    xcpkg bundle iPhoneOS-12.0-arm64/curl .tar.lz
    xcpkg bundle iPhoneOS-12.0-arm64/curl .tar.bz2
    xcpkg bundle iPhoneOS-12.0-arm64/curl .zip
    xcpkg bundle iPhoneOS-12.0-arm64/curl a/.zip
    xcpkg bundle iPhoneOS-12.0-arm64/curl a/x.zip
    ```

- **delete the unused cached files**

    ```bash
    xcpkg cleanup
    ```

## influential environment variables

- **HOME**

    This environment variable already have been set on macOS, if not set or set a empty string, you will receive an error message.

- **PATH**

    This environment variable already have been set on macOS, if not set or set a empty string, you will receive an error message.

- **DEVELOPER_DIR**

    If this environment variable is set, `xcpkg` will use it as the Xcode developer dir, otherwise, `xcpkg` will run command `xcode-select -p` to determine the Xcode developer dir.

- **SSL_CERT_FILE**

    ```bash
    curl -LO https://curl.se/ca/cacert.pem
    export SSL_CERT_FILE="$PWD/cacert.pem"
    ```

    In general, you don't need to set this environment variable, but, if you encounter the reporting `the SSL certificate is invalid`, trying to run above commands in your terminal will do the trick.

- **GOPROXY**

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```

- **XCPKG_HOME**

    If this environment variable is not set or set a empty string, `$HOME/.xcpkg` will be used as the default value.

    ```bash
    export XCPKG_HOME=$HOME/xcpkg-home
    ```

- **XCPKG_GITHUB_PROXY**

    ```bash
    export XCPKG_GITHUB_PROXY='https://ghfast.top'
    ```

    This is very useful for chinese users.

- **XCPKG_XTRACE**

    for debugging purposes.

    enables `set -x`:

    ```bash
    export XCPKG_XTRACE=1
    ```

- **XCPKG_DEFAULT_TARGET**

    Some ACTIONs of xcpkg are associated with an installed package which need `PACKAGE-SPEC` to be specified.

    **PACKAGE-SPEC** : a formatted string that has form: `<TARGET-PLATFORM>/<PACKAGE-NAME>`, represents an installed package.

    **PACKAGE-NAME** : should match the regular expression pattern `^[A-Za-z0-9+-_.@]{1,50}$`

    **TARGET-PLATFORM** : a formatted string that has form: `<TARGET-PLATFORM-NAME>-<TARGET-PLATFORM-VERSION>-<TARGET-PLATFORM-ARCH>`

    **TARGET-PLATFORM-ARCH** : indicates which cpu arch was built for. value might be any one of `x86_64` `arm64`, etc

    **TARGET-PLATFORM-NAME** : indicates which platform name was built for. value shall be any one of `AppleTVOS` `AppleTVSimulator` `DriverKit` `MacOSX` `WatchOS` `WatchSimulator` `iPhoneOS` `iPhoneSimulator`

    **TARGET-PLATFORM-VERSION** : indicates which platform version was built with.

    To simplify the usage, you are allowed to omit `<TARGET-PLATFORM>/`. If `<TARGET-PLATFORM>/` is omitted, environment variable `XCPKG_DEFAULT_TARGET` would be checked, if this environment variable is not set, then your current running platform target will be used as the default.

    **Example**:

    ```bash
    export XCPKG_DEFAULT_TARGET=MacOSX-10.15-x86_64
    ```

## environment variables unset by this software

|ENV|used by|
|-|-|
|`SDKROOT`|`clang` `clang++`|
|`MACOSX_DEPLOYMENT_TARGET`|`clang` `clang++`|
|`WATCHOS_DEPLOYMENT_TARGET`|`clang` `clang++`|
|`IPHONEOS_DEPLOYMENT_TARGET`|`clang` `clang++`|
|`TARGET_ARCH`|`gmake`|

## environment variables overridden by this software

|ENV|used by|
|-|-|
|`CC`|`configure`|
|`CXX`|`configure`|
|`CPP`|`configure`|
|`AS`|`configure`|
|`AR`|`configure`|
|`LD`|`configure`|
|`CFLAGS`|`configure`|
|`CXXFLAGS`|`configure`|
|`CPPFLAGS`|`configure`|
|`LDFLAGS`|`configure`|
|`LIBS`|`configure`|
|||
|`CMAKE_GENERATOR`|`cmake`|
|`CMAKE_BUILD_PARALLEL_LEVEL`|`cmake`|
|`CMAKE_EXPORT_COMPILE_COMMANDS`|`cmake`|
|||
|`RUST_BACKTRACE`|`cargo`|
|`RUSTFLAGS`|`cargo`|
|`CARGO_BUILD_JOBS`|`cargo`|
|`CARGO_BUILD_TARGET`|`cargo`|
|`CARGO_BUILD_TARGET_DIR`|`cargo`|
|||
|`CGO_ENABLED`|`go`|
|`CGO_CFLAGS`|`go`|
|`CGO_CXXFLAGS`|`go`|
|`CGO_CPPFLAGS`|`go`|
|`CGO_LDFLAGS`|`go`|
|`GO111MODULE`|`go`|
|`GOOS`|`go`|
|`GOARCH`|`go`|
|||
|`PKG_CONFIG_LIBDIR`|`pkg-config`|
|`PKG_CONFIG_PATH`|`pkg-config`|
|`PKG_CONFIG_DEBUG_SPEW`|`pkg-config`|
|||
|`ACLOCAL_PATH`|`aclocal`|
|`XDG_DATA_DIRS`|`g-ir-scanner`|
|`XML_CATALOG_FILES`|`xsltproc`|
|`PERL_MM_USE_DEFAULT`|`cpan`|
|`BAT_THEME`|`bat`|
|`IFS`|`shell`|

## xcpkg formula scheme

a xcpkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a xcpkg package's meta-information including one sentence description, package version, installation instructions, etc.

a xcpkg formula's filename suffix must be `.yml`

a xcpkg formula'a filename prefix would be treated as the package name.

a xcpkg formula'a filename prefix must match regular expression pattern `^[A-Za-z0-9+-._@]{1,50}$`

a xcpkg formula's file content only has one level mapping and shall has following KEY:

|KEY|required?|overview|
|-|-|-|
|`pkgtype`|optional|specify the package type. value shall be any one of `exe`, `lib`, `exe+lib`.<br>If this mapping is not present, `xcpkg` will determine the package type by package name, if a package name starts/ends with `lib`, it would be recognized as type `lib`, otherwise, it would be recognized as type `exe`|
|`summary`|required|one sentence description of this package.|
|`license`|optional|a space-separated list of [SPDX license short identifiers](https://spdx.github.io/spdx-spec/v2.3/SPDX-license-list/#a1-licenses-with-short-identifiers)|
|`version`|optional|the version of this package.<br>If this mapping is not present, it will be calculated from `src-url`, if `src-url` is also not present, it will be calculated from running time as format `date +%Y.%m.%d`|
||||
|`web-url`|optional|the home webpage of this package.<br>If this mapping is not present, `git-url` must be present.|
||||
|`git-url`|optional|the source code git repository.<br>If `src-url` is not present, this mapping must be present.|
|`git-ref`|optional|reference: <https://git-scm.com/book/en/v2/Git-Internals-Git-References> <br>example values: `HEAD` `refs/heads/master` `refs/heads/main` `refs/tags/v1`, default value is `HEAD`|
|`git-sha`|optional|the full git commit id, 40-byte hexadecimal string, if `git-ref` and `git-sha` both are present, `git-sha` takes precedence over `git-ref`|
|`git-nth`|optional|tell `xcpkg` that how many depth commits would you like to be fetched. default is `1`, this would save your time and storage. If you want to fetch all commits, set this to `0`|
||||
|`src-url`|optional|the source code download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/src` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/src`<br>also support format like `dir://DIR`|
|`src-uri`|optional|the mirror of `src-url`.|
|`src-sha`|optional|the `sha256sum` of source code.<br>`src-sha` and `src-url` must appear together.|
||||
|`fix-url`|optional|the patch file download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/fix` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/fix`.|
|`fix-uri`|optional|the mirror of `fix-url`.|
|`fix-sha`|optional|the `sha256sum` of patch file.<br>`fix-sha` and `fix-url` must appear together.|
|`fix-opt`|optional|options to be passed to `patch` command. default value is `-p1`.|
||||
|`patches`|optional|multiple lines of `<fix-sha>\|<fix-url>[\|fix-uri][\|fix-opt]`.|
||||
|`res-url`|optional|other resource download url of this package.<br>If value of this mapping ends with one of `.zip` `.tar.xz` `.tar.gz` `.tar.lz` `.tar.bz2` `.tgz` `.txz` `.tlz` `.tbz2` `.crate`, it will be uncompressed to `$PACKAGE_WORKING_DIR/res` when this package is installing, otherwise, it will be copied to `$PACKAGE_WORKING_DIR/res`.|
|`res-uri`|optional|the mirror of `res-url`.|
|`res-sha`|optional|the `sha256sum` of resource file.<br>`res-sha` and `res-url` must appear together.|
||||
|`reslist`|optional|multiple lines of `<res-sha>\|<res-url>[\|res-uri][\|unpack-dir][\|N]`. `unpack-dir` is relative to `$PACKAGE_WORKING_DIR/res`, default value is empty. `N` is `--strip-components=N`|
||||
|`dep-pkg`|optional|a space-separated list of   `xcpkg packages` that are depended by this package when installing and/or runtime, which will be installed via [xcpkg](https://github.com/leleliu008/xcpkg).|
|`dep-lib`|optional|a space-separated list of libraries that will be linked. library name starts with `-l` will be directly passed to the linker. otherwise, it will be recognized as a `pkg-config` package name and it will be calculated via `pkg-config --libs-only-l ` then passed to the linker.|
|`dep-upp`|optional|a space-separated list of   `uppm packages` that are depended by this package when installing and/or runtime, which will be installed via [uppm](https://github.com/leleliu008/uppm).|
|`dep-pym`|optional|a space-separated list of `python packages` that are depended by this package when installing and/or runtime, which will be installed via [pip3](https://github.com/pypa/pip).|
|`dep-plm`|optional|a space-separated list of    `perl modules` that are depended by this package when installing and/or runtime, which will be installed via [cpan](https://metacpan.org/dist/CPAN/view/scripts/cpan).|
||||
|`ccflags`|optional|append to `CFLAGS`|
|`xxflags`|optional|append to `CXXFLAGS`|
|`ppflags`|optional|append to `CPPFLAGS`|
|`ldflags`|optional|append to `LDFLAGS`|
||||
|`bsystem`|optional|build system name.<br>values can be one or a combination of `autogen` `autotools` `configure` `cmake` `cmake+gmake` `cmake+ninja` `meson` `xmake` `gmake` `ninja` `cargo` `cabal` `go` `gn` `rake` `waf`|
|`bscript`|optional|the directory where the build script is located in, relative to `PACKAGE_WORKING_DIR`. build script such as `configure`, `Makefile`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc.|
|`binbstd`|optional|whether to build in the directory where the build script is located in, otherwise build in other directory.<br>value shall be `0` or `1`. default value is `0`.|
|`movable`|optional|whether the installed files can be moved/copied to other locations.<br>value shall be `0` or `1`. default value is `1`.|
||||
|`ltoable`|optional|whether support [Link Time Optimization](https://llvm.org/docs/LinkTimeOptimization.html).<br>value shall be `0` or `1`. default value is `1`.|
|`mslable`|optional|whether support creating Mostly Statically Linked executables.<br>value shall be `0` or `1`. default value is `1`.<br>This mapping is only for `exe` type of package.|
|`parallel`|optional|whether support building in parallel.<br>value shall be `0` or `1`. default value is `1`.|
||||
|`dofetch`|optional|POSIX shell code to be run to take over the fetching process.<br>`PWD` is `$PACKAGE_WORKING_DIR`|
|`do12345`|optional|POSIX shell code to be run for native build.<br>It would be run in a separate process.|
|`dopatch`|optional|POSIX shell code to be run to apply patches manually.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR`|
|`prepare`|optional|POSIX shell code to be run to do some additional preparation.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR`|
|`install`|optional|POSIX shell code to be run when user run `xcpkg install <PKG>`. If this mapping is not present, `xcpkg` will run default install code according to `bsystem`.<br>`PWD` is `$PACKAGE_BSCRIPT_DIR` if `binbstd` is `0`, otherwise it is `$PACKAGE_BCACHED_DIR`|
|`dotweak`|optional|POSIX shell code to be run to do some tweaks immediately after installing.<br>`PWD` is `$PACKAGE_INSTALL_DIR`|
||||
|`bindenv`|optional|bind environment variables to executables. multiple lines of formatted string `<KEY>=<VALUE>`. `%s` in `<VALUE>` represents the install directory.|
||||
|`caveats`|optional|multiple lines of plain text to be displayed after installation.|

|phases of a package's installation|
|-|
|![phases](phases.svg)|

|build system name|build script file name|
|-|-|
|`meson`|`meson.build`|
|`cmake`|`CMakeLists.txt`|
|`gmake`|`GNUMakefile` or `Makefile`|
|`ninja`|`build.ninja`|
|`xmake`|`xmake.lua`|
|`cabal`|`cabal.project` `cabal.project.freeze` `cabal.project.local`|
|`cargo`|`Cargo.toml`|
|`go`|`go.mod`|
|`gn`|`BUILD.gn`|
|`rake`|`Rakefile`|
|`autogen`|`autogen.sh`|
|`autotools`|`configure.ac`|
|`configure`|`configure`|

**commands that can be used out of the box:**

|command|usage-example|
|-|-|
|`bash`|[Reference](https://www.gnu.org/software/bash/manual/bash.html)|
|`CoreUtils`|[Reference](https://www.gnu.org/software/coreutils/manual/coreutils.html)|
|`xargs`|[Reference](https://www.gnu.org/software/findutils/manual/html_node/find_html/Invoking-xargs.html)|
|`find`|[Reference](https://www.gnu.org/software/findutils/manual/html_mono/find.html)|
|`gawk`|[Reference](https://www.gnu.org/software/gawk/manual/gawk.html)|
|`gsed`|[Reference](https://www.gnu.org/software/sed/manual/sed.html)|
|`grep`|[Reference](https://www.gnu.org/software/grep/manual/grep.html)|
|`tree`|[Reference](https://linux.die.net/man/1/tree)|
|`pkg-config`|[Reference](https://people.freedesktop.org/~dbn/pkg-config-guide.html)|
|||
|`echo`|`echo 'your message.'`|
|`info`|`info 'your information.'`|
|`warn`|`warn "no package manager found."`|
|`error`|`error 'error message.'`|
|`abort`|`abort 1 "please specify a package name."`|
|`success`|`success "build success."`|
|`isInteger`|`isInteger $x \|\| abort 1 "should be an integer."`|
|`isCrossBuild`|`isCrossBuild && abort 1 "This package is not supposed to be cross built."`|
|`wfetch`|`wfetch <URL> [--uri=<URL-MIRROR>] [--sha256=<SHA256>] [-o <PATH> [-q]`|
|||
|`configure`|`configure --enable-pic`|
|`mesonw`|`mesonw -Dneon=disabled -Darm-simd=disabled`|
|`cmakew`|`cmakew -DBUILD_SHARED_LIBS=ON -DBUILD_STATIC_LIBS=ON`|
|`gmakew`|`gmakew`|
|`xmakew`|`xmakew`|
|`cargow`|`cargow`|
|`gow`|`gow`|

**shell variables can be used directly:**

|variable|overview|
|-|-|
|`TIMESTAMP_UNIX`|the unix timestamp of this action.|
|||
|`NATIVE_PLATFORM_KIND`|current running platform kind. value shall be `darwin`|
|`NATIVE_PLATFORM_TYPE`|current running platform type. value shall be `macos`|
|`NATIVE_PLATFORM_NAME`|current running platform name. value shall be `MacOSX`|
|`NATIVE_PLATFORM_VERS`|current running platform version.|
|`NATIVE_PLATFORM_ARCH`|current running platform arch. value might be any one of `x86_64`, `arm64`, etc|
|`NATIVE_PLATFORM_NCPU`|current running platform's cpu core count.|
|`NATIVE_PLATFORM_EUID`|current running platform's effective user ID.|
|`NATIVE_PLATFORM_EGID`|current running platform's effective group ID.|
|||
|`TARGET_PLATFORM_NAME`|target platform name that is built for. value shall be any one of `AppleTVOS` `AppleTVSimulator` `DriverKit` `MacOSX` `WatchOS` `WatchSimulator` `iPhoneOS` `iPhoneSimulator`|
|`TARGET_PLATFORM_VERS`|target platform version that is built with.|
|`TARGET_PLATFORM_ARCH`|target platform arch that is built for. value might be any one of `x86_64`, `arm64`, etc|
|||
|`CROSS_COMPILING`|value shall be 0 or 1. indicates whether is cross-compiling.|
|||
|`XCPKG_PATH`|the full path of `xcpkg` that you're running.|
|`XCPKG_HOME`|the home directory of `xcpkg` that you're running.|
|`XCPKG_VERSION`|the version of `xcpkg` that you're running.|
|||
|`CC_FOR_BUILD`|the C Compiler for native build.|
|`CFLAGS_FOR_BUILD`|the flags of `CC_FOR_BUILD`.|
|`CXX_FOR_BUILD`|the C++ Compiler for native build.|
|`CXXFLAGS_FOR_BUILD`|the flags of `CXX_FOR_BUILD`.|
|`CPP_FOR_BUILD`|the C/C++ PreProcessor for native build.|
|`CPPFLAGS_FOR_BUILD`|the flags of `CPP_FOR_BUILD`.|
|`AS_FOR_BUILD`|the assembler for native build.|
|`AR_FOR_BUILD`|the archiver for native build.|
|`RANLIB_FOR_BUILD`|the archiver extra tool for native build.|
|`LD_FOR_BUILD`|the linker for native build.|
|`LDFLAGS_FOR_BUILD`|the flags of `LD_FOR_BUILD`.|
|`NM_FOR_BUILD`|a command line tool to list symbols from object files for native build.|
|`STRIP_FOR_BUILD`|a command line tool to discard symbols and other data from object files for native build.|
|||
|`CC`|the C Compiler.|
|`CFLAGS`|the flags of `CC`.|
|`CXX`|the C++ Compiler.|
|`CXXFLAGS`|the flags of `CXX`.|
|`CPP`|the C/C++ PreProcessor.|
|`CPPFLAGS`|the flags of `CPP`.|
|`AS`|the assembler.|
|`AR`|the archiver.|
|`RANLIB`|the archiver extra tool.|
|`LD`|the linker.|
|`LDFLAGS`|the flags of `LD`.|
|`NM`|a command line tool to list symbols from object files.|
|`STRIP`|a command line tool to discard symbols and other data from object files.|
|||
|`PACKAGE_WORKING_DIR`|the working directory when installing.|
|`PACKAGE_BSCRIPT_DIR`|the directory where the build script (e.g. `Makefile`, `configure`, `CMakeLists.txt`, `meson.build`, `Cargo.toml`, etc) is located in.|
|`PACKAGE_BCACHED_DIR`|the directory where the temporary files are stored in when building.|
|`PACKAGE_INSTALL_DIR`|the directory where the final files will be installed to.|
|||
|`x_INSTALL_DIR`|the installation directory of x package.|
|`x_INCLUDE_DIR`|`$x_INSTALL_DIR/include`|
|`x_LIBRARY_DIR`|`$x_INSTALL_DIR/lib`|

## xcpkg formula repository

a typical hierarchical structure of a xcpkg formula repository looks like below:

```
XCPKGFormulaRepoName
├── formula
│   ├── packageA.yml
│   └── packageB.yml
├── LICENSE
└── README.md
```

## xcpkg formula repository local location

`${XCPKG_HOME}/repos.d/${XCPKGFormulaRepoName}`

## xcpkg formula repository local config

a xcpkg formula repository's config file is located at `${XCPKG_HOME}/repos.d/${XCPKGFormulaRepoName}/.xcpkg-formula-repo.yml`

a typical xcpkg formula repository's config file content looks like below:

```yaml
url: https://github.com/leleliu008/xcpkg-formula-repository-official-core
branch: master
pinned: 0
enabled: 1
created: 1673684639
updated: 1673684767
```

If a xcpkg formula repository is `pinned`, which means it would not be updated.

If a xcpkg formula repository is `disabled`, which means xcpkg would not search formulas in this formula repository.

## xcpkg formula repository management

run `xcpkg formula-repo-add ` command to create a new formula repository locally from an exsting remote git repository.

run `xcpkg formula-repo-init` command to create a new formula repository locally without taking any further action.

## xcpkg official formula repository

xcpkg official formula repository is hosted at <https://github.com/leleliu008/xcpkg-formula-repository-official-core>

It would be automatically fetched to your local repository as name `official-core` when you run `xcpkg update` command.

**Note:** If you find that a package is not in xcpkg official formula repository yet, PR is welcomed.

## prebuild packages built by this software

- <https://github.com/leleliu008/uppm-package-repository-macos-10.15-x86_64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-11.0-x86_64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-12.0-x86_64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-13.0-x86_64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-14.0-x86_64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-11.0-arm64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-12.0-arm64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-13.0-arm64/releases>
- <https://github.com/leleliu008/uppm-package-repository-macos-14.0-arm64/releases>
