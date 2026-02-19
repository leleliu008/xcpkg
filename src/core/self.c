#include <stdlib.h>

// https://developer.apple.com/library/archive/documentation/System/Conceptual/ManPages_iPhoneOS/man3/dyld.3.html
#include <mach-o/dyld.h>

#include "self.h"

int selfpath(char buf[]) {
    uint32_t bufSize = 0U;
    _NSGetExecutablePath(NULL, &bufSize);

    _NSGetExecutablePath(buf, &bufSize);

    return 0;
}
