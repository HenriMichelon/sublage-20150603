#include "internals_tables.h"
#include <string.h>

void opcodeGeInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.intValue >=
                                       	operand1->data.intValue)));
}

void opcodeGeInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.intValue >=
                                       	operand1->data.floatValue)));
}

void opcodeGeDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.floatValue >=
                                       	operand1->data.intValue)));
}

void opcodeGeDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, (operand2->data.floatValue >=
                                       	operand1->data.floatValue)));
}

void opcodeGeStringString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char* a = vmContextGetString(vc, operand1);
    char* b = vmContextGetString(vc, operand2);
    vmContextPush(vc, stackObjectNewBool(vc, strcmp(b, a) >= 0));
}

Internal2OperandsCode internalGeTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeGeInt64Int64, opcodeGeDoubleInt64, NULL, NULL},
    { NULL, NULL, opcodeGeInt64Double, opcodeGeDoubleDouble, NULL, NULL },
    { NULL, NULL, NULL, NULL, opcodeGeStringString, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

