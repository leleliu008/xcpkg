#include "core/log.h"

#include "xcpkg.h"

int xcpkg_cleanup(const bool verbose) {
    if (verbose) {
        LOG_SUCCESS1("Done.");
    }

    return XCPKG_OK;
}
