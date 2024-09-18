#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#define _VM_COMPILING_VMCONTEXT_C 1
#include "sublage/types.h"
#include "sublage/dynload.h"
#include "sublage/mem.h"
#include "sublage/stack.h"
#include "sublage/vmerrors.h"
#include "sublage/loader.h"
#include "sublage/debug.h"
#include "internals/internals.h"
#include "sublage/dump.h"

typedef struct {
    uint32_t imageIndex;
    uint64_t pointer;
    StackObject *self;
} Call;

#include "sublage/vmcontext.h"
#include "sublage/strbuffer.h"

VmContext *vmContextCreate(FILE* stackDumpFile,
        DebugContext *debugContext,
        ObjectPools *globalPool,
        uint32_t contextID) {
    VmContext *vc = memAlloc(sizeof (VmContext));
    vc->contextID = contextID;
    vc->stack = stackCreate();
    vc->callStack = stackCreate();
    if (globalPool == NULL) {
        vc->globalPool = memAlloc(sizeof (ObjectPools));
        vc->globalPool->strings = linkedListCreate();
        vc->globalPool->arrays = linkedListCreate();
        vc->globalPool->objects = linkedListCreate();
        ThreadMutexCreate(&vc->globalPool->mutex);
        vc->isClone = false;
    } else {
        vc->globalPool = globalPool;
        vc->isClone = true;
    }
    vc->garbage = stackCreate();
    vc->error = VM_NOERROR;
    vc->imagesCount = 0;
    vc->imagesIndex = 0;
    vc->self = NULL;
    vc->images = NULL;
    vc->isRunning = false;
    vc->stackDumpFile = stackDumpFile;
    vc->debugContext = debugContext;
    vc->isPaused = false;
    vc->stepInto = false;
    vc->exitFromPause = false;
    vc->inStepOver = 0;
    return vc;
}

VmContext *vmContextClone(VmContext *vc) {
    VmContext *newvc = newvc = vmContextCreate(vc->stackDumpFile,
            vc->debugContext, vc->globalPool, vc->contextID+1);
    newvc->imagesCount = vc->imagesCount;
    newvc->imagesIndex = vc->imagesIndex;
    newvc->images = memAlloc(vc->imagesCount * sizeof (BinExecImg*));
    for (int i = 0; i < vc->imagesCount; i++) {
        newvc->images[i] = binexecimgClone(vc->images[i]);
    }
    if (newvc->debugContext != NULL) {
        debugAddContext(newvc->debugContext, newvc);
    }
    return newvc;
}

void vmContextDestroy(VmContext *vc) {
    if (!vc->isClone) {
        LinkedListIterator *it = linkedListCreateIterator(vc->globalPool->objects);
        ClassInstance *object;
        while ((object = linkedListIteratorNext(it)) != NULL) {
            if (!object->issuper) {
                for (int i = 0; i < object->nivars; i++) {
                    memFree(object->ivars[i]);
                }
                memFree(object->ivars);
            }
        }
        linkedListIteratorDestroy(it);
        ThreadMutexLock(&vc->globalPool->mutex);
        linkedListDestroy(vc->globalPool->arrays, true);
        linkedListDestroy(vc->globalPool->strings, true);
        linkedListDestroy(vc->globalPool->objects, true);
        ThreadMutexDestroy(&vc->globalPool->mutex);
        memFree(vc->globalPool);
    }
    linkedListDestroy(vc->garbage, true);
    if (vc->debugContext != NULL) {
        debugRemoveContext(vc->debugContext, vc);
    }
    stackDestroy(vc->callStack, true);
    stackDestroy(vc->stack, true);
    if (vc->images != NULL) {
        for (int i = 0; i < vc->imagesCount; i++) {
            if (vc->isClone) {
                memFree(vc->images[i]);
            } else {
                uint64_t codeIndex = 0;
                StackObject *so = NULL;
                while ((so = loaderNextStackObject(vc->images[i]->bef, &codeIndex)) != NULL) {
                    switch (so->opcode) {
                        case OPCODE_INTERNALCALL_REF:
                        {
                            memFree(so->data.privateData);
                            break;
                        }
                    }
                };
                binexecimgDestroy(vc->images[i]);
            }
        }
        memFree(vc->images);
    }
    memFree(vc);
}

