# xcpkg

A package manager for [Xcode](https://developer.apple.com/xcode) to build C/C++/Rust/Go project.

## Caveats

- This software is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

- Please do NOT place your own files under `~/.xcpkg` directory, as `xcpkg` will change files under `~/.xcpkg` directory without notice.

- Please do NOT run `xcpkg` command in parallell to avoid generating dirty data.

## Manually build packages using this software via GitHub Actions

In this way, you will be liberated from the rut of setting up the build environemt.

In this way, all you need to do is just clicking the buttons and waiting for finishing. After finishing, a url refers to a zip archive will be provided to download.

For more details please refer to <https://github.com/leleliu008/xcpkg-package-manually-build>

## Install xcpkg via cURL

```bash
curl -LO https://raw.githubusercontent.com/leleliu008/xcpkg/master/xcpkg
chmod a+x xcpkg
./xcpkg setup
```

## Install xcpkg via wget

```bash
wget https://cdn.jsdelivr.net/gh/leleliu008/xcpkg/xcpkg
chmod a+x xcpkg
./xcpkg setup
```

## Install xcpkg via git

```bash
git clone --depth 1 https://github.com/leleliu008/xcpkg
xcpkg/xcpkg setup
```

## ~/.xcpkg

all relevant directories and files are located under `~/.xcpkg` directory.

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
    xcpkg env
    ```

- **show basic information about your current running operation system**

    ```bash
    xcpkg sysinfo
    ```

- **show your current actived [Xcode](https://developer.apple.com/xcode) information**

    ```bash
    xcpkg xcinfo
    ```

- **show the given [Xcode](https://developer.apple.com/xcode) information**

    ```bash
    xcpkg xcinfo --developer-dir=/Applications/Xcode12.app/Contents/Developer
    ```

- **show the supported target platform names only**

    ```bash
    xcpkg xcinfo --list-supported-platform-names
    xcpkg xcinfo --list-supported-platform-names --developer-dir=/Applications/Xcode12.app/Contents/Developer
    ```

- **generate url-transform sample**

    ```bash
    xcpkg gen-url-transform-sample
    ```

- **install essential tools**

    ```bash
    xcpkg setup
    ```

    This command is actually to do two things:

  - install [uppm](https://github.com/leleliu008/uppm) to `~/.xcpkg/core`
  - install other essential tools (listed below) that are used by this shell script via [uppm](https://github.com/leleliu008/uppm)

    - [GNU Bash](https://www.gnu.org/software/bash/manual/bash.html)
    - [GNU CoreUtils](https://www.gnu.org/software/coreutils/manual/coreutils.html)
    - [GNU FindUtils](https://www.gnu.org/software/findutils/manual/html_mono/find.html)
    - [GNU awk](https://www.gnu.org/software/gawk/manual/gawk.html)
    - [GNU sed](https://www.gnu.org/software/sed/manual/sed.html)
    - [GNU grep](https://www.gnu.org/software/grep/manual/grep.html)
    - [BSD tar](https://man.archlinux.org/man/core/libarchive/bsdtar.1.en)
    - [tree](https://linux.die.net/man/1/tree)
    - [curl](https://curl.se/docs/manpage.html)
    - [git](https://git-scm.com/docs/git)
    - [yq](https://mikefarah.gitbook.io/yq/)
    - [jq](https://stedolan.github.io/jq/manual/)

- **integrate `zsh-completion` script**

    ```bash
    xcpkg integrate zsh
    xcpkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
    xcpkg integrate zsh -v
    ```

    This project provides a zsh-completion script for `xcpkg`. when you've typed `xcpkg` then type `TAB` key, the rest of the arguments will be automatically complete for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit` in your terminal (your current running shell must be zsh).

- **update all available formula repositories**

    ```bash
    xcpkg update
    ```

- **search all available packages whose name matches the given regular expression pattern**

    ```bash
    xcpkg search curl
    xcpkg search lib
    ```

- **show information of all available packages**

    ```bash
    xcpkg info @all
    ```

- **show information of the given available package**

    ```bash
    xcpkg info curl
    xcpkg info curl --yaml
    xcpkg info curl --json
    xcpkg info curl version
    xcpkg info curl license
    xcpkg info curl summary
    xcpkg info curl web-url
    xcpkg info curl git-url
    xcpkg info curl git-sha
    xcpkg info curl git-ref
    xcpkg info curl src-url
    xcpkg info curl src-sha
    ```

- **show information of the given installed package**

    ```bash
    xcpkg info iPhoneOS-12.0-arm64/curl
    xcpkg info iPhoneOS-12.0-arm64/curl --yaml
    xcpkg info iPhoneOS-12.0-arm64/curl --json
    xcpkg info iPhoneOS-12.0-arm64/curl --path
    xcpkg info iPhoneOS-12.0-arm64/curl version
    xcpkg info iPhoneOS-12.0-arm64/curl license
    xcpkg info iPhoneOS-12.0-arm64/curl summary
    xcpkg info iPhoneOS-12.0-arm64/curl web-url
    xcpkg info iPhoneOS-12.0-arm64/curl git-url
    xcpkg info iPhoneOS-12.0-arm64/curl git-sha
    xcpkg info iPhoneOS-12.0-arm64/curl git-ref
    xcpkg info iPhoneOS-12.0-arm64/curl src-url
    xcpkg info iPhoneOS-12.0-arm64/curl src-sha
    xcpkg info iPhoneOS-12.0-arm64/curl builtat
    xcpkg info iPhoneOS-12.0-arm64/curl builtat-iso-8601
    xcpkg info iPhoneOS-12.0-arm64/curl builtat-rfc-3339
    xcpkg info iPhoneOS-12.0-arm64/curl builtat-iso-8601-utc
    xcpkg info iPhoneOS-12.0-arm64/curl builtat-rfc-3339-utc
    xcpkg info iPhoneOS-12.0-arm64/curl installed-dir
    xcpkg info iPhoneOS-12.0-arm64/curl installed-files
    ```

- **show packages that are depended by the given package**

    ```bash
    xcpkg depends curl

    xcpkg depends curl -t dot
    xcpkg depends curl -t box
    xcpkg depends curl -t png
    xcpkg depends curl -t svg

    xcpkg depends curl -t dot -o dependencies/
    xcpkg depends curl -t box -o dependencies/
    xcpkg depends curl -t png -o dependencies/
    xcpkg depends curl -t svg -o dependencies/

    xcpkg depends curl -o curl-dependencies.dot
    xcpkg depends curl -o curl-dependencies.box
    xcpkg depends curl -o curl-dependencies.png
    xcpkg depends curl -o curl-dependencies.svg
    ```

- **download resources of the given package to the local cache**

    ```bash
    xcpkg fetch curl
    xcpkg fetch @all

    xcpkg fetch curl -v
    xcpkg fetch @all -v
    ```

- **install packages**

    ```bash
    xcpkg install curl
    xcpkg install curl --upgrade
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

- **edit the formula of the given package**

    ```bash
    xcpkg formula-edit curl
    xcpkg formula-edit curl --editor=/usr/local/bin/vim
    ```

    **Note**: xcpkg do NOT save your changes, which means that your changes may be lost after the formula repository is updated!

- **list all avaliable formula repositories**

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
    ```

- **list all installed packages**

    ```bash
    xcpkg ls-installed
    ```

- **list all outdated packages**

    ```bash
    xcpkg ls-outdated
    ```

- **list installed files of the given installed package**

    ```bash
    xcpkg list curl
    xcpkg list iPhoneOS-12.0-arm64/curl -L 3
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

- **pack the given installed package**

    ```bash
    xcpkg pack curl
    xcpkg pack iPhoneOS-12.0-arm64/curl
    xcpkg pack iPhoneOS-12.0-arm64/curl -t tar.xz
    xcpkg pack iPhoneOS-12.0-arm64/curl -t tar.gz
    xcpkg pack iPhoneOS-12.0-arm64/curl -t tar.lz
    xcpkg pack iPhoneOS-12.0-arm64/curl -t tar.bz2
    xcpkg pack iPhoneOS-12.0-arm64/curl -t zip
    xcpkg pack iPhoneOS-12.0-arm64/curl -t zip -o a/
    xcpkg pack iPhoneOS-12.0-arm64/curl -o a/x.zip
    ```

- **delete the unused cached files**

    ```bash
    xcpkg cleanup
    ```

## environment variables

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

- **XCPKG_URL_TRANSFORM**

    ```bash
    export XCPKG_URL_TRANSFORM=/path/of/url-transform
    ```

    `/path/of/url-transform` command would be invoked as `/path/of/url-transform <URL>`

    `/path/of/url-transform` command must output a `<URL>`

    you can generate a url-transform sample via `xcpkg gen-url-transform-sample`

    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.

- **XCPKG_XTRACE**

    for debugging purposes.

    enables `set -x`:

    ```bash
    export XCPKG_XTRACE=1
    ```

- **XCPKG_DEFAULT_TARGET**

    some commands need `<PACKAGE-SPEC>` to be specified. `<PACKAGE-SPEC>` has the form `<TARGET>/<PACKAGE-NAME>`. To simplify the usage, `xcpkg` allows omitting `<TARGET>/`. If `<TARGET>/` is omitted, this environment variable will be used, if this environment variable is not set, then will retrive your current running system's information.

    `<TARGET>` has the form `<TARGET-PLATFORM-NAME>-<TARGET-PLATFORM-VERSION>-<TARGET-PLATFORM-ARCH>`

    example:

    ```bash
    export XCPKG_DEFAULT_TARGET=MacOSX-10.15-x86_64
    ```

**Note:** some commonly used environment variables are override by this software, these are `CC`, `CXX`, `CPP`, `AS`, `AR`, `LD`, `CFLAGS`, `CPPFLAGS`, `LDFLAGS`, `PKG_CONFIG_LIBDIR`, `PKG_CONFIG_PATH`, `ACLOCAL_PATH`

## xcpkg formula

a xcpkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a xcpkg package's meta-information including one sentence description, package version, installation instructions, etc.

a xcpkg formula's filename suffix must be `.yml`

a xcpkg formula'a filename prefix would be treated as the package name.

a xcpkg formula'a filename prefix must match regular expression pattern `^[A-Za-z0-9+-._@]{1,50}$`

a xcpkg formula's file content must follow [the xcpkg formula scheme](https://github.com/leleliu008/xcpkg-formula-repository-offical-core)

## xcpkg formula repository

a xcpkg formula repository is a git repository.

a xcpkg formula repository's root dir should have a `formula` named sub dir, this repository's formulas all should be located in this dir.

a xcpkg formula repository's local path is `~/.xcpkg/repos.d/${XCPKGFormulaRepoName}`

xcpkg supports multiple formula repositories.

## xcpkg formula repository's config

After a xcpkg formula repository is successfully fetched from server to local, a config file for this repository would be created at `~/.xcpkg/repos.d/${XCPKGFormulaRepoName}/.xcpkg-formula-repo.yml`

a typical xcpkg formula repository's config as following:

```yaml
url: https://github.com/leleliu008/xcpkg-formula-repository-offical-core
branch: master
pinned: 0
enabled: 1
created: 1673684639
updated: 1673684767
```

If a xcpkg formula repository is `pinned`, which means it would not be updated.

If a xcpkg formula repository is `disabled`, which means xcpkg would not search formulas in this formula repository.

## xcpkg offical formula repository

xcpkg offical formula repository's url: <https://github.com/leleliu008/xcpkg-formula-repository-offical-core>

xcpkg offical formula repository would be automatically fetched to local cache as name `offical-core` when you run `xcpkg update` command.

**Note:** If you find that a package is not in xcpkg offical formula repository yet, PR is welcomed.

## xcpkg offical prebuild package repository

I have built and packed some commonly used packages using this software:

- <https://github.com/leleliu008/uppm-package-repository-macos10.15-x86_64>
- <https://github.com/leleliu008/uppm-package-repository-macos11.0-x86_64>
- <https://github.com/leleliu008/uppm-package-repository-macos12.0-x86_64>
- <https://github.com/leleliu008/uppm-package-repository-macos13.0-x86_64>
- <https://github.com/leleliu008/uppm-package-repository-macos11.0-arm64>
- <https://github.com/leleliu008/uppm-package-repository-macos12.0-arm64>
- <https://github.com/leleliu008/uppm-package-repository-macos13.0-arm64>
