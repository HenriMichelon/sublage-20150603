#pragma once

#include <stdio.h>

#include "sublage/thread.h"
#include "sublage/mem.h"
#include "sublage/stack.h"
#include "sublage/stackobject.h"
#include "sublage/binexecimg.h"
#include "sublage/vmerrors.h"

#define SUBLAGE_REVISION 0

typedef struct {
    bool        debugging;
    bool        running;
    bool        verbose;
    FILE*       debugFileIn;
    FILE*       debugFileOut;
    LinkedList  *breakpoints;
    LinkedList  *contexts;      // All VmContext except #0
    Stack       *pausedContexts;
    ThreadThread thread;
    ThreadMutex  mutex;
} DebugContext;

typedef struct {
    LinkedList  *strings;
    LinkedList  *arrays;
    LinkedList  *objects; // classes instances
    ThreadMutex mutex;
} ObjectPools;

typedef struct {
    uint32_t    contextID;
    Stack       *stack;         // Execution stack (full of StackObject)
    Stack       *callStack;     // Function call return offsets stack.
    ObjectPools *globalPool;    // Global stack object pool, one for all contexts
    Stack       *garbage;       // Stack objets popped from execution stacks during a function call.
    VmErrorCode error;          // Last execution error code
    char        errorMessage[300];
    BinExecImg  **images;       // Array of binary file in-memory images
    uint32_t    imagesCount;
    uint32_t    imagesIndex;
    StackObject *self;
    bool        isRunning;
    bool        isClone;
    FILE*       stackDumpFile;
    DebugContext *debugContext;
    bool        isPaused;       // Only used when debugContext != NULL
    bool        exitFromPause;
    bool        stepInto;       // Only used when debugContext != NULL
    bool        stepOver;       // Only used when debugContext != NULL
    bool        stepOut;        // Only used when debugContext != NULL
    uint32_t    inStepOver;     // Only used when debugContext != NULL
} VmContext;