void vmContextConvertInternalCallRef(VmContext *vc, StackObject *so) {
    InternalFunctionRef *ref = memAlloc(sizeof (InternalFunctionRef));
    ref->functionOffset = so->data.functionOffset;
    ref->imageIndex = vc->imagesCount - 1;
    so->data.privateData = ref;
}

StackObject* stackObjectNewArrayFromBinExec(VmContext *vc, uint32_t arrayIndex) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_ARRAY;
    LinkedList *newarray = linkedListCreate();
    LinkedListIterator *it = linkedListCreateIterator(
            binexecimgGetArray(vc->images[vc->imagesIndex], arrayIndex));
    StackObject *elem = NULL;
    while ((elem = linkedListIteratorNext(it)) != NULL) {
        StackObject *clone = NULL;
        switch (elem->opcode) {
            case OPCODE_STRING:
                clone = stackObjectNewString(vc,
                        binexecimgGetString(vc->images[vc->imagesIndex],
                        elem->data.stringIndex));
                break;
            case OPCODE_INTERNALCALL_REF:
                vmContextConvertInternalCallRef(vc, elem);
                clone = stackObjectClone(vc, elem);
                break;
            default:
                clone = stackObjectClone(vc, elem);
                break;
        }
        linkedListAppend(newarray, clone);
    }
    linkedListIteratorDestroy(it);
    ThreadMutexLock(&vc->globalPool->mutex);
    so->data.arrayIndex = (uint32_t) linkedListSize(vc->globalPool->arrays);
    linkedListAppend(vc->globalPool->arrays, newarray);
    ThreadMutexUnlock(&vc->globalPool->mutex);
    return so;
}

void vmContextSetVariable(VmContext *vc, const StackObject *var) {
    StackObject *so = vmContextPop(vc);
    if (so == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return;
    }
    BinExecImg *img = vmContextGetCurrentImage(vc);
    if (img->variables[var->data.variableIndex] != NULL) {
        memFree(img->variables[var->data.variableIndex]);
    }
    img->variables[var->data.variableIndex] = stackObjectClone(vc, so);
}

void vmContextGetVariable(VmContext *vc, const StackObject *var) {
    BinExecImg *img = vmContextGetCurrentImage(vc);
    StackObject *so = img->variables[var->data.variableIndex];
    if (so == NULL) {
        vmContextPush(vc, stackObjectNewNull(vc));
    } else {
        vmContextPush(vc, stackObjectClone(vc, so));
    }
}

void vmContextCallInstanceField(VmContext *vc, const char *fieldname);
void vmContextCallInstanceFieldForClass(VmContext *vc, const char *fieldname,
                                           ClassPointer *cp, StackObject *instance,
                                           ClassInstance *object, int index) {
    // search for an instance variable
    LinkedListIterator *it = linkedListCreateIterator(cp->variables);
    InstanceVariablePointer *ivar = NULL;
    while ((ivar = linkedListIteratorNext(it)) != NULL) {
        if (strbufferEquals(fieldname, ivar->name)) {
            break;
        }
        index++;
    }
    linkedListIteratorDestroy(it);
    if (ivar != NULL) {
        ClassInstance *self = NULL;
        if (vc->self != NULL) {
            self = vc->self->data.privateData;
        }
        // check access level if out of class
        if ((self == NULL) || (self->cp != cp)) {
            if (ivar->readAccess == false) {
                vmContextSetError(vc, VM_ERROR_WRITEONLYIVAR,
                                  cp->name, fieldname);
                return;
            }
            if (ivar->readMethod[0] != '\0') {
                vmContextPush(vc, stackObjectClone(vc, instance));
                vmContextCallInstanceField(vc, ivar->readMethod);
                return;
            }
        }
        vmContextPush(vc, stackObjectClone(vc, object->ivars[index]));
        return;
    }
    
    // search for an instance method
    it = linkedListCreateIterator(cp->functions);
    InstanceFunctionPointer *fp = NULL;
    while ((fp = linkedListIteratorNext(it)) != NULL) {
        if (strbufferEquals(fieldname, fp->name)) {
            break;
        }
    }
    linkedListIteratorDestroy(it);
    if (fp != NULL) {
        ClassInstance *self = NULL;
        if (vc->self != NULL) {
            self = vc->self->data.privateData;
        }
        // check access level if out of class
        if ((self == NULL || (self->cp != cp)) &&
            (fp->private)) {
            vmContextSetError(vc, VM_ERROR_PRIVATEMETHOD,
                                  cp->name, fieldname);
            return;
        }
        vmContextPushCall(vc);
        if (vc->self != NULL) {
            memFree(vc->self);
        }
        vc->self = stackObjectClone(vc, instance);
        vmContextSetCurrentImage(vc, cp->imageIndex);
        binexecimgJumpToOffset(vmContextGetCurrentImage(vc),
                               fp->offset);
        return;
    }
    
    if (cp->parent != NULL) {
        StackObject *super =stackObjectSuper(vc, object);
        vmContextCallInstanceFieldForClass(vc, fieldname, cp->parent,
                                           super, object, index);
        memFree(super);
    } else {
        vmContextSetError(vc, VM_ERROR_INVALIDINSTANCEMEMBER,
                      fieldname, object->cp->name);
    }
}

