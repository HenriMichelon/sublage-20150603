#include "internals_tables.h"

void opcodeOrInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue |
                                       operand1->data.intValue));
}

void opcodeOrBoolBool(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.booleanValue ||
                                       operand1->data.booleanValue));
}

Internal2OperandsCode internalOrTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, opcodeOrBoolBool, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeOrInt64Int64, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

