/*
    Binary file in-memory image
*/
 #pragma once

#include <stdio.h>
#include "sublage/binexec.h"
#include "sublage/stackobject.h"

typedef struct {
    uint32_t imageIndex;
    uint64_t offset;
} ExternalFunction;

typedef void (*VmNativeFunction) (void*);

typedef struct {
    int32_t imageIndex;
    VmNativeFunction function;
} NativeFunction;

typedef StackObject* Variable;

typedef struct {
    void* id;
    bool issuper;
    ClassPointer *cp;
    uint64_t nivars;
    Variable  *ivars;
} ClassInstance;

typedef struct {
    BinExecFile *bef;           // Binary file in memory.
    char*       importName;     // stripped binary file name
    uint64_t    codeIndex;      // Currently executed StackObject index in ‘codeStack‘
    uint64_t    lastCodeIndex;  // Previously executed StackObject index before a jump or a function call
    uint32_t    externalsCount;
    uint32_t    nativeCount;
    ExternalFunction **externals;
    NativeFunction **natives;
    Variable    *variables;
} BinExecImg;

BinExecImg* binexecimgCreate(BinExecFile *bef);
void binexecimgDestroy(BinExecImg *bei);
BinExecImg* binexecimgClone(BinExecImg *bei);

char* binexecimgGetString(BinExecImg *bei, uint32_t stringIndex);
LinkedList* binexecimgGetArray(BinExecImg *bei, uint32_t arrayIndex);

StackObject* binexecimgNextStackObject(BinExecImg *bef);
StackObject* binexecimgCurrentStackObject(BinExecImg *bef);

void binexecimgJumpToOffset(BinExecImg *bef, uint64_t offset);
void binexecimgRelativeJumpToOffset(BinExecImg *bef, int64_t offset);

static inline uint64_t binexecimgGetCurrentIndex(BinExecImg *bef) {
    return bef->codeIndex;
}
