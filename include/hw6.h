#pragma once

#include <stdlib.h>

enum HW6TestMode {
    HW6TestMode_None,
    HW6TestMode_ExitThread,
    HW6TestMode_KillThreadWithPthreadSelf,
    HW6TestMode_KillThreadWithGettid
};

void hw6(
    char const * const *inFilePaths,
    size_t inFileCount,
    char const *outFilePath,
    enum HW6TestMode testMode
);
