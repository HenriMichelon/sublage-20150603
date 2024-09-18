#include "internals_tables.h"
#include <string.h>

void opcodeLeInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.intValue <=
                                       	operand1->data.intValue)));
}

void opcodeLeInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.intValue <=
                                       	operand1->data.floatValue)));
}

void opcodeLeDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.floatValue <=
                                       	operand1->data.intValue)));
}

void opcodeLeDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.floatValue <=
                                       	operand1->data.floatValue)));
}

void opcodeLeStringString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char* a = vmContextGetString(vc, operand1);
    char* b = vmContextGetString(vc, operand2);
    vmContextPush(vc, stackObjectNewBool(vc, strcmp(b, a) <= 0));
}

Internal2OperandsCode internalLeTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeLeInt64Int64, opcodeLeDoubleInt64, NULL, NULL},
    { NULL, NULL, opcodeLeInt64Double, opcodeLeDoubleDouble, NULL, NULL },
    { NULL, NULL, NULL, NULL, opcodeLeStringString, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