void vmContextCallInstanceField(VmContext *vc, const char *fieldname) {
    StackObject *instance = vmContextPopOpcode(vc, OPCODE_INSTANCE);
    ClassInstance *object = instance->data.privateData;
    vmContextCallInstanceFieldForClass(vc, fieldname, object->cp,
                                      instance, object, 0);

}

void vmContextSetInstanceFieldForClass(VmContext *vc, const char *fieldname,
                                       ClassPointer *cp, StackObject *instance,
                                       ClassInstance *object, int index) {
    // search for an instance variable
    LinkedListIterator *it = linkedListCreateIterator(cp->variables);
    InstanceVariablePointer *ivar = NULL;
    while ((ivar = linkedListIteratorNext(it)) != NULL) {
        if (strbufferEquals(fieldname, ivar->name)) {
            break;
        }
        index++;
    }
    linkedListIteratorDestroy(it);
    if (ivar != NULL) {
        ClassInstance *self = NULL;
        if (vc->self != NULL) {
            self = vc->self->data.privateData;
        }
        if ((self == NULL) || (self->cp != cp)) {
            if (ivar->writeAccess == false) {
                vmContextSetError(vc, VM_ERROR_READONLYIVAR,
                                  cp->name, fieldname);
                return;
            }
            if (ivar->writeMethod[0] != '\0') {
                vc->self = stackObjectClone(vc, instance);
                vmContextPush(vc, stackObjectClone(vc, instance));
                vmContextCallInstanceField(vc, ivar->writeMethod);
                return;
            }
        }
        memFree(object->ivars[index]);
        object->ivars[index] = stackObjectClone(vc, vmContextPop(vc));
        return;
    }
    
    if (cp->parent != NULL) {
        vmContextSetInstanceFieldForClass(vc, fieldname, cp->parent,
                                          instance, object, index);
    } else {
        vmContextSetError(vc, VM_ERROR_INVALIDINSTANCEMEMBER,
                      fieldname, object->cp->name);
    }
}

void vmContextSetInstanceField(VmContext *vc, const char *fieldname) {
    StackObject *instance = vmContextPopOpcode(vc, OPCODE_INSTANCE);
    if (instance == NULL) { return; }
    ClassInstance *object = instance->data.privateData;
    vmContextSetInstanceFieldForClass(vc, fieldname, object->cp,
                                      instance, object, 0);
}

void vmContextFlushObjectsPool(VmContext* vc) {
    // XXX TODO : clear StackObject datas
    linkedListClear(vc->garbage, true);
}

