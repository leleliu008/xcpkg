#include <string.h>
#include <stdbool.h>

#include "../xcpkg.h"

/**
 *  xcpkg upgrade-self
 */
int xcpkg_main_upgrade_self(int argc, char* argv[]) {
    bool verbose = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
            break;
        }
    }

    return xcpkg_upgrade_self(verbose);
}
