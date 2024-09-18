#include "sublage/mem.h"
#include "sublage/strbuffer.h"
#include "import.h"

Import *importCreate(char *name, BinExecFile *bef) {
    Import *i = memAlloc(sizeof (Import));
    i->name = name;
    i->bef = bef;
    i->alias = NULL;
    return i;
}

void importDestroy(Import *i) {
    if (i->alias != NULL) {
        strbufferDestroy(i->alias);
    }
    strbufferDestroy(i->name);
    binexecDestroy(i->bef);
    memFree(i);
}

void importSetAlias(Import *i, char*alias) {
    if (i->alias != NULL) {
        strbufferDestroy(i->alias);
    }
    i->alias = strbufferClone(alias);
}