void vmContextRun(VmContext *parentvc, uint64_t startOffset, StackObject *so) {
    VmContext *vc = vmContextClone(parentvc);
    if (so != NULL) {
        vmContextPush(vc, stackObjectClone(vc, so));
    }
    binexecimgJumpToOffset(vmContextGetCurrentImage(vc), startOffset);
    so = NULL;
    vmContextSetRunning(vc, true);
    while ((so = binexecimgNextStackObject(vmContextGetCurrentImage(vc))) != NULL) {
        if (vc->debugContext != NULL) {
            if (!debugDebug(vc, vmContextGetCurrentImage(vc)->lastCodeIndex)) {
                vmContextDestroy(vc);
                break;
            }
        }
        if (vc->stackDumpFile != NULL) {
            dumpStackObject(so, vc);
        }
        switch (so->opcode) {
            case OPCODE_NULL:
            case OPCODE_PRIVATE:
            case OPCODE_INT:
            case OPCODE_FLOAT:
            case OPCODE_BOOLEAN:
                vmContextPush(vc, stackObjectClone(vc, so));
                break;
            case OPCODE_ARRAY:
                vmContextPush(vc, stackObjectNewArrayFromBinExec(vc, so->data.arrayIndex));
                break;
            case OPCODE_STRING:
                vmContextPush(vc, so = stackObjectNewString(vc,
                        binexecimgGetString(vc->images[vc->imagesIndex],
                        so->data.stringIndex)));
                break;
            case OPCODE_RETURN:
                vmContextSetRunning(vc, vmContextPopCall(vc));
                if ((vc->debugContext != NULL) && (vc->inStepOver)) {
                    vc->inStepOver--;
                    vc->isPaused = vc->inStepOver == 0;
                }
                break;
            case OPCODE_INTERNAL:
            {
                InternalFunction internalFunc = codeArray[so->data.internal];
                if (internalFunc == NULL) {
                    vmContextSetError(parentvc, VM_ERROR_INVALIDINTERNAL, so->data.internal);
                    break;
                }
                (*internalFunc)(vc);
                break;
            }
            case OPCODE_INTERNALCALL:
            case OPCODE_EXTERNALCALL:
                if (vc->debugContext != NULL) {
                    if (vc->stepOver) {
                        vc->inStepOver = 1;
                        vc->isPaused = false;
                    } else if (vc->inStepOver) {
                        vc->inStepOver++;
                    }
                }
                vmContextCallFunction(vc, so);
                break;
            case OPCODE_NATIVECALL:
                vmContextCallFunction(vc, so);
                break;
            case OPCODE_JUMP:
            {
                binexecimgRelativeJumpToOffset(vmContextGetCurrentImage(vc),
                        so->data.jumpOffset);
                break;
            }
            case OPCODE_JUMPIFNOT:
            {
                StackObject *operand1 = vmContextPop(vc);
                if (operand1 == NULL) {
                    vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
                }
                if (operand1->opcode != OPCODE_BOOLEAN) {
                    vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
                }
                if (!operand1->data.booleanValue) {
                    binexecimgRelativeJumpToOffset(vmContextGetCurrentImage(vc),
                            so->data.jumpOffset);
                }
                break;
            }
            case OPCODE_JUMPIF:
            {
                StackObject *operand1 = vmContextPop(vc);
                if (operand1 == NULL) {
                    vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
                }
                if (operand1->opcode != OPCODE_BOOLEAN) {
                    vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
                }
                if (operand1->data.booleanValue) {
                    binexecimgRelativeJumpToOffset(vmContextGetCurrentImage(vc),
                            so->data.jumpOffset);
                }
                break;
            }
            case OPCODE_INTERNALCALL_REF:
            case OPCODE_NATIVECALL_REF:
            case OPCODE_EXTERNALCALL_REF:
            case OPCODE_VAR_REF:
            case OPCODE_CLASSREF:
                vmContextPush(vc, stackObjectClone(vc, so));
                break;
            case OPCODE_VAR_SET:
                vmContextSetVariable(vc, so);
                break;
            case OPCODE_VAR_GET:
                vmContextGetVariable(vc, so);
                break;
            case OPCODE_IFIELDCALL:
            {
                char *fieldname = binexecimgGetString(vc->images[vc->imagesIndex],
                                                      so->data.stringIndex);
                vmContextCallInstanceField(vc, fieldname);
                break;
            }
            case OPCODE_IFIELD_SET:
            {
                char *fieldname = binexecimgGetString(vc->images[vc->imagesIndex],
                                                      so->data.stringIndex);
                vmContextSetInstanceField(vc, fieldname);
                break;
            }
            default:
                vmContextSetError(vc, VM_ERROR_UNKNOWNOPCODE, so->opcode);
                break;
        }
        if (vc->stackDumpFile != NULL) {
            dumpContext(vc);
        }
        if (vmContextGetError(vc) != VM_NOERROR) {
            vmContextSetRunning(vc, false);
        }
        if (!vmContextIsRunning(vc)) {
            vmContextDestroy(vc);
            return;
        }
        vmContextFlushObjectsPool(vc);
    }
}

