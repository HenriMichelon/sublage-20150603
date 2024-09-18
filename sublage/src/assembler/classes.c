#include "sublage/mem.h"
#include <string.h>
#include "classes.h"
#include "sublage/strbuffer.h"

Class *classCreate(const char *name,uint32_t importIndex) {
    Class *f = memAlloc(sizeof (Class));
    f->name = strbufferClone(name);
    f->parent = NULL;
    f->import = importIndex;
    if (importIndex == -1) {
        f->functions = linkedListCreate();
        f->variables = linkedListCreate();
    }
    f->refCalls = linkedListCreate();
    return f;
}

void classDestroy(Class *c) {
    if (c->import == -1) {
        LinkedListIterator *lli = linkedListCreateIterator(c->functions);
        Function *f = NULL;
        while ((f = linkedListIteratorNext(lli)) != NULL) {
            functionDestroy(f);
        }
        linkedListIteratorDestroy(lli);
        lli = linkedListCreateIterator(c->variables);
        InstanceVariable *iv = NULL;
        while ((iv = linkedListIteratorNext(lli)) != NULL) {
            ivarDestroy(iv);
        }
        linkedListIteratorDestroy(lli);
        linkedListDestroy(c->functions, false);
        linkedListDestroy(c->variables, false);
    }
    linkedListDestroy(c->refCalls, true);
    if (c->parent != NULL) {
        strbufferDestroy(c->parent);
    }
    strbufferDestroy(c->name);
    memFree(c);
}

void classSetParent(Class *c, const char* parent) {
    c->parent = strbufferClone(parent);
}

char* classGetName(Class *c) {
    return c->name;
}

LinkedList* classGetFunctions(Class *c) {
    return c->functions;
}

LinkedList* classGetVariables(Class *c) {
    return c->variables;
}

void classAddFunction(Class *c, Function *f) {
    linkedListAppend(c->functions, f);
}

void classAddVariable(Class *c, InstanceVariable *v) {
    linkedListAppend(c->variables, v);
}

InstanceVariable* ivarCreate(char* name) {
    InstanceVariable *iv = memAlloc(sizeof(InstanceVariable));
    size_t len = strlen(name);
    iv->name = memAlloc(len + 1);
    strncpy(iv->name, name, len);
    iv->name[len] = 0;
    iv->readAccess = false;
    iv->writeAccess = false;
    iv->readMethod = strbufferCreate();
    iv->writeMethod = strbufferCreate();
    return iv;
}

void ivarDestroy(InstanceVariable *iv) {
    strbufferDestroy(iv->readMethod);
    strbufferDestroy(iv->writeMethod);
    memFree(iv->name);
    memFree(iv);
}
