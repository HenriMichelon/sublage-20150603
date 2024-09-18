#pragma once

#include "sublage/internals.h"

typedef enum {
    RESERVEDWORD_STOPFLAG = -1,
    RESERVEDWORD_NULL,
    RESERVEDWORD_IMPORT,
    RESERVEDWORD_AS,
    RESERVEDWORD_TRUE,
    RESERVEDWORD_FALSE,
    RESERVEDWORD_RETURN,
    RESERVEDWORD_IF,
    RESERVEDWORD_ELSE,
    RESERVEDWORD_WHILE,
    RESERVEDWORD_DO,
    RESERVEDWORD_NATIVE,
    RESERVEDWORD_VAR,
    RESERVEDWORD_BREAK,
    RESERVEDWORD_CONTINUE,
    RESERVEDWORD_FOR,
    RESERVEDWORD_FOREACH,
    RESERVEDWORD_CLASS,
    RESERVEDWORD_READ,
    RESERVEDWORD_WRITE,
    RESERVEDWORD_EXTENDS,
} ReservedWord;

typedef struct {
    ReservedWord  word;
    const char*   identifier;
} ReservedWordIdentifier;


extern ReservedWordIdentifier asReservedWords[];

const char* asOpcode2Identifier(Internal opcode);
const char* asReservedWord2Identifier(ReservedWord word);
long asIdentier2Internal(const char*identifier);
long asIdentier2ReservedWord(const char*identifier);

