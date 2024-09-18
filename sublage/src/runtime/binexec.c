#include <stdlib.h>
#include <string.h>
#include "sublage/dynload.h"
#include "sublage/mem.h"
#include "sublage/strbuffer.h"
#include "sublage/binexec.h"

const char* binexecGetSourceFileName(BinExecFile *bef) {
    if (bef->sourceFileName != NULL) {
        return bef->sourceFileName;
    }
    return "(no debug symbols)";
}

char* binexecGetString(BinExecFile *bef, uint32_t stringIndex) {
    if (stringIndex >= linkedListSize(bef->strings)) {
        return NULL;
    }
    LinkedListIterator *it = linkedListCreateIterator(bef->strings);
    char* string = NULL;
    do {
        string = linkedListIteratorNext(it);
    } while (stringIndex--);
    linkedListIteratorDestroy(it);
    return string;
}

LinkedList* binexecGetArray(BinExecFile *bef, uint32_t arrayIndex) {
    if (arrayIndex >= linkedListSize(bef->arrays)) {
        return NULL;
    }
    LinkedListIterator *it = linkedListCreateIterator(bef->arrays);
    LinkedList* array = NULL;
    do {
        array = linkedListIteratorNext(it);
    } while (arrayIndex--);
    linkedListIteratorDestroy(it);
    return array;
}

LinkedList* binexecGetImports(BinExecFile *bef) {
    return bef->imports;
}

LinkedList* binexecGetFunctions(BinExecFile *bef) {
    return bef->functions;
}

LinkedList* binexecGetClasses(BinExecFile *bef) {
    return bef->classes;
}

LinkedList* binexecGetImportedFunctions(BinExecFile *bef) {
    return bef->importedFunctions;
}

void* binexecGetNativeHandler(BinExecFile *bef) {
    return bef->dlHandle;
}

uint8_t* binexecGetCodeStack(BinExecFile *bef) {
    return bef->codeStack;
}

uint64_t binexecGetCodeSize(BinExecFile *bef) {
    return bef->codeSize;
}

LinkedList* binexecGetNativeFunctions(BinExecFile *bef) {
    return bef->nativesFunctions;
}

FunctionPointer* binexecFindFunction(BinExecFile *bef, char *name) {
    LinkedListIterator *it = linkedListCreateIterator(bef->functions);
    FunctionPointer *f = NULL;
    while ((f = linkedListIteratorNext(it)) != NULL) {
        if (strcmp(name, f->name) == 0) {
            break;
        }
    }
    linkedListIteratorDestroy(it);
    return f;
}

ClassPointer* binexecFindClass(BinExecFile *bef, char *name) {
    LinkedListIterator *it = linkedListCreateIterator(bef->classes);
    ClassPointer *cp = NULL;
    while ((cp = linkedListIteratorNext(it)) != NULL) {
        if (strcmp(name, cp->name) == 0) {
            break;
        }
    }
    linkedListIteratorDestroy(it);
    return cp;
}

int32_t binexecFindNativeFunctionIndex(BinExecFile *bef, char *name) {
    LinkedListIterator *it = linkedListCreateIterator(bef->nativesFunctions);
    NativePointer *np = NULL;
    uint32_t index = 0;
    while ((np = linkedListIteratorNext(it)) != NULL) {
        if ((strcmp(name, np->name) == 0) && (np->import == -1)) {
            linkedListIteratorDestroy(it);
            return index;
        }
        index++;
    }
    linkedListIteratorDestroy(it);
    return -1;
}

void binexecDestroy(BinExecFile *bef) {
    if( bef->sourceFileName != NULL) {
        strbufferDestroy(bef->sourceFileName);
        if (bef->debugSymbols != NULL) {
            memFree(bef->debugSymbols);
        }
        if (bef->variablesName != NULL) {
            linkedListDestroy(bef->variablesName, true);
        }
        if (bef->sourceText != NULL) {
            memFree(bef->sourceText);
        }
    }
    if (bef->functions != NULL) {
        LinkedListIterator *it = linkedListCreateIterator(bef->functions);
        FunctionPointer *f = NULL;
        while ((f = linkedListIteratorNext(it)) != NULL) {
            strbufferDestroy(f->name);
            memFree(f);
        }
        linkedListIteratorDestroy(it);
        linkedListDestroy(bef->functions, false);
    }

    if (bef->importedFunctions != NULL) {
        LinkedListIterator *it = linkedListCreateIterator(bef->importedFunctions);
        FunctionPointer *f = NULL;
        while ((f = linkedListIteratorNext(it)) != NULL) {
            strbufferDestroy(f->name);
            memFree(f);
        }
        linkedListIteratorDestroy(it);
        linkedListDestroy(bef->importedFunctions, false);
    }

    if (bef->nativesFunctions != NULL) {
        LinkedListIterator *it = linkedListCreateIterator(bef->nativesFunctions);
        NativePointer *np = NULL;
        while ((np = linkedListIteratorNext(it)) != NULL) {
            strbufferDestroy(np->name);
            memFree(np);
        }
        linkedListIteratorDestroy(it);
        linkedListDestroy(bef->nativesFunctions, false);
    }
    if (bef->classes != NULL) {
        LinkedListIterator *it = linkedListCreateIterator(bef->classes);
        ClassPointer *cp = NULL;
        while ((cp = linkedListIteratorNext(it)) != NULL) {
            strbufferDestroy(cp->name);
            if (cp->import == -1) {
                strbufferDestroy(cp->parentClassName);
                LinkedListIterator *itp = linkedListCreateIterator(cp->variables);
                InstanceVariablePointer *ivp = NULL;
                while ((ivp = linkedListIteratorNext(itp)) != NULL) {
                    strbufferDestroy(ivp->name);
                    strbufferDestroy(ivp->readMethod);
                    strbufferDestroy(ivp->writeMethod);
                }
                linkedListIteratorDestroy(itp);
                linkedListDestroy(cp->variables, true);
                itp = linkedListCreateIterator(cp->functions);
                InstanceFunctionPointer *ifp = NULL;
                while ((ifp = linkedListIteratorNext(itp)) != NULL) {
                    strbufferDestroy(ifp->name);
                }
                linkedListIteratorDestroy(itp);
                linkedListDestroy(cp->functions, true);
            }
        }
        linkedListIteratorDestroy(it);
        linkedListDestroy(bef->classes, true);
    }
    if (bef->fileName != NULL) {
        strbufferDestroy(bef->fileName);
    }
    if (bef->imports != NULL) {
        linkedListDestroy(bef->imports, true);
    }
    if (bef->arrays != NULL) {
        linkedListDestroy(bef->arrays, true);
    }
    if (bef->strings != NULL) {
        linkedListDestroy(bef->strings, true);
    }
    if (bef->codeStack != NULL) {
        memFree(bef->codeStack);
    }
    if (bef->dlHandle != NULL) {
        DynloadUnload(bef->dlHandle);
    }
    memFree(bef);
}
