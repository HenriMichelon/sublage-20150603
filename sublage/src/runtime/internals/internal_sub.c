#include "internals_tables.h"

void opcodeSubInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue -
                                       operand1->data.intValue));
}

void opcodeSubInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.intValue -
                                       operand1->data.floatValue));
}

void opcodeSubDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue -
                                       operand1->data.intValue));
}

void opcodeSubDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue -
                                       operand1->data.floatValue));
}


Internal2OperandsCode internalSubTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeSubInt64Int64, opcodeSubDoubleInt64, NULL, NULL },
    { NULL, NULL, opcodeSubInt64Double, opcodeSubDoubleDouble, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

