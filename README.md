# ipkg
a package manager for [Xcode](https://developer.apple.com/xcode) to build C/C++ project.

## Installation

via [HomeBrew](http://blog.fpliu.com/it/os/macOS/software/HomeBrew)

```
brew tap leleliu008/fpliu
brew install ipkg
```

## zsh-completion for ipkg
I have provide a zsh-completion script for `ipkg`. when you've typed `ipkg` then type `TAB` key, it will auto complete the rest for you.

**Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`

## ipkg command usage
*   print the help infomation of `ipkg` command
        
        ipkg -h
        ipkg --help
        ipkg help
        
*   print the version of `ipkg` and `Xcode`
        
        ipkg -V
        ipkg --version
        ipkg version
        
*   search packages can be installed
        
        ipkg search curl
        ipkg search lib
        
*   print the basic infomation of packages
        
        ipkg info curl
        ipkg info curl openssl
        
*   install packages
        
        ipkg install phone curl
        ipkg install phone curl bzip2 --min-version=8.0 --archs=armv7,armv7s,arm64,arm64e --simulator -v -x
        ipkg install watch curl bzip2 --min-version=5.0 --archs=armv7k,arm64_32 --simulator -v -x
        ipkg install tv    curl bzip2 --min-version=9.0 --simulator -v -x
        ipkg install mac   curl bzip2 --min-version=10.13
        
*   reinstall packages
        
        ipkg reinstall curl
        ipkg reinstall curl bzip2 -v
        
*   uninstall packages
        
        ipkg uninstall curl
        ipkg uninstall curl bzip2
        
*   upgrade the outdated packages
        
        ipkg upgrade curl
        ipkg upgrade curl bzip2 -v

*   view the formula source code of a package
        
        ipkg cat curl
        
*   edit the formula source code of a package
        
        ipkg edit curl
        
*   list the supported target devices
        
        ipkg list devices
        
*   list the supported target platforms
        
        ipkg list platforms
        
*   list the available packages
        
        ipkg list available
        ipkg list available -q
        
*   list the installed packages
        
        ipkg list installed
        ipkg list installed -q
        
*   list the outdated packages
        
        ipkg list outdated
        ipkg list outdated -q
        
*   is the specified package available ?
        
        ipkg is available curl
        
*   is the specified package installed ?
        
        ipkg is installed curl
        
*   is the specified package outdated ?
        
        ipkg is outdated curl
        
*   list contents of a installed package directory in a tree-like format.
        
        ipkg tree curl
        
*   print the environment variable settings
        
        ipkg env curl --device=phone --min-version=8.0   --arch=armv7s
        ipkg env curl --device=watch --min-version=6.2   --arch=armv7k
        ipkg env curl --device=tv    --min-version=8.0   --arch=arm64
        ipkg env curl --device=mac   --min-version=10.14 --arch=x86_64

*   update the [formula repository](https://github.com/leleliu008/ipkg-formula)
        
        ipkg update
        
*   download formula resources of a package to the cache
        
        ipkg fetch curl
        
*   print the logs of a installed package
        
        ipkg logs curl iPhoneOS armv7s
        ipkg logs curl iPhoneOS arm64
        
*   pack a installed package
        
        ipkg pack curl
        
*   visit the homepage of a formula or the `ipkg` project
        
        ipkg homepage
        ipkg homepage curl
        
*   show the installation direcotory of a formula or the `ipkg` home
        
        ipkg prefix
        ipkg prefix curl
        
*   cleanup the unused cache
        
        ipkg cleanup
        