void vmContextSetError(VmContext *vc, VmErrorCode ec, ...) {
    vc->error = ec;
    char msg[200];
    va_list ap;
    va_start(ap, ec);
    vsnprintf(msg, 200, errorMessages[ec], ap);
    va_end(ap);
    switch (ec) {
        case VM_ERROR_NORUNFUNCTION:
            snprintf(vc->errorMessage, 300, "%s", msg);
            break;
        case VM_ERROR_LOADINGIMPORT:
        case VM_ERROR_LOADINGBINEXEC:
            snprintf(vc->errorMessage, 300, "%s (was: `%s`)",
                    msg, loaderGetErrorMessage());
            break;
        default:
        {
            snprintf(vc->errorMessage, 300, "%s", msg);
            BinExecFile *bef = vmContextGetCurrentImage(vc)->bef;
            debugLoadFile(bef);
            uint64_t index = vmContextGetCurrentImage(vc)->lastCodeIndex;
            DebugSymbol *ds = debugFindSymbol(bef, index);
            if (ds == NULL) {
                fprintf(stderr,
                        "Thread #%d halted in `%s` at 0x%08llx (no debug symbol found): %s.\n",
                        vmContextGetID(vc),
                        bef->fileName,
                        index,
                        vmContextGetErrorMessage(vc));            
            } else {
                fprintf(stderr,
                        "Thread #%d halted in `%s` at 0x%08llx (%s line %lld, token `%s`): %s.\n",
                        vmContextGetID(vc),
                        bef->fileName,
                        index,
                        binexecGetSourceFileName(bef),
                        ds->lineNumber,
                        debugGetText(bef, ds->startingChar, ds->endingChar),
                        vmContextGetErrorMessage(vc));
            }
        }
    }
}

void vmContextCallFunction(VmContext *vc, StackObject *so) {
    switch (so->opcode) {
        case OPCODE_INTERNALCALL:
            vmContextPushCall(vc);
            binexecimgJumpToOffset(vmContextGetCurrentImage(vc),
                    so->data.functionOffset);
            break;
        case OPCODE_INTERNALCALL_REF:
        {
            vmContextPushCall(vc);
            InternalFunctionRef *ref = (InternalFunctionRef*) so->data.privateData;
            fflush(stdout);
            if (ref->imageIndex != vc->imagesIndex) {
                fflush(stdout);
                vmContextSetCurrentImage(vc, ref->imageIndex);
            }
            fflush(stdout);
            binexecimgJumpToOffset(vmContextGetCurrentImage(vc),
                    ref->functionOffset);
        }
            break;
        case OPCODE_EXTERNALCALL:
        case OPCODE_EXTERNALCALL_REF:
        {
            vmContextPushCall(vc);
            ExternalFunction *ef = vmContextGetCurrentImage(vc)->externals[so->data.importIndex];
            vmContextSetCurrentImage(vc, ef->imageIndex);
            binexecimgJumpToOffset(vmContextGetCurrentImage(vc),
                    ef->offset);
            break;
        }
        case OPCODE_NATIVECALL:
        case OPCODE_NATIVECALL_REF:
        {
            NativeFunction *nf = vmContextGetCurrentImage(vc)->natives[so->data.nativeIndex];
            (*nf->function)(vc);
            break;
        }
        default:
            vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
            break;
    }
}

bool vmContextIsRunning(VmContext *vc) {
    return vc->isRunning;
}

uint32_t vmContextGetID(VmContext *vc) {
    return vc->contextID;
}

void vmContextSetRunning(VmContext *vc, bool running) {
    vc->isRunning = running;
}

VmErrorCode vmContextGetError(VmContext *vc) {
    return vc->error;
}

const char* vmContextGetErrorMessage(VmContext *vc) {
    return vc->errorMessage;
}

char *vmContextGetString(VmContext *vc, const StackObject *so) {
    ThreadMutexLock(&vc->globalPool->mutex);
    char* str = linkedListGet(vc->globalPool->strings, so->data.stringIndex);
    ThreadMutexUnlock(&vc->globalPool->mutex);
    return str;
}

LinkedList* vmContextGetArray(VmContext *vc, const StackObject *so) {
    ThreadMutexLock(&vc->globalPool->mutex);
    LinkedList *array = linkedListGet(vc->globalPool->arrays, so->data.arrayIndex);
    ThreadMutexUnlock(&vc->globalPool->mutex);
    return array;
}

