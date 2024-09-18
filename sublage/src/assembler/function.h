#pragma once

#include "sublage/lexical.h"
#include "sublage/linkedlist.h"

typedef struct {
  char          *name;
  uint64_t      offset;
  LinkedList    *code;
  LinkedList    *calls;
  bool          private;
} Function;

Function *functionCreate(char *name);
void functionDestroy(Function *f);
//void functionAppendCode(Function *f, LexicalToken *lt);
LinkedList *functionGetCodeList(Function *f);
char* functionGetName(Function *f);
void functionAppendCall(Function *f, uint64_t offset);
LinkedList* functiongetGetCallsList(Function *f);
void functionSetOffset(Function *f, uint64_t offset);
uint64_t functionGetOffset(Function *f);

