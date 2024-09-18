#include "internals_tables.h"
#include "sublage/strbuffer.h"

void opcodeAddInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue +
                                       operand1->data.intValue));
}

void opcodeAddInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.intValue +
                                       operand1->data.floatValue));
}

void opcodeAddDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue +
                                       operand1->data.intValue));
}

void opcodeAddDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue +
                                       operand1->data.floatValue));
}

void opcodeAddStringString(VmContext *vc,
                         StackObject *operand1,
                         StackObject *operand2) {
    char *s = strbufferClone(vmContextGetString(vc, operand2));
    s = strbufferAppendStr(s, vmContextGetString(vc, operand1), -1);
    vmContextPush(vc, stackObjectNewString(vc, s));
    strbufferDestroy(s);
}

void opcodeAddArrayAny(VmContext *vc,
                         StackObject *operand1,
                         StackObject *operand2) {
    StackObject *so = stackObjectNewArray(vc, vmContextGetArray(vc, operand2));
    LinkedList *array = vmContextGetArray(vc, so);
    linkedListAppend(array, stackObjectClone(vc, operand1));
    vmContextPush(vc, so);
}

void opcodeAddAnyArray(VmContext *vc,
                         StackObject *operand1,
                         StackObject *operand2) {
    StackObject *so = stackObjectNewArray(vc, vmContextGetArray(vc, operand1));
    LinkedList *array = vmContextGetArray(vc, so);
    linkedListInsert(array, 0, stackObjectClone(vc, operand2));
    vmContextPush(vc, so);
}


void opcodeAddArrayArray(VmContext *vc,
                         StackObject *operand1,
                         StackObject *operand2) {
    StackObject *so = stackObjectNewArray(vc, vmContextGetArray(vc, operand2));
    LinkedList *array = vmContextGetArray(vc, so);
    linkedListAppendList(array, vmContextGetArray(vc, operand1));
    vmContextPush(vc, so);
}


void opcodeAddStringInt64(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone(vmContextGetString(vc, operand2));
    char *istr = memAlloc(25);
    snprintf(istr, 25, "%lld", operand1->data.intValue);
    str = strbufferAppendStr(str, istr, -1);
    vmContextPush(vc, stackObjectNewString(vc, str));
    strbufferDestroy(str);
    strbufferDestroy(istr);
}

void opcodeAddInt64String(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone(vmContextGetString(vc, operand1));
    char *istr = memAlloc(25);
    snprintf(istr, 25, "%lld", operand2->data.intValue);
    istr = strbufferAppendStr(istr, str, -1);
    vmContextPush(vc, stackObjectNewString(vc, istr));
    strbufferDestroy(str);
    strbufferDestroy(istr);
}

void opcodeAddStringDouble(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone(vmContextGetString(vc, operand2));
    char *istr = memAlloc(25);
    snprintf(istr, 25, "%f", operand1->data.floatValue);
    str = strbufferAppendStr(str, istr, -1);
    vmContextPush(vc, stackObjectNewString(vc, str));
    strbufferDestroy(str);
    strbufferDestroy(istr);
}

void opcodeAddDoubleString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone(vmContextGetString(vc, operand1));
    char *istr = memAlloc(25);
    snprintf(istr, 25, "%f", operand2->data.floatValue);
    istr = strbufferAppendStr(istr, str, -1);
    vmContextPush(vc, stackObjectNewString(vc, istr));
    strbufferDestroy(str);
    strbufferDestroy(istr);
}

void opcodeAddStringBoolean(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone(vmContextGetString(vc, operand2));
    str = strbufferAppendStr(str, (operand1->data.booleanValue ? "true" : "false"), -1);
    vmContextPush(vc, stackObjectNewString(vc, str));
    strbufferDestroy(str);
}

void opcodeAddBooleanString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone((operand2->data.booleanValue ? "true" : "false"));
    str = strbufferAppendStr(str, vmContextGetString(vc, operand1), -1);
    vmContextPush(vc, stackObjectNewString(vc, str));
    strbufferDestroy(str);
}

void opcodeAddStringNull(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone(vmContextGetString(vc, operand2));
    str = strbufferAppendStr(str, "null", -1);
    vmContextPush(vc, stackObjectNewString(vc, str));
    strbufferDestroy(str);
}

void opcodeAddNullString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char *str = strbufferClone("null");
    str = strbufferAppendStr(str, vmContextGetString(vc, operand1), -1);
    vmContextPush(vc, stackObjectNewString(vc, str));
    strbufferDestroy(str);
}

Internal2OperandsCode internalAddTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, opcodeAddStringNull, opcodeAddArrayAny},
    { NULL, NULL, NULL, NULL, opcodeAddStringBoolean, opcodeAddArrayAny },
    { NULL, NULL, opcodeAddInt64Int64, opcodeAddDoubleInt64, opcodeAddStringInt64, opcodeAddArrayAny},
    { NULL, NULL, opcodeAddInt64Double, opcodeAddDoubleDouble, opcodeAddStringDouble, opcodeAddArrayAny},
    { opcodeAddNullString, opcodeAddBooleanString, opcodeAddInt64String, opcodeAddDoubleString, opcodeAddStringString, opcodeAddArrayAny},
    { opcodeAddAnyArray, opcodeAddAnyArray, opcodeAddAnyArray, opcodeAddAnyArray, opcodeAddAnyArray, opcodeAddArrayArray },
};

