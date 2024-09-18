#pragma once

#include <stdio.h>
#include <math.h>
#include "sublage/types.h"
#include "sublage/reservedwords.h"
#include "sublage/linkedlist.h"

typedef enum {
    LEXICALSYMBOL_UNKNOWN,
    LEXICALSYMBOL_DECIMAL,
    LEXICALSYMBOL_INTERNAL,
    LEXICALSYMBOL_RESERVEDWORD,
    LEXICALSYMBOL_IDENTIFIER,
    LEXICALSYMBOL_STARTFUNC,
    LEXICALSYMBOL_ENDFUNC,
    LEXICALSYMBOL_STRING,
    LEXICALSYMBOL_REF,
    LEXICALSYMBOL_VARSET,
    LEXICALSYMBOL_STARTARRAY,
    LEXICALSYMBOL_ENDARRAY,
    LEXICALSYMBOL_EOF,
} LexicalSymbol;

#define IDENTIFER_MAX_SIZE 100

typedef struct {
    LexicalSymbol symbol;
    uint64_t      lineNumber;
    uint64_t      startingChar;
    uint64_t      endingChar;
    
    union {
        int64_t intValue;
        double64_t floatValue;
        Internal internal;
        ReservedWord word;
        char identifier[IDENTIFER_MAX_SIZE];
        char *string;
        LinkedList *array;
    } data;

    union {
        bool isDecimalFloat;

        struct {
            LinkedList *code;
            LinkedList *code2;
        } code;
    } options;
} LexicalToken;

typedef struct {
    FILE*   file;
    char*   fileName;
    char    currentChar;
    uint64_t currentLine;
} LexicalContext;

LexicalContext* lexicalInit(FILE*file, const char* fileName);
void lexicalDone(LexicalContext *lc);

void lexicalNextToken(LexicalContext *lc, LexicalToken *lt);
uint64_t lexicalCurrentLine(LexicalContext *lc);

LexicalToken* lexicalCloneToken(LexicalToken *lt);
void lexicalDestroyToken(LexicalToken *lt);

