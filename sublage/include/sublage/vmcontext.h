/*
    Virtual machine context execution.
*/
#pragma once

#include "sublage/debug.h"

/* 
 Internal function reference.
 In a StackObject struct, the functionOffset field is replaced by an 
 InternalFunctionRef struct at run-time (when binary file image is added to the 
 VM execution context, when the image index is known).
 The InternalFunctionRef pointer is stored in the privateData field
 */
typedef struct {
    uint64_t functionOffset;
    uint32_t imageIndex;
} InternalFunctionRef;

//typedef void (*VmNativeFunction) (VmContext *vc);
VmContext *vmContextCreate(FILE* stackDumpFile,
                           DebugContext *debugContext,
                           ObjectPools *globalPool,
                           uint32_t contextID);
void vmContextDestroy(VmContext *vc);
VmContext *vmContextClone(VmContext *vc);

void vmContextSetError(VmContext *vc, VmErrorCode ec, ...);
VmErrorCode vmContextGetError(VmContext *vc);
const char* vmContextGetErrorMessage(VmContext *vc);

Stack* vmContextGetStack(VmContext *vc);
void vmContextPush(VmContext *vc, StackObject *so);
StackObject* vmContextPop(VmContext *vc);
StackObject* vmContextPopOpcode(VmContext *vc, uint8_t opcode);

char *vmContextGetString(VmContext *vc, const StackObject *so);
LinkedList *vmContextGetArray(VmContext *vc, const StackObject *so);

void vmContextSetVariable(VmContext *vc, const StackObject *so);
void vmContextGetVariable(VmContext *vc, const StackObject *so);

void vmContextPushCall(VmContext *vc);
bool vmContextPopCall(VmContext *vc);
void vmContextCallFunction(VmContext *vc, StackObject *so);

uint32_t vmContextAddImage(VmContext *vc, BinExecImg *img);
BinExecImg * vmContextGetCurrentImage(VmContext *vc);
void vmContextSetCurrentImage(VmContext *vc, uint32_t imageIndex);

void vmContextRun(VmContext *vc, uint64_t startOffset, StackObject *so);
bool vmContextIsRunning(VmContext *vc);
uint32_t vmContextGetID(VmContext *vc);
void vmContextSetRunning(VmContext *vc, bool running);

StackObject* stackObjectClone(VmContext *vc, const StackObject *so);
StackObject* stackObjectNewObject(VmContext *vc, ClassPointer *cp);
StackObject* stackObjectSuper(VmContext *vc, ClassInstance *object);
StackObject* stackObjectNewString(VmContext *vc, const char *str);
StackObject* stackObjectNewEmptyArray(VmContext *vc);
StackObject* stackObjectNewArray(VmContext *vc, LinkedList *array);
StackObject* stackObjectNewInt(VmContext *vc, int64_t i);
StackObject* stackObjectNewFloat(VmContext *vc, double64_t f);
StackObject* stackObjectNewBool(VmContext *vc, bool b);
StackObject* stackObjectNewString(VmContext *vc, const char *str);
StackObject* stackObjectNewPrivate(VmContext *vc, void* data);
StackObject* stackObjectNewData(VmContext *vc, void* data, uint64_t nbytes);
StackObject* stackObjectNewNull(VmContext *vc);

