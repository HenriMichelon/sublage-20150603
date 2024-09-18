#include "internals_tables.h"
#include <string.h>

void opcodeNeqNullAny(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand1->opcode != OPCODE_NULL));
}

void opcodeNeqAnyNull(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    vmContextPush(vc, stackObjectNewBool(vc, operand2->opcode != OPCODE_NULL));
}

void opcodeNeqInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue !=
                                       operand1->data.intValue));
}

void opcodeNeqInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue !=
                                       operand1->data.floatValue));
}

void opcodeNeqDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue !=
                                       operand1->data.intValue));
}

void opcodeNeqDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue !=
                                       operand1->data.floatValue));
}

void opcodeNeqBoolBool(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.booleanValue !=
                                       operand1->data.booleanValue));
}

void opcodeNeqStringString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char* a = vmContextGetString(vc, operand1);
    char* b = vmContextGetString(vc, operand2);
    vmContextPush(vc, stackObjectNewBool(vc, strcmp(a, b) != 0));
}
Internal2OperandsCode internalNeqTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { opcodeNeqNullAny, opcodeNeqAnyNull, opcodeNeqAnyNull, opcodeNeqAnyNull, opcodeNeqAnyNull, opcodeNeqAnyNull },
    { opcodeNeqNullAny, opcodeNeqBoolBool, NULL, NULL, NULL, NULL },
    { opcodeNeqNullAny, NULL, opcodeNeqInt64Int64, opcodeNeqDoubleInt64, NULL, NULL },
    { opcodeNeqNullAny, NULL, opcodeNeqInt64Double, opcodeNeqDoubleDouble, NULL, NULL },
    { opcodeNeqNullAny, NULL, NULL, NULL, opcodeNeqStringString, NULL },
    { opcodeNeqNullAny, NULL, NULL, NULL, NULL, NULL },
};
