#include "internals_tables.h"
#include "sublage/strbuffer.h"
#include <string.h>

void opcodeMulInt64Int64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewInt(vc, operand2->data.intValue *
                                       operand1->data.intValue));
}

void opcodeMulInt64Double(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.intValue *
                                       operand1->data.floatValue));
}

void opcodeMulDoubleInt64(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue *
                                       operand1->data.intValue));
}

void opcodeMulDoubleDouble(VmContext *vc, 
                         StackObject *operand1, 
                         StackObject *operand2) {
 vmContextPush(vc, stackObjectNewFloat(vc, operand2->data.floatValue *
                                       operand1->data.floatValue));
}

void opcodeMulStringInt64(VmContext *vc,
        StackObject *operand1,
        StackObject *operand2) {
    const char *b = vmContextGetString(vc, operand2);
    char *str = strbufferCreate();
    for (int64_t i = 0; i < operand1->data.intValue; i++) {
        str = strbufferAppendStr(str, b, -1);
    }
    vmContextPush(vc, stackObjectNewString(vc, str));
    strbufferDestroy(str);
}

Internal2OperandsCode internalMulTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT] = {
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, opcodeMulInt64Int64, opcodeMulDoubleInt64, opcodeMulStringInt64, NULL },
    { NULL, NULL, opcodeMulInt64Double, opcodeMulDoubleDouble, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL },
};

