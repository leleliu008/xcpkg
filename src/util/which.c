#include <stdio.h>
#include <string.h>

#include "../core/exe.h"
#include "../core/log.h"

#include "../util.h"

/**
 *  xcpkg util which <COMMAND-NAME> [-a]
 */
int xcpkg_util_which(int argc, char* argv[]) {
    if (argv[3] == NULL) {
        fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> , <COMMAND-NAME> is unspecified.\n", argv[0], argv[1], argv[2]);
        return 1;
    }

    if (argv[3][0] == '\0') {
        fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> , <COMMAND-NAME> should be a non-empty string.\n", argv[0], argv[1], argv[2]);
        return 1;
    }

    bool findAll = false;

    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) {
            findAll = true;
        } else {
            LOG_ERROR2("unknown argument: ", argv[i]);
            fprintf(stderr, "USAGE: %s %s %s <COMMAND-NAME> [-a]\n", argv[0], argv[1], argv[2]);
            return 1;
        }
    }

    char ** pathList = NULL;

    int ret = exe_search(argv[3], &pathList, findAll);

    if (ret > 0) {
        for (int i = 0; i < ret; i++) {
            printf("%s\n", pathList[i]);

            free(pathList[i]);
            pathList[i] = NULL;
        }

        free(pathList);
        pathList = NULL;

        ret = 0;
    }

    return ret;
}
