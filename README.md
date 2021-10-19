# ipkg
a package manager for [Xcode](https://developer.apple.com/xcode) to build C/C++/Rust project.

## Install ipkg via HomeBrew

```bash
brew tap leleliu008/fpliu
brew install ipkg
```

## Install ipkg via cURL
```bash
curl -LO https://raw.githubusercontent.com/leleliu008/ipkg/master/bin/ipkg
chmod a+x ipkg
mv ipkg /usr/local/bin/

# following instrutions is optional, and these instructions only worked in zsh
ipkg integrate zsh
autoload -U compinit && compinit
```

## ipkg command usage
*   show the help of this command.
        
        ipkg -h
        ipkg --help
        
*   show the version of this command
        
        ipkg -V
        ipkg --version

*   show home directory this software

        ndk-pkg --homedir

*   show home webpage this software

        ndk-pkg --homepage

*   show current machine os and [Xcode](https://developer.apple.com/xcode) toolchain info
        
        ipkg env

*   integrate `zsh-completion` script
        
        ipkg integrate zsh
        ipkg integrate zsh -x
        ipkg integrate zsh --china
        ipkg integrate zsh --china -x
        
    I have provide a zsh-completion script for `ipkg`. when you've typed `ipkg` then type `TAB` key, it will auto complete the rest for you.

    **Note**: to apply this feature, you may need to run the command `autoload -U compinit && compinit`
*   update the formula repositories

        ipkg update
        
    **Note:** this software supports multi formula repositories. Offical formula repository is [ipkg-formula-repository](https://github.com/leleliu008/ipkg-formula-repository) 

*   search packages can be installed
        
        ipkg search curl
        ipkg search lib
        
*   show infomation of the given package or all available packages
        
        ipkg info curl
        ipkg info curl version
        ipkg info curl summary
        ipkg info curl webpage
        ipkg info curl src.git
        ipkg info curl installed-dir
        ipkg info curl installed-metadata
        ipkg info curl installed-files
        ipkg info curl installed-abis
        ipkg info curl installed-datetime-unix
        ipkg info curl installed-datetime-formatted
        ipkg info curl installed-pkg-version
        ipkg info curl --json
        ipkg info curl --json | jq .
        ipkg info @all
        ipkg info @all --json
        ipkg info @all --json | jq .
         
    For more keys please read [README.md](https://github.com/leleliu008/ipkg-formula-repository/blob/master/README.md#the-function-must-be-invoked-on-top-of-the-formula)

*   install packages
        
        ipkg install curl
        ipkg install curl bzip2 --rule=xx
        ipkg install curl bzip2 --rule=xx --jobs=4
        ipkg install curl bzip2 --rule=xx --jobs=4 -v
        ipkg install curl bzip2 --rule=xx --jobs=4 -v -d
        ipkg install curl bzip2 --rule=xx --jobs=4 -v -d -x
        ipkg install curl bzip2 --rule=xx --jobs=4 -v -d -x --dry-run
        ipkg install curl bzip2 --rule=xx --jobs=4 -v -d -x --dry-run --keep-work-dir
        
*   reinstall packages
        
        ipkg reinstall curl
        ipkg reinstall curl bzip2 -v
        
*   uninstall packages
        
        ipkg uninstall curl
        ipkg uninstall curl bzip2
        
*   upgrade the outdated packages
        
        ipkg upgrade
        ipkg upgrade curl
        ipkg upgrade curl bzip2 -v
        
*   upgrade this software

        ipkg upgrade @self
        ipkg upgrade @self -x
        ipkg upgrade @self --china
        ipkg upgrade @self --china -x
        
*   list the avaliable formula repos
        
        ipkg formula-repo list
        
*   add a new formula repo
        
        ipkg formula-repo add my_repo https://github.com/leleliu008/ipkg-formula-repository.git
        
*   delete a existing formula repo
        
        ipkg formula-repo del my_repo
        
*   view the formula of the given package

        ipkg formula view curl
        
*   edit the formula of the given package

        ipkg formula edit curl
        
*   create the formula of the given package

        ipkg formula create curl
        
*   delete the formula of the given package

        ipkg formula delete curl
        
*   rename the formula of the given package to new name

        ipkg formula rename curl curl7
        
*   view the given rule

        ipkg rule view curl
        
*   edit the given rule

        ipkg rule edit curl
        
*   create a new rule

        ipkg rule create xx
        
*   delete the given rule

        ipkg rule delete xx
        
*   rename the given rule to new name

        ipkg rule rename xx yy
        
*   list rules
        
        ipkg rule list
        
*   list the supported target platforms
        
        ipkg target platforms
        
*   list the supported target platform's versions

        ipkg target versions
        ipkg target versions iPhoneOS
        
*   list the supported target archs

        ipkg target archs
        ipkg target archs iPhoneOS
        ipkg target archs iPhoneOS 64bit
        ipkg target archs iPhoneOS 32bit
        ipkg target archs iPhoneOS all
        
*   list the supported target abis

        ipkg target abis
        ipkg target abis 64bit
        ipkg target abis 32bit
        ipkg target abis all

*   list the supported target triples

        ipkg target triples
        ipkg target triples 64bit
        ipkg target triples 32bit
        ipkg target triples all
        
*   list the available packages
        
        ipkg ls-available
        
*   list the installed packages
        
        ipkg ls-installed
        
*   list the outdated packages
        
        ipkg ls-outdated
        
*   is the given package available ?
        
        ipkg is-available curl
        ipkg is-available curl ge 7.50.0
        ipkg is-available curl gt 7.50.0
        ipkg is-available curl le 7.50.0
        ipkg is-available curl lt 7.50.0
        ipkg is-available curl eq 7.50.0
        ipkg is-available curl ne 7.50.0
        
*   is the given package installed ?
        
        ipkg is-installed curl
        
*   is the given package outdated ?
        
        ipkg is-outdated curl
        
*   list files of the given installed package in a tree-like format.
        
        ipkg tree curl
        
*   download formula resources of the given package to the cache
        
        ipkg fetch curl
        
*   show logs of the given installed package
        
        ipkg logs curl iPhoneOS/armv7s
        ipkg logs curl iPhoneOS/arm64
        
*   pack the given installed package
        
        ipkg pack curl
        
*   show or open the homepage of the given package or this project
        
        ipkg homepage
        ipkg homepage --open
        ipkg homepage --open curl
        ipkg homepage curl --open
        
*   show the depended packages of the given package
        
        ipkg depends curl
        
*   cleanup the unused cache
        
        ipkg cleanup
        
