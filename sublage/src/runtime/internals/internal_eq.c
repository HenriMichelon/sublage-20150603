#include "internals_tables.h"
#include <string.h>

void opcodeEqNullAny(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand1->opcode == OPCODE_NULL));
}

void opcodeEqAnyNull(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand2->opcode == OPCODE_NULL));
}

void opcodeEqInt64Int64(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue ==
            operand1->data.intValue));
}

void opcodeEqInt64Double(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue ==
            operand1->data.floatValue));
}

void opcodeEqDoubleInt64(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue ==
            operand1->data.intValue));
}

void opcodeEqDoubleDouble(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue ==
            operand1->data.floatValue));
}

void opcodeEqBoolBool(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand2->data.booleanValue ==
            operand1->data.booleanValue));
}

void opcodeEqStringString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char* a = vmContextGetString(vc, operand1);
    char* b = vmContextGetString(vc, operand2);
    vmContextPush(vc, stackObjectNewBool(vc, strcmp(a, b) == 0));
}

Internal2OperandsCode internalEqTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { opcodeEqNullAny, opcodeEqAnyNull, opcodeEqAnyNull, opcodeEqAnyNull, opcodeEqAnyNull, opcodeEqAnyNull},
    { opcodeEqNullAny, opcodeEqBoolBool, NULL, NULL, NULL, NULL},
    { opcodeEqNullAny, NULL, opcodeEqInt64Int64, opcodeEqDoubleInt64, NULL, NULL},
    { opcodeEqNullAny, NULL, opcodeEqInt64Double, opcodeEqDoubleDouble, NULL, NULL},
    { opcodeEqNullAny, NULL, NULL, NULL, opcodeEqStringString, NULL},
    { opcodeEqNullAny, NULL, NULL, NULL, NULL, NULL},
};

