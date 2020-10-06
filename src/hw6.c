#include "../include/hw6.h"

#include "../include/util/memory.h"
#include "../include/util/thread.h"
#include "../include/util/file.h"
#include "../include/util/guard.h"

#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdio.h>
#include <assert.h>

struct ReadFileCharactersThreadStartArg {
    char const *inFilePath;
    bool finished;
    char *characterOutPtr;

    pthread_mutex_t syncMutex;
    pthread_cond_t readCondition;
    pthread_cond_t wroteCondition;
};

static void *readFileCharactersThreadStart(void * const argAsVoidPtr);

/**
 * Run CSCI 451 HW6. This reads characters one at a time from each input file, printing the character to the output file
 * and cycling to the next file after each character is read. A separate thread is dedicated for reading each file,
 * while the writing occurs on the calling thread.
 *
 * @param inFilePaths The input file paths.
 * @param inFileCount The number of input files.
 * @param outFilePath The output file path.
 */
void hw6(
    char const * const * const inFilePaths,
    size_t const inFileCount,
    char const * const outFilePath
) {
    guardNotNull(inFilePaths, "inFilePaths", "hw6");
    guardNotNull(outFilePath, "outFilePath", "hw6");

    FILE * const outFile = safeFopen(outFilePath, "w", "hw6");

    char readCharacter;

    struct ReadFileCharactersThreadStartArg * const threadStartArgs = (
        safeMalloc(sizeof *threadStartArgs * inFileCount, "hw6")
    );
    pthread_t * const threadIds = safeMalloc(sizeof *threadIds * inFileCount, "hw6");
    for (size_t i = 0; i < inFileCount; i += 1) {
        char const * const inFilePath = inFilePaths[i];
        struct ReadFileCharactersThreadStartArg * const threadStartArgPtr = &threadStartArgs[i];

        threadStartArgPtr->inFilePath = inFilePath;
        threadStartArgPtr->finished = false;
        threadStartArgPtr->characterOutPtr = &readCharacter;

        safeMutexInit(&threadStartArgPtr->syncMutex, NULL, "hw6");
        safeConditionInit(&threadStartArgPtr->readCondition, NULL, "hw6");
        safeConditionInit(&threadStartArgPtr->wroteCondition, NULL, "hw6");

        // Lock this file's mutex before its thread launches to ensure the thread can wait on the read condition before
        // the main thread locks the mutex and signals the read condition
        safeMutexLock(&threadStartArgPtr->syncMutex, "hw6");

        threadIds[i] = safePthreadCreate(
            NULL,
            readFileCharactersThreadStart,
            threadStartArgPtr,
            "hw6"
        );
    }

    while (true) {
        bool foundUnfinished = false;

        for (size_t i = 0; i < inFileCount; i += 1) {
            struct ReadFileCharactersThreadStartArg * const threadStartArgPtr = &threadStartArgs[i];

            if (threadStartArgPtr->finished) {
                continue;
            }

            safeMutexLock(&threadStartArgPtr->syncMutex, "hw6");
            safeConditionSignal(&threadStartArgPtr->readCondition, "hw6");
            safeConditionWait(&threadStartArgPtr->wroteCondition, &threadStartArgPtr->syncMutex, "hw6");
            safeMutexUnlock(&threadStartArgPtr->syncMutex, "hw6");

            if (threadStartArgPtr->finished) {
                continue;
            }
            foundUnfinished = true;

            safeFprintf(outFile, "hw6", "%c\n", readCharacter);
        }

        if (!foundUnfinished) {
            break;
        }
    }

    for (size_t i = 0; i < inFileCount; i += 1) {
        struct ReadFileCharactersThreadStartArg * const threadStartArgPtr = &threadStartArgs[i];
        pthread_t const threadId = threadIds[i];

        safePthreadJoin(threadId, "hw6");
        safeMutexDestroy(&threadStartArgPtr->syncMutex, "hw6");
        safeConditionDestroy(&threadStartArgPtr->readCondition, "hw6");
        safeConditionDestroy(&threadStartArgPtr->wroteCondition, "hw6");
    }

    free(threadStartArgs);
    free(threadIds);

    fclose(outFile);
}

static void *readFileCharactersThreadStart(void * const argAsVoidPtr) {
    assert(argAsVoidPtr != NULL);
    struct ReadFileCharactersThreadStartArg * const argPtr = argAsVoidPtr;

    FILE * const inFile = safeFopen(argPtr->inFilePath, "r", "readFileCharactersThreadStart");

    // syncMutex is already locked
    while (true) {
        safeConditionWait(&argPtr->readCondition, &argPtr->syncMutex, "readFileCharactersThreadStart");

        bool const scanned = scanFileExact(inFile, 1, "%c\n", argPtr->characterOutPtr);
        if (!scanned) {
            argPtr->finished = true;
        }

        safeConditionSignal(&argPtr->wroteCondition, "readFileCharactersThreadStart");

        if (!scanned) {
            break;
        }
    }
    safeMutexUnlock(&argPtr->syncMutex, "readFileCharactersThreadStart");

    fclose(inFile);

    return NULL;
}
