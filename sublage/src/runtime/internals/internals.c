#include <stdlib.h>
#include <stdio.h>
#include "internals.h"
#include "internals_tables.h"
#include <string.h>

void code1Operand(VmContext *vc, Opcode1OperandTable table) {
    StackObject *operand = vmContextPop(vc);
    if (operand == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (operand->opcode >= OPCODE_TYPECOUNT) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    Internal1OperandCode f = table[operand->opcode];
    if (f == NULL) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    (*f)(vc, operand);
}

void code2Operands(VmContext *vc, Opcode2OperandsTable table) {
    StackObject *operand1 = vmContextPop(vc);
    StackObject *operand2 = vmContextPop(vc);
    if ((operand1 == NULL) || (operand2 == NULL)) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if ((operand1->opcode >= OPCODE_TYPECOUNT) ||
            (operand2->opcode >= OPCODE_TYPECOUNT)) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    Internal2OperandsCode f = table[operand1->opcode][operand2->opcode];
    if (f == NULL) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    (*f)(vc, operand1, operand2);
}

void codeNop(VmContext *vc) {
}

void codeAdd(VmContext *vc) {
    code2Operands(vc, internalAddTable);
}

void codeSub(VmContext *vc) {
    code2Operands(vc, internalSubTable);
}

void codeMul(VmContext *vc) {
    code2Operands(vc, internalMulTable);
}

void codeDiv(VmContext *vc) {
    code2Operands(vc, internalDivTable);
}

void codeMod(VmContext *vc) {
    code2Operands(vc, internalModTable);
}

void codeEq(VmContext *vc) {
    code2Operands(vc, internalEqTable);
}

void codeNeq(VmContext *vc) {
    code2Operands(vc, internalNeqTable);
}

void codeAnd(VmContext *vc) {
    code2Operands(vc, internalAndTable);
}

void codeOr(VmContext *vc) {
    code2Operands(vc, internalOrTable);
}

void codeGt(VmContext *vc) {
    code2Operands(vc, internalGtTable);
}

void codeLt(VmContext *vc) {
    code2Operands(vc, internalLtTable);
}

void codeGe(VmContext *vc) {
    code2Operands(vc, internalGeTable);
}

void codeLe(VmContext *vc) {
    code2Operands(vc, internalLeTable);
}

void codeNot(VmContext *vc) {
    code1Operand(vc, internalNotTable);
}

void codeSwap(VmContext *vc) {
    Stack *s = vmContextGetStack(vc);
    if (linkedListSize(s) < 2) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    StackObject *last = linkedListGetLast(s);
    uint64_t prev = linkedListSize(s) - 2;
    linkedListSet(s, linkedListGet(s, prev), prev + 1);
    linkedListSet(s, last, prev);
}

void codeDrop(VmContext *vc) {
    StackObject *operand1 = vmContextPop(vc);
    if (operand1 == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
    }
}

void dropn(VmContext *vc, int64_t n) {
    Stack *s = vmContextGetStack(vc);
    if ((n > linkedListSize(s)) || (n < 0)) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    for (int64_t i = 0; i < n; i++) {
        vmContextPop(vc);
    }
}

void codeDropn(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (n->opcode != OPCODE_INT) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    dropn(vc, n->data.intValue);
}

void codeDrop2(VmContext *vc) {
    dropn(vc, 2);
}

void codeDrop3(VmContext *vc) {
    dropn(vc, 3);
}

void codeClear(VmContext *vc) {
    dropn(vc, linkedListSize(vmContextGetStack(vc)));
}

void codeDepth(VmContext *vc) {
    vmContextPush(vc, stackObjectNewInt(vc, linkedListSize(vmContextGetStack(vc))));
}

void codeDup(VmContext *vc) {
    StackObject *operand1 = linkedListGetLast(vmContextGetStack(vc));
    if (operand1 == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    vmContextPush(vc, stackObjectClone(vc, operand1));
}

void codeExec(VmContext *vc) {
    StackObject *ref = vmContextPop(vc);
    if (ref == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    vmContextCallFunction(vc, ref);
}

void codeRoll(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (n->opcode != OPCODE_INT) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    Stack *s = vmContextGetStack(vc);
    if (n->data.intValue > linkedListSize(s)) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    vmContextPush(vc, linkedListRemoveAt(s, linkedListSize(s) - n->data.intValue));
}

void codeRoll3(VmContext *vc) {
    Stack *s = vmContextGetStack(vc);
    if (linkedListSize(s) < 3) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    vmContextPush(vc, linkedListRemoveAt(s, linkedListSize(s) - 3));
}

void codeRolld(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (n->opcode != OPCODE_INT) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    if (n->data.intValue == 0) {
        return;
    }
    Stack *s = vmContextGetStack(vc);
    int64_t pos = linkedListSize(s) - n->data.intValue;
    if ((pos < 0) || (n->data.intValue < 0)) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    linkedListInsert(s, pos, linkedListRemoveLast(s));
}

void codeRolld3(VmContext *vc) {
    Stack *s = vmContextGetStack(vc);
    if (linkedListSize(s) < 3) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    linkedListInsert(s, linkedListSize(s) - 3, linkedListRemoveLast(s));
}

void codeUnpick(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (n->opcode != OPCODE_INT) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    StackObject *obj = vmContextPop(vc);
    if (obj == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (n->data.intValue == 0) {
        return;
    }
    Stack *s = vmContextGetStack(vc);
    if (linkedListSize(s) < n->data.intValue) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    uint64_t pos = linkedListSize(s) - n->data.intValue;
    linkedListRemoveAt(s, pos);
    linkedListInsert(s, pos, stackObjectClone(vc, obj));
}

void pick(VmContext *vc, uint64_t n) {
    Stack *s = vmContextGetStack(vc);
    if (linkedListSize(s) < n) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    vmContextPush(vc, stackObjectClone(vc, linkedListGet(s, linkedListSize(s) - n)));
}

void codePick(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (n->opcode != OPCODE_INT) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    if (n->data.intValue == 0) {
        return;
    }
    pick(vc, n->data.intValue);
}

void codePick3(VmContext *vc) {
    pick(vc, 3);
}

void codePick2Over(VmContext *vc) {
    pick(vc, 2);
}

void dupn(VmContext *vc, int64_t n) {
    if (n < 0) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    Stack *s = vmContextGetStack(vc);
    LinkedListIterator *it = linkedListCreateIteratorAtPos(s, linkedListSize(s) - n);
    for (int64_t i = 0; i < n; i++) {
        vmContextPush(vc, stackObjectClone(vc, linkedListIteratorNext(it)));
    }
    linkedListIteratorDestroy(it);
}

void codeDupn(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    if (n->opcode != OPCODE_INT) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return;
    }
    dupn(vc, n->data.intValue);
}

void codeDup2(VmContext *vc) {
    dupn(vc, 2);
}

void codeDup3(VmContext *vc) {
    dupn(vc, 3);
}

void codeToArray(VmContext *vc) {
    StackObject *n = vmContextPopOpcode(vc, OPCODE_INT);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    Stack *s = vmContextGetStack(vc);
    if ((n->data.intValue < 0) || (n->data.intValue > linkedListSize(s))) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERAND);
        return;
    }
    StackObject *newarray = stackObjectNewEmptyArray(vc);
    LinkedList *array = vmContextGetArray(vc, newarray);
    for (int64_t i = 0; i < n->data.intValue; i++) {
        linkedListInsert(array, 0, stackObjectClone(vc, vmContextPop(vc)));
    }
    vmContextPush(vc, newarray);
}

void codeToString(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
#define TOSTRINGBUFLENGTH 25
    char* str = memAlloc(TOSTRINGBUFLENGTH);
    switch (n->opcode) {
        case OPCODE_INT:
            snprintf(str, TOSTRINGBUFLENGTH, "%lld", n->data.intValue);
            break;
        case OPCODE_FLOAT:
            snprintf(str, TOSTRINGBUFLENGTH, "%f", n->data.floatValue);
            break;
        case OPCODE_BOOLEAN:
            strncpy(str, (n->data.booleanValue ? "true" : "false"), TOSTRINGBUFLENGTH);
            break;
        case OPCODE_NULL:
            strncpy(str, "null", TOSTRINGBUFLENGTH);
            break;
        default:
            vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
            return;
    }
    vmContextPush(vc, stackObjectNewString(vc, str));
    memFree(str);
}

void codeToInt(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    int64_t i;
    switch (n->opcode) {
        case OPCODE_FLOAT:
            i = (int64_t) n->data.floatValue;
            break;
        case OPCODE_STRING:
            i = atoll(vmContextGetString(vc, n));
            break;
        default:
            vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
            return;
    }
    vmContextPush(vc, stackObjectNewInt(vc, i));
}

void codeToFloat(VmContext *vc) {
    StackObject *n = vmContextPop(vc);
    if (n == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    double64_t d;
    switch (n->opcode) {
        case OPCODE_INT:
            d = (double64_t) n->data.intValue;
            break;
        case OPCODE_STRING:
            d = atof(vmContextGetString(vc, n));
            break;
        default:
            vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
            return;
    }
    vmContextPush(vc, stackObjectNewFloat(vc, d));
}

void codeIsNull(VmContext *vc) {
    StackObject *so = vmContextPop(vc);
    if (so == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
    } else {
        vmContextPush(vc, stackObjectNewBool(vc, so->opcode == OPCODE_NULL));
    }
}

void codeIsNotNull(VmContext *vc) {
    StackObject *so = vmContextPop(vc);
    if (so == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
    } else {
        vmContextPush(vc, stackObjectNewBool(vc, so->opcode != OPCODE_NULL));
    }
}

void codeExplode(VmContext *vc) {
    StackObject *oldarray = vmContextPopOpcode(vc, OPCODE_ARRAY);
    if (oldarray == NULL) {
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

void codeNew(VmContext *vc) {
    StackObject *so = vmContextPopOpcode(vc, OPCODE_CLASSREF);
    ClassPointer *cp = linkedListGet(vmContextGetCurrentImage(vc)->bef->classes,
                                     so->data.classIndex);
    vmContextPush(vc, stackObjectNewObject(vc, cp));
}

void codeSelf(VmContext *vc) {
    if (vc->self == NULL) {
        vmContextSetError(vc, VM_ERROR_SELFOUTSIDECLASS);
        return;
    }
    vmContextPush(vc, stackObjectClone(vc, vc->self));
}

void codeSuper(VmContext *vc) {
    if (vc->self == NULL) {
        vmContextSetError(vc, VM_ERROR_SUPEROUTSIDECLASS);
        return;
    }
    ClassInstance *object = vc->self->data.privateData;
    if (object->cp->parent == NULL) {
        vmContextSetError(vc, VM_ERROR_SUPERWITHOUTPARENT);
        return;
    }
    vmContextPush(vc, stackObjectSuper(vc, object));
}

const InternalFunction codeArray[] = {
    codeNop,
    codeAdd,
    codeSub,
    codeMul,
    codeDiv,
    codeSwap,
    codeDrop,
    codeDup,
    codeEq,
    codeNeq,
    codeNot,
    codeAnd,
    codeOr,
    codeGt,
    codeLt,
    codeGe,
    codeLe,
    codeExec,
    codeDupn,
    codePick2Over,
    codeToString,
    codeToInt,
    codeToFloat,
    codeMod,
    codeDepth,
    codeRoll,
    codeRolld,
    codeRoll3,
    codeRolld3,
    codePick,
    codePick3,
    codeDup2,
    codeDup3,
    codeDropn,
    codeDrop2,
    codeDrop3,
    codeClear,
    codeUnpick,
    codeToArray,
    codePick2Over,
    codeIsNull,
    codeIsNotNull,
    codeExplode,
    codeNew,
    codeSelf,
    codeSuper,
    NULL
};
