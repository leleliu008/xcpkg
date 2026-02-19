#include "xcpkg.h"

int xcpkg_formula_view(const char * packageName, const char * targetPlatformName, const bool raw) {
    if (raw) {
        return xcpkg_formula_cat(packageName, targetPlatformName);
    } else {
        return xcpkg_formula_bat(packageName, targetPlatformName, 0, NULL);
    }
}
