#pragma once

#include "sublage/vmcontext.h"

typedef void (*Internal2OperandsCode) (VmContext *vc, StackObject *op1, StackObject *op2);
typedef Internal2OperandsCode Opcode2OperandsTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];

typedef void (*Internal1OperandCode) (VmContext *vc, StackObject *op);
typedef Internal1OperandCode Opcode1OperandTable[OPCODE_TYPECOUNT];

extern Internal1OperandCode internalNotTable[OPCODE_TYPECOUNT];

extern Internal2OperandsCode internalAddTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalSubTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalMulTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalDivTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalModTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalEqTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalNeqTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalAndTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalOrTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalGtTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalLtTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalGeTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];
extern Internal2OperandsCode internalLeTable[OPCODE_TYPECOUNT][OPCODE_TYPECOUNT];

