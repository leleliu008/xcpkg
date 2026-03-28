#include<stdio.h>
#include<stdlib.h>

#include<unistd.h>
#include<limits.h>

int list_PATH() {
    const char * p = getenv("PATH");

    if (p == NULL) {
        return 0;
    }

    char q[PATH_MAX];

loop:
    for (size_t i = 0U; i < PATH_MAX; i++) {
        q[i] = p[i];

        if (q[i] == '\0') {
            if (i != 0U) {
                puts(q);
            }

            return 0;
        }

        if (q[i] == ':') {
            q[i] = '\0';

            if (i != 0U) {
                puts(q);
            }

            p += i + 1;

            goto loop;
        }
    }

    fprintf(stderr, "PATH item too long, longer than %d\n", PATH_MAX);
    return 1;
}
