#include <fcntl.h>
#include <unistd.h>

#include "xcpkg.h"

int xcpkg_read_the_first_n_bytes_of_a_file(const char * fp, unsigned int n, char buf[]) {
    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return XCPKG_ERROR;
    }

    ssize_t readSize = read(fd, buf, n);

    if (readSize == -1) {
        perror(fp);
        close(fd);
        return XCPKG_ERROR;
    } else {
        close(fd);
        return XCPKG_OK;
    }
}
