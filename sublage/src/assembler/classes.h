#pragma once

#include "sublage/lexical.h"
#include "function.h"

typedef struct {
    char *name;
    int32_t import;
    LinkedList *functions;
    LinkedList *variables;
    LinkedList *refCalls;
    char *parent;
} Class;

typedef struct {
    char *name;
    bool readAccess;
    char *readMethod;
    bool writeAccess;
    char *writeMethod;
} InstanceVariable;

Class *classCreate(const char *name, uint32_t importIndex);
void classSetParent(Class *c, const char* parent);
void classDestroy(Class *c);
char* classGetName(Class *c);
LinkedList* classGetFunctions(Class *c);
LinkedList* classGetVariables(Class *c);
void classAddFunction(Class *c, Function *f);
void classAddVariable(Class *c, InstanceVariable *v);

InstanceVariable* ivarCreate(char* name);
void ivarDestroy(InstanceVariable *iv);
