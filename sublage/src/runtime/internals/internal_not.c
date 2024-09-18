#include "internals_tables.h"

void opcodeNotBool(VmContext *vc,
        StackObject *operand) {
    vmContextPush(vc, stackObjectNewBool(vc, !operand->data.booleanValue));
}

Internal1OperandCode internalNotTable[OPCODE_TYPECOUNT] = {
    NULL, opcodeNotBool, NULL, NULL, NULL, NULL
};

