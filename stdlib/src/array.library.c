#include "sublage/vmcontext.h"

 void native_get(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_INT);
    if (so == NULL) return;
    uint64_t index = so->data.intValue;
    so = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (so == NULL) return;
    LinkedList *array = vmContextGetArray(vc, so);
    if (index >= linkedListSize(array)) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    vmContextPush(vc, stackObjectClone(vc, linkedListGet(array, index)));
}

 void native_set(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_INT);
    if (so == NULL) return;
    StackObject *data = vmContextPop(vc);
    if (data == NULL) return;
    uint64_t index = so->data.intValue;
    so = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (so == NULL) return;
    LinkedList *array = vmContextGetArray(vc, so);
    linkedListSet(array, stackObjectClone(vc, data), index);
    vmContextPush(vc, stackObjectClone(vc, so));
}

 void native_count(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (so == NULL) return;
    LinkedList *array = vmContextGetArray(vc, so);
    vmContextPush(vc, stackObjectNewInt(vc, linkedListSize(array)));
}

 void native_remove_first(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (so == NULL) return;
    LinkedList *array = vmContextGetArray(vc, so);
    StackObject *elem = linkedListRemoveFirst(array);
    vmContextPush(vc, stackObjectClone(vc, elem));
}

 void native_remove_last(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (so == NULL) return;
    LinkedList *array = vmContextGetArray(vc, so);
    StackObject *elem = linkedListRemoveLast(array);
    vmContextPush(vc, stackObjectClone(vc, elem));
}

 void native_clone(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (so == NULL) return;
    LinkedList *array = vmContextGetArray(vc, so);
    vmContextPush(vc, stackObjectClone(vc, so));
    vmContextPush(vc, stackObjectNewArray(vc, array));
}

 void native_insert(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_INT);
    if (so == NULL) return;
    StackObject *data = vmContextPop(vc);
    if (data == NULL) return;
    uint64_t index = so->data.intValue;
    so = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (so == NULL) return;
    LinkedList *array = vmContextGetArray(vc, so);
    linkedListInsert(array, index, stackObjectClone(vc, data));
    vmContextPush(vc, stackObjectClone(vc, so));
}


 void native_explode(VmContext *vc) {
    StackObject *oldarray = vmContextPop(vc);
    if (oldarray == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (oldarray->opcode != OPCODE_ARRAY) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    LinkedList *array = vmContextGetArray(vc, oldarray);
    LinkedListIterator *it = linkedListCreateIterator(array);
    StackObject *so;
    while ((so = linkedListIteratorNext(it)) != NULL) {
        vmContextPush(vc, stackObjectClone(vc, so));
    }
    vmContextPush(vc, stackObjectNewInt(vc, linkedListSize(array)));
}

