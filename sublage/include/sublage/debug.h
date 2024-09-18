#pragma once

#include "sublage/context.h"
#include "sublage/binexec.h"
#include <stdio.h>

typedef struct {
    char*       source;
    uint64_t    line;
} DebugBreakpoint;

DebugContext* debugCreateContext(FILE* debugFileIn, FILE* debugFileOut);
void debugDestroyContext(DebugContext *dc);
void debugAddContext(DebugContext *ctx, VmContext *vc);
void debugRemoveContext(DebugContext *ctx, VmContext *vc);
void debugSendMessage(DebugContext *dc, const char* fmt, ...);
void debugSendInfoMessage(DebugContext *ctx, const char* fmt, ...);
void debugSendAck(DebugContext *ctx);

void debugLoadFile(BinExecFile *bef);
bool debugIsAtBreakpoint(VmContext *vc, uint64_t codeIndex);
bool debugIsAtLineNumber(BinExecFile *bef, uint64_t offset, uint64_t line);
uint64_t debugFindLineNumber(BinExecFile *bef, uint64_t offset);
DebugSymbol* debugFindSymbol(BinExecFile *bef, uint64_t offset);
const char* debugFindVariableName(BinExecFile *bef, uint32_t var_index);
int32_t debugFindVariableIndex(BinExecFile *bef, char *varname);
char* debugGetText(BinExecFile *bef, uint64_t start, uint64_t end);

bool debugDebug(VmContext *vc, uint64_t codeIndex);

