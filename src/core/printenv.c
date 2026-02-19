/* 
 * https://www.gnu.org/software/gnulib/manual/html_node/environ.html
 */

#include <stdio.h>

#include <crt_externs.h>

#include "printenv.h"

void printenv() {
    char **envlist = *_NSGetEnviron();

    for (int i = 0; envlist[i] != NULL; i++) {
        puts(envlist[i]);
    }
}