uint32_t vmContextAddImage(VmContext *vc, BinExecImg *img) {
    
    // load imports
    LinkedListIterator *it = linkedListCreateIterator(binexecGetImports(img->bef));
    uint32_t *imports = NULL;
    uint32_t importsCount = 0;

    char *importFileName = NULL;
    while ((importFileName = linkedListIteratorNext(it)) != NULL) {
        bool alreadyLoaded = false;
        uint32_t index = 0;
        // search for already loaded library
        for (index = 0; index < vc->imagesCount; index++) {
            if (strcmp(importFileName, vc->images[index]->importName) == 0) {
                alreadyLoaded = true;
                break;
            }
        }
        // load library
        if (!alreadyLoaded) {
            BinExecFile *bef = loaderLoadFileFromFileName(importFileName);
            if (bef == NULL) {
                if (imports != NULL) {
                    memFree(imports);
                }
                linkedListIteratorDestroy(it);
                vmContextSetError(vc, VM_ERROR_LOADINGIMPORT, importFileName);
                return 0;
            }
            BinExecImg *img = binexecimgCreate(bef);
            index = vmContextAddImage(vc, img);
            if (vc->error != VM_NOERROR) {
                binexecimgDestroy(img);
                if (imports != NULL) {
                    memFree(imports);
                }
                linkedListIteratorDestroy(it);
                return 0;
            }
            img->importName = strbufferClone(importFileName);
        }
        imports = memRealloc(imports, sizeof (uint32_t)*(++importsCount));
        imports[importsCount - 1] = index;
    }
    linkedListIteratorDestroy(it);

    // link imported functions
    it = linkedListCreateIterator(binexecGetImportedFunctions(img->bef));
    FunctionPointer *f = NULL;
    while ((f = linkedListIteratorNext(it)) != NULL) {
        BinExecImg *imp = vc->images[imports[f->offset]];
        FunctionPointer *fp = binexecFindFunction(imp->bef, f->name);
        if (fp == NULL) {
            if (imports != NULL) {
                memFree(imports);
            }
            linkedListIteratorDestroy(it);
            vmContextSetError(vc, VM_ERROR_LINKINGEXTERNALFUNCTION,
                    f->name, imp->bef->fileName);
            return 0;
        }
        //printf("linking %s %d -> 0x%04x\n", f->name, f->offset, fp->offset);
        ExternalFunction *ef = memAlloc(sizeof (ExternalFunction));
        ef->imageIndex = imports[f->offset];
        ef->offset = fp->offset;
        img->externals = memRealloc(img->externals, sizeof (ExternalFunction*)*
                (++(img->externalsCount)));
        img->externals[img->externalsCount - 1] = ef;
    }
    linkedListIteratorDestroy(it);

    // load internal native functions
    it = linkedListCreateIterator(binexecGetNativeFunctions(img->bef));
    NativePointer *np = NULL;
    while ((np = linkedListIteratorNext(it)) != NULL) {
        NativeFunction *nf = memAlloc(sizeof (NativeFunction));
        img->natives = memRealloc(img->natives, sizeof (NativeFunction*)*
                (++(img->nativeCount)));
        img->natives[img->nativeCount - 1] = nf;
        // link native function
        if (np->import == -1) {
            nf->imageIndex = -1;
            char* nativeName = strbufferClone("native_");
            nativeName = strbufferAppendStr(nativeName, np->name, -1);
            nf->function = (VmNativeFunction) DynloadGetFunction(
                    binexecGetNativeHandler(img->bef),
                    nativeName);
            strbufferDestroy(nativeName);
            if (nf->function == NULL) {
                linkedListIteratorDestroy(it);
                vmContextSetError(vc, VM_ERROR_LINKINGNATIVEFUNCTION, np->name);
                return 0;
            }
        } else if (np->import < importsCount) {
            uint32_t index = binexecFindNativeFunctionIndex(
                    vc->images[imports[np->import]]->bef, np->name);
            nf->function = vc->images[imports[np->import]]->natives[index]->function;
        } else {
            linkedListIteratorDestroy(it);
            vmContextSetError(vc, VM_ERROR_LINKINGNATIVEFUNCTION, np->name);
            return 0;
        }
    }
    linkedListIteratorDestroy(it);
    
    
    // Link classes
    it = linkedListCreateIterator(img->bef->classes);
    ClassPointer *cp = NULL;
    while ((cp = linkedListIteratorNext(it)) != NULL) {
        if (cp->import == -1) {
            cp->imageIndex = vc->imagesCount;
        } else {
            BinExecImg *imp = vc->images[imports[cp->import]];
            ClassPointer *realcp = binexecFindClass(imp->bef, cp->name);
            if (realcp == NULL) {
                if (imports != NULL) {
                    memFree(imports);
                }
                linkedListIteratorDestroy(it);
                vmContextSetError(vc, VM_ERROR_LINKINGEXTERNALCLASS,
                                  f->name, imp->bef->fileName);
                return 0;
            }
            cp->imageIndex = imports[cp->import];
            cp->functions = realcp->functions;
            cp->variables = realcp->variables;
            cp->parent = realcp->parent;
//            cp->parentImport = realcp->parentImport;
//            cp->parentClassName = realcp->parentClassName;
        }
    }
    linkedListIteratorDestroy(it);
    
    // Link parent classes
    it = linkedListCreateIterator(img->bef->classes);
    cp = NULL;
    while ((cp = linkedListIteratorNext(it)) != NULL) {
        if ((cp->import == -1) && (strlen(cp->parentClassName) > 0)) {
            ClassPointer *parent = NULL;
            BinExecImg *imp = NULL;
            if (cp->parentImport == -1) {
                imp = img;
            } else {
                imp = vc->images[imports[cp->parentImport]];
            }
            parent = binexecFindClass(imp->bef, cp->parentClassName);
            if (parent == NULL) {
                if (imports != NULL) {
                    memFree(imports);
                }
                linkedListIteratorDestroy(it);
                vmContextSetError(vc, VM_ERROR_LINKINGEXTERNALCLASS,
                                    f->name, imp->bef->fileName);
                return 0;
            }
            cp->parent = parent;
        }
    }
    linkedListIteratorDestroy(it);
    

    if (imports != NULL) {
        memFree(imports);
    }

    vc->images = memRealloc(vc->images, sizeof (BinExecImg*)*(++(vc->imagesCount)));
    vc->images[vc->imagesCount - 1] = img;

    // Convert functionOffset fields to InternalFunctionRef references
    uint64_t codeIndex = 0;
    StackObject *so = NULL;
    while ((so = loaderNextStackObject(img->bef, &codeIndex)) != NULL) {
        switch (so->opcode) {
            case OPCODE_INTERNALCALL_REF:
            {
                vmContextConvertInternalCallRef(vc, so);
                break;
            }
        }
    };
    
    debugLoadFile(img->bef);

    return vc->imagesCount - 1;
}

