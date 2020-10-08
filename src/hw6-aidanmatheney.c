/*
 * Aidan Matheney
 * aidan.matheney@und.edu
 *
 * CSCI 451 HW6
 */

#include "../include/hw6.h"

#include "../include/util/macro.h"

#include <stdlib.h>

int main(int const argc, char ** const argv) {
    static char const * const inFilePaths[] = {
        "hw6-1.in",
        "hw6-2.in",
        "hw6-3.in"
    };
    hw6(inFilePaths, ARRAY_LENGTH(inFilePaths), "hw6.out", HW6TestMode_ExitThread);
    return EXIT_SUCCESS;
}
