#include "internals_tables.h"

void opcodeDivInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue /
                                       operand1->data.intValue));
}

void opcodeDivInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.intValue /
                                       operand1->data.floatValue));
}

void opcodeDivDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue /
                                       operand1->data.intValue));
}

void opcodeDivDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue /
                                       operand1->data.floatValue));
}


Internal2OperandsCode internalDivTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeDivInt64Int64, opcodeDivDoubleInt64, NULL, NULL},
    { NULL, NULL, opcodeDivInt64Double, opcodeDivDoubleDouble, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