Stack* vmContextGetStack(VmContext *vc) {
    return vc->stack;
}

StackObject* vmContextPop(VmContext *vc) {
    StackObject *so = stackPop(vc->stack);
    if (so != NULL) {
        stackPush(vc->garbage, so);
    }
    return so;
}

StackObject* vmContextPopOpcode(VmContext *vc, uint8_t opcode) {
    StackObject *so = vmContextPop(vc);
    if (so == NULL) {
        vmContextSetError(vc, VM_ERROR_EMPTYSTACK);
        return NULL;
    }
    if (so->opcode != opcode) {
        vmContextSetError(vc, VM_ERROR_INVALIDOPERANDTYPE);
        return NULL;
    }
    return so;
}

void vmContextPush(VmContext *vc, StackObject *so) {
    stackPush(vc->stack, so);
}

bool vmContextPopCall(VmContext *vc) {
    Call* c = stackPop(vc->callStack);
    if (c == NULL) {
        return false;
    }
    vc->imagesIndex = c->imageIndex;
    if (vc->self != NULL) {
        memFree(vc->self);
    }
    vc->self = c->self;
    binexecimgJumpToOffset(vc->images[c->imageIndex], c->pointer);
    memFree(c);
    return true;
}

void vmContextPushCall(VmContext *vc) {
    Call *c = memAlloc(sizeof (Call));
    if (vc->self != NULL) {
        c->self = stackObjectClone(vc, vc->self);
    } else {
        c->self = NULL;
    }
    c->imageIndex = vc->imagesIndex;
    c->pointer = binexecimgGetCurrentIndex(vc->images[c->imageIndex]);
    stackPush(vc->callStack, c);
}

BinExecImg * vmContextGetCurrentImage(VmContext *vc) {
    return vc->images[vc->imagesIndex];
}

void vmContextSetCurrentImage(VmContext *vc, uint32_t imageIndex) {
    vc->imagesIndex = imageIndex;
}

StackObject* stackObjectClone(VmContext *vc, const StackObject *so) {
    StackObject *clone = memAlloc(sizeof (StackObject));
    memcpy(clone, so, sizeof (StackObject));
    //printf("%x\n", clone);
    return clone;
}

