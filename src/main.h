#ifndef XCPKG_MAIN_H
#define XCPKG_MAIN_H

#define DECLARE_MAIN(name) int xcpkg_main_##name(int argc, char* argv[]);

DECLARE_MAIN(about)
DECLARE_MAIN(xcinfo)
DECLARE_MAIN(cleanup)
DECLARE_MAIN(completion)
DECLARE_MAIN(util)

DECLARE_MAIN(upgrade_self)
DECLARE_MAIN(update)

DECLARE_MAIN(search)
DECLARE_MAIN(depends)
DECLARE_MAIN(info_available)
DECLARE_MAIN(info_installed)
DECLARE_MAIN(fetch)

DECLARE_MAIN(install)
DECLARE_MAIN(reinstall)
DECLARE_MAIN(uninstall)
DECLARE_MAIN(upgrade)

DECLARE_MAIN(tree)
DECLARE_MAIN(logs)
DECLARE_MAIN(bundle)

DECLARE_MAIN(ls_available)
DECLARE_MAIN(ls_installed)
DECLARE_MAIN(ls_outdated)

DECLARE_MAIN(is_available)
DECLARE_MAIN(is_installed)
DECLARE_MAIN(is_outdated)

DECLARE_MAIN(formula_cat)
DECLARE_MAIN(formula_bat)
DECLARE_MAIN(formula_set)
DECLARE_MAIN(formula_edit)
DECLARE_MAIN(formula_parse)

DECLARE_MAIN(formula_repo_list)
DECLARE_MAIN(formula_repo_init)
DECLARE_MAIN(formula_repo_add)
DECLARE_MAIN(formula_repo_del)
DECLARE_MAIN(formula_repo_sync)
DECLARE_MAIN(formula_repo_conf)
DECLARE_MAIN(formula_repo_info)

#endif
