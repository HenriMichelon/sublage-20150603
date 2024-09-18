#include "internals_tables.h"

void opcodeModInt64Int64(VmContext *vc,
                         StackObject *operand1,
                         StackObject *operand2) {
    vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue %
                                        operand1->data.intValue));
}

void opcodeModInt64Double(VmContext *vc,
                           StackObject *operand1,
                           StackObject *operand2) {
    vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue %
                                          (int64_t)operand1->data.floatValue));
}

void opcodeModDoubleInt64(VmContext *vc,
                           StackObject *operand1,
                           StackObject *operand2) {
    vmContextPush(vc, stackObjectNewInt(vc,
                                          (int64_t)operand2->data.floatValue %
                                          operand1->data.intValue));
}

void opcodeModFLoat32Double(VmContext *vc,
                             StackObject *operand1,
                             StackObject *operand2) {
    vmContextPush(vc, stackObjectNewInt(vc,
                                          (int64_t)operand2->data.floatValue %
                                          (int64_t)operand1->data.floatValue));
}


Internal2OperandsCode internalModTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeModInt64Int64, opcodeModDoubleInt64, NULL, NULL},
    { NULL, NULL, opcodeModInt64Double, opcodeModFLoat32Double, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

