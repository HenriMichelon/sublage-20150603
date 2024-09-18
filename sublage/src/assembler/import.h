#pragma once

#include "sublage/binexec.h"

typedef struct {
  char        *name;
  char        *alias;
  BinExecFile *bef;
} Import;

Import *importCreate(char *name, BinExecFile *bef);
void importDestroy(Import *i);
void importSetAlias(Import *i, char*alias);

static inline char* importGetName(Import *i) {
  return i->name;
}
static inline char* importGetAlias(Import *i) {
  return i->alias;
}
static inline LinkedList *importGetFunctions(Import *i) {
    return binexecGetFunctions(i->bef);
}
static inline LinkedList *importGetClasses(Import *i) {
    return binexecGetClasses(i->bef);
}
static inline LinkedList *importGetNativeFunctions(Import *i) {
  return binexecGetNativeFunctions(i->bef);
}
