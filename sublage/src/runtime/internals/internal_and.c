#include "internals_tables.h"

void opcodeAndInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue &
                                       operand1->data.intValue));
}


void opcodeAndBoolBool(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewBool(vc, operand2->data.booleanValue &&
                                       operand1->data.booleanValue));
}

Internal2OperandsCode internalAndTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, opcodeAndBoolBool, NULL, NULL, NULL },
    { NULL, NULL, opcodeAndInt64Int64, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

