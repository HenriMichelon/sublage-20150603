#include "internals_tables.h"
#include <string.h>

void opcodeLtInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue <
                                       operand1->data.intValue));
}

void opcodeLtInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue <
                                       operand1->data.floatValue));
}

void opcodeLtDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue <
                                       operand1->data.intValue));
}

void opcodeLtDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue <
                                       operand1->data.floatValue));
}

void opcodeLtStringString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char* a = vmContextGetString(vc, operand1);
    char* b = vmContextGetString(vc, operand2);
    vmContextPush(vc, stackObjectNewBool(vc, strcmp(b, a) < 0));
}

Internal2OperandsCode internalLtTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeLtInt64Int64, opcodeLtDoubleInt64, NULL, NULL},
    { NULL, NULL, opcodeLtInt64Double, opcodeLtDoubleDouble, NULL, NULL },
    { NULL, NULL, NULL, NULL, opcodeLtStringString, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

