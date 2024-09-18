#pragma once

#include "sublage/lexical.h"
#include "function.h"

typedef struct {
    LexicalContext *lexicalContext;
    LexicalToken currentToken;
    LexicalToken nextToken;
    Function *currentFunction;
    LinkedList *functions;
    LinkedList *imports;
    LinkedList *strings;
    LinkedList *natives;
    LinkedList *variables;
    LinkedList *arrays;
    LinkedList *classes;
} SyntaxContext;

SyntaxContext* syntaxCreate(LexicalContext *lc);
SyntaxContext* syntaxInit(LexicalContext *lc);
void syntaxDone(SyntaxContext *sc);
void syntaxAnalyse(SyntaxContext *sc);
