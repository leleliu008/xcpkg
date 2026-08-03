#include <string.h>
#include <stdbool.h>

#include "../xcpkg.h"

/**
 *  xcpkg setup
 */
int xcpkg_main_setup(int argc, char* argv[]) {
    bool verbose = false;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0) {
            verbose = true;
            break;
        }
    }

    return xcpkg_setup(verbose);
}