StackObject* stackObjectSuper(VmContext *vc, ClassInstance *object) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_INSTANCE;
    ClassInstance *super = memAlloc(sizeof(ClassInstance));
    super->id = object->id;
    super->cp = object->cp->parent;
    super->ivars = object->ivars + linkedListSize(object->cp->variables);
    super->issuper = true;
    so->data.privateData = super;
    ThreadMutexLock(&vc->globalPool->mutex);
    linkedListAppend(vc->globalPool->objects, super);
    ThreadMutexUnlock(&vc->globalPool->mutex);
    return so;
}

StackObject* stackObjectNewObject(VmContext *vc, ClassPointer *cp) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_INSTANCE;
    ClassInstance *object = memAlloc(sizeof(ClassInstance));
    object->id = object;
    object->cp = cp;
    object->issuper = false;
    object->nivars = linkedListSize(cp->variables);
    ClassPointer *parent = cp->parent;
    while (parent != NULL) {
        object->nivars += linkedListSize(parent->variables);
        parent = parent->parent;
    }
    object->ivars = memAlloc(sizeof(StackObject*) * object->nivars);
    for (int n = 0; n < object->nivars; n++) {
        object->ivars[n] = stackObjectNewNull(vc);
    }
    so->data.privateData = object;
    ThreadMutexLock(&vc->globalPool->mutex);
    linkedListAppend(vc->globalPool->objects, object);
    ThreadMutexUnlock(&vc->globalPool->mutex);
    vmContextPush(vc, stackObjectClone(vc, so));
    vmContextCallInstanceField(vc, "oncreate");
    return so;
}

StackObject* stackObjectNewString(VmContext *vc, const char *str) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_STRING;
    ThreadMutexLock(&vc->globalPool->mutex);
    so->data.stringIndex = (uint32_t) linkedListSize(vc->globalPool->strings);
    linkedListAppend(vc->globalPool->strings, strbufferClone(str));
    ThreadMutexUnlock(&vc->globalPool->mutex);
    return so;
}

StackObject* stackObjectNewArray(VmContext *vc, LinkedList *array) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_ARRAY;
    ThreadMutexLock(&vc->globalPool->mutex);
    so->data.arrayIndex = (uint32_t) linkedListSize(vc->globalPool->arrays);
    LinkedList *newarray = linkedListCreate();
    LinkedListIterator *it = linkedListCreateIterator(array);
    StackObject *elem = NULL;
    while ((elem = linkedListIteratorNext(it)) != NULL) {
        linkedListAppend(newarray, stackObjectClone(vc, elem));
    }
    linkedListIteratorDestroy(it);
    linkedListAppend(vc->globalPool->arrays, newarray);
    ThreadMutexUnlock(&vc->globalPool->mutex);
    return so;
}

StackObject* stackObjectNewEmptyArray(VmContext *vc) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_ARRAY;
    ThreadMutexLock(&vc->globalPool->mutex);
    so->data.arrayIndex = (uint32_t) linkedListSize(vc->globalPool->arrays);
    linkedListAppend(vc->globalPool->arrays, linkedListCreate());
    ThreadMutexUnlock(&vc->globalPool->mutex);
    return so;
}

StackObject* stackObjectNewInt(VmContext *vc, int64_t i) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_INT;
    so->data.intValue = i;
    return so;
}

StackObject* stackObjectNewFloat(VmContext *vc, double64_t f) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_FLOAT;
    so->data.floatValue = f;
    return so;
}

StackObject* stackObjectNewBool(VmContext *vc, bool b) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_BOOLEAN;
    so->data.booleanValue = b;
    return so;
}

StackObject* stackObjectNewData(VmContext *vc, void* data, uint64_t nbytes) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_DATA;
    StackObjectData *sodata = memAlloc(sizeof(StackObjectData));
    sodata->nbytes = nbytes;
    sodata->data = data;
    so->data.privateData = sodata;
    return so;
}

StackObject* stackObjectNewPrivate(VmContext *vc, void* data) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_PRIVATE;
    so->data.privateData = data;
    return so;
}

StackObject* stackObjectNewNull(VmContext *vc) {
    StackObject *so = memAlloc(sizeof (StackObject));
    so->opcode = OPCODE_NULL;
    //printf("%x\n", so);
    return so;
}
