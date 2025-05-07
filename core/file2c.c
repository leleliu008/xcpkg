#include <stdio.h>
#include <string.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

int file2c(const char * const fp, const char * varName) {
    int fd = open(fp, O_RDONLY);

    if (fd == -1) {
        perror(fp);
        return -1;
    }

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror(fp);
        close(fd);
        return -1;
    }

    if (st.st_size == 0) {
        fprintf(stderr, "empty file: %s\n", fp);
        close(fd);
        return -1;
    }

    char * data = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        perror(fp);
        close(fd);
        return -1;
    }

    printf("size_t %s_LENGTH = %lld;\n", varName, st.st_size);
    printf("char %s[] = {", varName);
    printf("0x%.2X", data[0]);

    for (int i = 1; i < st.st_size; i++) {
        printf(", 0x%.2X", data[i]);
    }

    printf(", 0x00};\n");

    if (munmap(data, st.st_size) == -1) {
        perror("Failed to unmap file");
    }

    close(fd);

    return 0;
}

int main(int argc, char* argv[]) {
    if (argv[1] == NULL || argv[1][0] == '\0' || argv[2] == NULL || argv[2][0] == '\0') {
        fprintf(stderr, "%s <FILE> <VAR-NAME>\n", argv[0]);
        return 1;
    }

    return file2c(argv[1], argv[2]);
}
