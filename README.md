# xcpkg

a package manager for [Xcode](https://developer.apple.com/xcode) to build C/C++/Rust/Go project.

<br>

**Note**: This project is being actively developed. It's in beta stage and may not be stable. Some features are subject to change without notice.

## Install xcpkg via cURL

```bash
curl -LO https://raw.githubusercontent.com/leleliu008/xcpkg/master/xcpkg
chmod a+x xcpkg
mv xcpkg /usr/local/bin/
xcpkg setup
# xcpkg setup --use-brew
```

## ~/.xcpkg

all relevant dirs and files are located in `~/.xcpkg` directory.

**Note**: Please do NOT place your own files in `~/.xcpkg` directory, as `xcpkg` will change files in `~/.xcpkg` directory without notice.

## xcpkg command usage

* **show help of this command**

    ```bash
    xcpkg -h
    xcpkg --help
    ```

* **show version of this command**

    ```bash
    xcpkg -V
    xcpkg --version
    ```

* **show your system's information**

    ```bash
    xcpkg sysinfo
    ```

* **show your current actived [Xcode](https://developer.apple.com/xcode) information**

    ```bash
    xcpkg xcinfo
    ```

* **show the given [Xcode](https://developer.apple.com/xcode) information**

    ```bash
    xcpkg xcinfo --developer-dir=/Applications/Xcode12.app/Contents/Developer
    ```

* **show the supported target platform names only**

    ```bash
    xcpkg xcinfo --list-target-platform-names-only
    xcpkg xcinfo --list-target-platform-names-only --developer-dir=/Applications/Xcode12.app/Contents/Developer
    ```

* **show your system and current actived [Xcode](https://developer.apple.com/xcode) information**

    ```bash
    xcpkg env
    ```

* **install essential tools via uppm**

    ```bash
    xcpkg setup
    ```

    **Note**:
  * above command will install all the essential tools (e.g. `coreutils` `findutils` `gawk` `gsed` `grep` `gtar` `gzip` `lzip` `unzip` `zip` `jq` `yq` `git` `curl` `tree`) that are used by this shell script via [uppm](https://github.com/leleliu008/uppm)
  * dependent tools (e.g. `automake`, `autoconf`, `libtool`, `gmake`, `cmake`, `xmake`, etc) when installing package will also be installed via [uppm](https://github.com/leleliu008/uppm).

* **install essential tools via homebrew**

    ```bash
    xcpkg setup --use-brew
    ```

    **Note**:
  * above command will install all the essential tools (e.g. `coreutils` `findutils` `gawk` `gsed` `grep` `gnu-tar` `gzip` `lzip` `unzip` `zip` `jq` `yq` `git` `curl` `tree`) that are used by this shell script via [homebrew](https://brew.sh/)
  * dependent tools (e.g. `automake`, `autoconf`, `libtool`, `gmake`, `cmake`, `xmake`, etc) when installing package will also be installed via [homebrew](https://brew.sh/).

* **integrate `zsh-completion` script**

    ```bash
    xcpkg integrate zsh
    xcpkg integrate zsh --output-dir=/usr/local/share/zsh/site-functions
    xcpkg integrate zsh -v
    ```

    This project provides a zsh-completion script for `xcpkg`. when you've typed `xcpkg` then type `TAB` key, the rest of the arguments will be automatically complete for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit` in your terminal (your current running shell must be zsh).

* **update all available formula repositories**

    ```bash
    xcpkg update
    ```

* **search all available packages whose name matches the given regular express pattern**

    ```bash
    xcpkg search curl
    xcpkg search lib
    ```

* **show information of the given package**

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

    xcpkg info curl installed-dir
    xcpkg info curl installed-files
    xcpkg info curl installed-version
    xcpkg info curl installed-timestamp-unix
    xcpkg info curl installed-timestamp-iso-8601
    xcpkg info curl installed-timestamp-rfc-3339
    xcpkg info curl installed-timestamp-iso-8601-utc
    xcpkg info curl installed-timestamp-rfc-3339-utc

    xcpkg info @all
    ```

* **show formula of the given package**

    ```bash
    xcpkg formula curl
    xcpkg formula curl --yaml
    xcpkg formula curl --json
    xcpkg formula curl --path
    xcpkg formula curl version
    xcpkg formula curl license
    xcpkg formula curl summary
    xcpkg formula curl web-url
    xcpkg formula curl git-url
    xcpkg formula curl git-sha
    xcpkg formula curl git-ref
    xcpkg formula curl src-url
    xcpkg formula curl src-sha
    ```

* **show receipt of the given installed package**

    ```bash
    xcpkg receipt iPhoneOS/9.0/armv7s/curl
    xcpkg receipt iPhoneOS/9.0/armv7s/curl --yaml
    xcpkg receipt iPhoneOS/9.0/armv7s/curl --json
    xcpkg receipt iPhoneOS/9.0/armv7s/curl --path
    xcpkg receipt iPhoneOS/9.0/armv7s/curl version
    xcpkg receipt iPhoneOS/9.0/armv7s/curl license
    xcpkg receipt iPhoneOS/9.0/armv7s/curl summary
    xcpkg receipt iPhoneOS/9.0/armv7s/curl web-url
    xcpkg receipt iPhoneOS/9.0/armv7s/curl git-url
    xcpkg receipt iPhoneOS/9.0/armv7s/curl git-sha
    xcpkg receipt iPhoneOS/9.0/armv7s/curl git-ref
    xcpkg receipt iPhoneOS/9.0/armv7s/curl src-url
    xcpkg receipt iPhoneOS/9.0/armv7s/curl src-sha
    ```

* **show packages that are depended by the given package**

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

* **download resources of the given package to the local cache**

    ```bash
    xcpkg fetch curl
    xcpkg fetch @all

    xcpkg fetch curl -v
    xcpkg fetch @all -v
    ```

* **install packages**

    ```bash
    xcpkg install curl
    xcpkg install curl bzip2 -v
    ```

* **reinstall packages**

    ```bash
    xcpkg reinstall curl
    xcpkg reinstall curl bzip2 -v
    ```

* **uninstall packages**

    ```bash
    xcpkg uninstall curl
    xcpkg uninstall curl bzip2 -v
    ```

* **upgrade the outdated packages**

    ```bash
    xcpkg upgrade
    xcpkg upgrade curl
    xcpkg upgrade curl bzip2 -v
    ```

* **upgrade this software**

    ```bash
    xcpkg upgrade-self
    xcpkg upgrade-self -v
    ```

* **edit the formula of the given package**

    ```bash
    xcpkg formula-edit curl
    xcpkg formula-edit curl --editor=/usr/local/bin/vim
    ```

    **Note**: xcpkg do NOT save your changes, which means that your changes may be lost after the formula repository is updated!

* **list all avaliable formula repositories**

    ```bash
    xcpkg formula-repo-list
    ```

* **add a new formula repository**

    ```bash
    xcpkg formula-repo-add my_repo https://github.com/leleliu008/xcpkg-formula-repository-my_repo
    xcpkg formula-repo-add my_repo https://github.com/leleliu008/xcpkg-formula-repository-my_repo master
    xcpkg formula-repo-add my_repo https://github.com/leleliu008/xcpkg-formula-repository-my_repo main
    ```

* **delete a existing formula repository**

    ```bash
    xcpkg formula-repo-del my_repo
    ```

* **list all available packages**

    ```bash
    xcpkg ls-available
    ```

* **list all installed packages**

    ```bash
    xcpkg ls-installed
    ```

* **list all outdated packages**

    ```bash
    xcpkg ls-outdated
    ```

* **check if the given package is available**

    ```bash
    xcpkg is-available curl
    ```

* **check if the given package is installed**

    ```bash
    xcpkg is-installed curl
    xcpkg is-installed iPhoneOS/9.0/armv7s/curl
    ```

* **check if the given package is outdated**

    ```bash
    xcpkg is-outdated  curl
    xcpkg is-outdated  iPhoneOS/9.0/armv7s/curl
    ```

* **list installed files of the given installed package in a tree-like format**

    ```bash
    xcpkg tree curl
    xcpkg tree iPhoneOS/9.0/armv7s/curl -L 3
    ```

* **show logs of the given installed package**

    ```bash
    xcpkg logs curl
    xcpkg logs curl iPhoneOS/9.0/armv7s/curl
    xcpkg logs curl iPhoneOS/9.0/arm64/curl
    ```

* **pack the given installed package**

    ```bash
    xcpkg pack curl
    xcpkg pack iPhoneOS/9.0/armv7s/curl
    xcpkg pack iPhoneOS/9.0/armv7s/curl -t tar.xz
    xcpkg pack iPhoneOS/9.0/armv7s/curl -t tar.gz
    xcpkg pack iPhoneOS/9.0/armv7s/curl -t tar.lz
    xcpkg pack iPhoneOS/9.0/armv7s/curl -t tar.bz2
    xcpkg pack iPhoneOS/9.0/armv7s/curl -t zip
    xcpkg pack iPhoneOS/9.0/armv7s/curl -t zip -o a/
    xcpkg pack iPhoneOS/9.0/armv7s/curl -o a/x.zip
    ```

* **generate url-transform sample**

    ```bash
    xcpkg gen-url-transform-sample
    ```

* **delete the unused cached files**

    ```bash
    xcpkg cleanup
    ```

## environment variables

* **HOME**

    This environment variable already have been set on macOS, if not set or set a empty string, you will receive an error message.

* **PATH**

    This environment variable already have been set on macOS, if not set or set a empty string, you will receive an error message.

* **DEVELOPER_DIR**

    If this environment variable is set, `xcpkg` will use it as the Xcode developer dir, otherwise, `xcpkg` will run command `xcode-select -p` to determine the Xcode developer dir.

* **SSL_CERT_FILE**

    ```bash
    curl -LO https://curl.se/ca/cacert.pem
    export SSL_CERT_FILE="$PWD/cacert.pem"
    ```

    In general, you don't need to set this environment variable, but, if you encounter the reporting `the SSL certificate is invalid`, trying to run above commands in your terminal will do the trick.

* **XCPKG_URL_TRANSFORM**

    ```bash
    export XCPKG_URL_TRANSFORM=/path/of/url-transform
    ```

    `/path/of/url-transform` command would be invoked as `/path/of/url-transform <URL>`

    `/path/of/url-transform` command must output a `<URL>`

    you can generate a url-transform sample via `xcpkg gen-url-transform-sample`

    If you want to change the request url, you can set this environment variable. It is very useful for chinese users.

* **XCPKG_XTRACE**

    for debugging purposes.

    this environment variable only affects POSIX-Shell-based implementation.

    enables `set -x`:

    ```bash
    export XCPKG_XTRACE=1
    ```

* **XCPKG_DEFAULT_TARGET_PLATFORM_SPEC**

    some commands need `<PACKAGE-SPEC>` to be specified. `<PACKAGE-SPEC>` has the form `<TARGET-PLATFORM-SPEC>/<PACKAGE-NAME>`, you can omit `<TARGET-PLATFORM-SPEC>/`. If `<TARGET-PLATFORM-SPEC>/` is omitted, this environment variables will be used, if this environment variables is not set, then will retrive your current os's info.

    example:

    ```bash
    export XCPKG_DEFAULT_TARGET_PLATFORM_SPEC=MacOSX/10.15/x86_64
    ```

* **other relevant environment variables**

    |utility|reference|
    |-|-|
    |[cmake](https://cmake.org/)|[reference](https://cmake.org/cmake/help/latest/manual/cmake-env-variables.7.html)|
    |[cargo](https://doc.rust-lang.org/cargo/)|[reference](https://doc.rust-lang.org/cargo/reference/environment-variables.html)|
    |[go](https://golang.org/)|[reference](https://golang.org/doc/install/source#environment)|
    |[pkg-config](https://www.freedesktop.org/wiki/Software/pkg-config/)|[reference](https://www.linuxhowtos.org/manpages/1/pkg-config.htm#lbAF)|
    |[aclocal](https://www.gnu.org/software/automake/manual/html_node/configure.html)|[reference](https://www.gnu.org/software/automake/manual/html_node/Macro-Search-Path.html)|

    example:

    ```bash
    export GOPROXY='https://goproxy.cn'
    ```

## xcpkg formula

a xcpkg formula is a [YAML](https://yaml.org/spec/1.2.2/) format file which is used to config a xcpkg package's meta-information including one sentence description, package version, installation instructions, etc.

a xcpkg formula's filename suffix must be `.yml`

a xcpkg formula'a filename prefix would be treated as the package name.

a xcpkg formula'a filename prefix must match regular expression pattern `^[A-Za-z0-9+-._]{1,50}$`

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
timestamp-created: 1673684639
timestamp-updated: 1673684767
```

If a xcpkg formula repository is `pinned`, which means it would not be updated.

If a xcpkg formula repository is `disabled`, which means xcpkg would not search formulas in this formula repository.

## xcpkg offical formula repository

xcpkg offical formula repository's url: <https://github.com/leleliu008/xcpkg-formula-repository-offical-core>

xcpkg offical formula repository would be automatically fetched to local cache as name `offical-core` when you run `xcpkg update` command.

**Note:** If you find that a package is not in xcpkg offical formula repository yet, PR is welcomed.
