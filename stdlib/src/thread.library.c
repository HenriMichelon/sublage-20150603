#include "sublage/linkedlist.h"
#include "sublage/vmcontext.h"
#include "sublage/thread.h"
#include <stdlib.h>
#include <time.h>

typedef struct {
    VmContext*   vc;
    ThreadThread thread;
    uint64_t     functionOffset;
    StackObject* data;
} vmThread_t;

static LinkedList *threads = NULL;
static ThreadMutex threads_mutex;

void cleanUpThreads() {
    if (threads != NULL) {
        ThreadMutexLock(&threads_mutex);
        linkedListDestroy(threads, true);
        ThreadMutexDestroy(&threads_mutex);

    }
}

ThreadRoutineReturnType __run(void* param) {
    vmThread_t *thread = (vmThread_t *)param;
    vmContextRun(thread->vc, thread->functionOffset, thread->data);
    memFree(thread->data);
    return ThreadRoutineReturn;
}

 void native_join(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (so == NULL) return;
    vmThread_t *thread = (vmThread_t *)so->data.privateData;
    ThreadJoin(thread->thread);
    ThreadMutexLock(&threads_mutex);
    linkedListRemove(threads, thread);
    ThreadMutexUnlock(&threads_mutex);
    memFree(thread);
}

 void native_start(VmContext *vc) {
    StackObject *func = vmContextPopOpcode(vc, OPCODE_INTERNALCALL_REF);
    if (func == NULL) return;
    StackObject *data = vmContextPop(vc);
    if (data == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    } 

    vmThread_t *thread = memAlloc(sizeof(vmThread_t));
    InternalFunctionRef *ref = (InternalFunctionRef*)func->data.privateData;
    thread->functionOffset = ref->functionOffset;
    thread->vc = vc;
    vmContextPush(vc, stackObjectNewPrivate(vc, thread));
    if (threads == NULL) {
        ThreadMutexCreate(&threads_mutex);
        ThreadMutexLock(&threads_mutex);
        threads = linkedListCreate();
        atexit(cleanUpThreads);
    } else {
        ThreadMutexLock(&threads_mutex);
    }
    linkedListAppend(threads, thread);
    ThreadMutexUnlock(&threads_mutex);
    thread->data = stackObjectClone(vc, data);
    ThreadCreate(&thread->thread, __run, (void*)thread);
}

 void native_sleepm(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_INT);
    if (so == NULL) return;
    
#ifdef WIN32
    Sleep(so->data.intValue);
#else
    struct timespec ts = { 0, 1000000 * so->data.intValue };
    nanosleep(&ts, NULL);
#endif
}

 void native_sleepn(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_INT);
    if (so == NULL) return;
#ifndef WIN32
    struct timespec ts = { 0, so->data.intValue };
    nanosleep(&ts, NULL);    
#endif
}

 void native_sleeps(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_INT);
    if (so == NULL) return;
#ifndef WIN32
    struct timespec ts = { so->data.intValue, 0 };
    nanosleep(&ts, NULL);
#endif
}

