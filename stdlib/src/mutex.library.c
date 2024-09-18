#include "sublage/vmcontext.h"
#include "sublage/thread.h"

 void native_create(VmContext *vc) {
    ThreadMutex *mutex = memAlloc(sizeof(ThreadMutex));
    ThreadMutexCreate(mutex);
    vmContextPush(vc, stackObjectNewPrivate(vc, mutex));
}

 void native_destroy(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (so == NULL) return;
    ThreadMutexDestroy((ThreadMutex*)so->data.privateData);
    memFree(so->data.privateData);
}

 void native_lock(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (so == NULL) return;
    ThreadMutexLock((ThreadMutex*)so->data.privateData);
}

 void native_unlock(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_PRIVATE);
    if (so == NULL) return;
    ThreadMutexUnlock((ThreadMutex*)so->data.privateData);
}
