#pragma once

#include <stdio.h>
#include <stdbool.h>
#include "sublage/binexec.h"
#include "sublage/internals.h"
#include "function.h"
#include "syntax.h"

bool binexecWriteHeader(FILE *out, FILE *debug, SyntaxContext *sc);
bool binexecWriteContent(FILE *out, FILE *debug, SyntaxContext *sc);
bool binexecWriteDebugSymbol(FILE *out, FILE *debug, LexicalToken *lt,
                             uint64_t currentFunctionStartOffset);
