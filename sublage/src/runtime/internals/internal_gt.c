#include "internals_tables.h"
#include <string.h>

void opcodeGtInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue >
                                       operand1->data.intValue));
}

void opcodeGtDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue >
                                       operand1->data.floatValue));
}

void opcodeGtInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.intValue >
                                       operand1->data.floatValue));
}

void opcodeGtDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.floatValue >
                                       operand1->data.intValue));
}

void opcodeGtStringString(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    char* a = vmContextGetString(vc, operand1);
    char* b = vmContextGetString(vc, operand2);
    vmContextPush(vc, stackObjectNewBool(vc, strcmp(b, a) > 0));
}
Internal2OperandsCode internalGtTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeGtInt64Int64, opcodeGtDoubleInt64, NULL, NULL},
    { NULL, NULL, opcodeGtInt64Double, opcodeGtDoubleDouble, NULL, NULL },
    { NULL, NULL, NULL, NULL, opcodeGtStringString, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

