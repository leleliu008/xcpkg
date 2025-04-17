#include "core/sysinfo.h"

#include "xcpkg.h"

int xcpkg_sysinfo() {
    SysInfo sysinfo = {0};

    int ret = sysinfo_make(&sysinfo);

    if (ret == XCPKG_OK) {
        sysinfo_dump(&sysinfo);
    }

    return ret;
}
