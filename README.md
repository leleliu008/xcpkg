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
        
        ipkg install curl
        ipkg install curl bzip2
        ipkg install curl bzip2 --rule=xx
        ipkg install curl bzip2 -v --xtrace --verbose --keep-working-dir
        
*   reinstall packages
        
        ipkg reinstall curl
        ipkg reinstall curl bzip2 -v
        
*   uninstall packages
        
        ipkg uninstall curl
        ipkg uninstall curl bzip2
        
*   upgrade the outdated packages
        
        ipkg upgrade curl
        ipkg upgrade curl bzip2 -v
        
*   view the formula of a package

        ipkg formula view curl
        
*   edit the formula of a package

        ipkg formula edit curl
        
*   create a formula

        ipkg formula create curl
        
*   delete a formula

        ipkg formula delete curl
        
*   rename a formula to new name

        ipkg formula rename curl curl7
        
*   view a rule

        ipkg rule view curl
        
*   edit a rule

        ipkg rule edit curl
        
*   create a rule

        ipkg rule create xx
        
*   delete a formula

        ipkg rule delete xx
        
*   rename a rule to new name

        ipkg rule rename xx yy
        
*   list rules
        
        ipkg rule list
         
*   list the supported platforms
        
        ipkg list platforms
        
*   list the supported archs

        ipkg list archs
        ipkg list archs iPhoneOS
        
*   list the supported abis

        ipkg list abis
        
*   list the supported versions

        ipkg list versions
        ipkg list versions iPhoneOS
        
*   list the available packages
        
        ipkg list available
        ipkg list available -q
        
*   list the installed packages
        
        ipkg list installed
        
*   list the outdated packages
        
        ipkg list outdated
        
*   is the specified package available ?
        
        ipkg is available curl
        
*   is the specified package installed ?
        
        ipkg is installed curl
        
*   is the specified package outdated ?
        
        ipkg is outdated curl
        
*   list contents of a installed package directory in a tree-like format.
        
        ipkg tree curl
        
*   update the [formula repository](https://github.com/leleliu008/ipkg-formula)
        
        ipkg update
        
*   download formula resources of a package to the cache
        
        ipkg fetch curl
        
*   print the logs of a installed package
        
        ipkg logs curl iPhoneOS/armv7s
        ipkg logs curl iPhoneOS/arm64
        
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
        
