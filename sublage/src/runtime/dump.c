#include "sublage/thread.h"
#include "sublage/dump.h"
#include "sublage/strbuffer.h"
#include "sublage/internalsIdentifiers.h"

#ifdef WIN32
static ThreadMutex dump_mutex = NULL;
#else
static ThreadMutex dump_mutex;
#endif

void dumpStart() {
    ThreadMutexCreate(&dump_mutex);
}

void dumpEnd() {
#ifdef WIN32
    if (dump_mutex != NULL) 
#endif
    {
        ThreadMutexDestroy(&dump_mutex);
    }
}

char* dumpFindInternalFunctionNameByOffset(VmContext *vc, uint64_t offset) {
    BinExecFile *bef = vmContextGetCurrentImage(vc)->bef;
    LinkedListIterator *it_fp = linkedListCreateIterator(binexecGetFunctions(bef));
    FunctionPointer *fp = NULL;
    while ((fp = linkedListIteratorNext(it_fp)) != NULL) {
        if (fp->offset == offset) {
            break;
        }
    }
    linkedListIteratorDestroy(it_fp);
    if (fp != NULL) {
        return fp->name;
    }
    return NULL;
}

char* dumpFindExternalFunctionNameByIndex(VmContext *vc, uint32_t index) {
    BinExecFile *bef = vmContextGetCurrentImage(vc)->bef;
    LinkedListIterator *it_fp = linkedListCreateIterator(binexecGetImportedFunctions(bef));
    FunctionPointer *fp = NULL;
    int i = 0;
    while ((fp = linkedListIteratorNext(it_fp)) != NULL) {
        if (i++ == index) {
            linkedListIteratorDestroy(it_fp);
            return fp->name;
        }
    }
    linkedListIteratorDestroy(it_fp);
    return "?";
}

char* dumpFindNativeFunctionNameByIndex(VmContext *vc, uint32_t index) {
    BinExecFile *bef = vmContextGetCurrentImage(vc)->bef;
    LinkedListIterator *it_fp = linkedListCreateIterator(binexecGetNativeFunctions(bef));
    NativePointer *np = NULL;
    int i = 0;
    while ((np = linkedListIteratorNext(it_fp)) != NULL) {
        if (i++ == index) {
            linkedListIteratorDestroy(it_fp);
            return np->name;
        }
    }
    linkedListIteratorDestroy(it_fp);
    return "?";
}

void dumpFormatObject(char* buf, size_t bufsize, StackObject *so, VmContext *vc,
                      bool stringfrompool) {
    const char* defaultfmt = "%s";
    switch (so->opcode) {
        case OPCODE_NULL:
            snprintf(buf, bufsize, defaultfmt, "null");
            break;
        case OPCODE_RETURN:
            snprintf(buf, bufsize, defaultfmt, "ret");
            break;
        case OPCODE_INTERNAL:
            snprintf(buf, bufsize, defaultfmt, internalsIdentifiers[so->data.internal].mnemonic);
            break;
        case OPCODE_INT:
            snprintf(buf, bufsize, "%lld", so->data.intValue);
            break;
        case OPCODE_FLOAT:
            snprintf(buf, bufsize, "%f", so->data.floatValue);
            break;
        case OPCODE_STRING:
            if (vc == NULL) {
                snprintf(buf, bufsize, "\"String #%d\"", so->data.stringIndex);
            } else {
                char *str;
                if (stringfrompool) {
                    str = vmContextGetString(vc, so);
                } else {
                    str = binexecimgGetString(vc->images[vc->imagesIndex],
                                              so->data.stringIndex);
                }
                snprintf(buf, bufsize, "\"%s\"", str);
            }
            break;
        case OPCODE_ARRAY:
        {
            if (vc != NULL) {
                snprintf(buf, bufsize, "[");
                int offset = 1;
                LinkedList *array = vmContextGetArray(vc, so);
                if (array != NULL) {
                    LinkedListIterator *it = linkedListCreateIterator(array);
                    StackObject *so;
                    char buf2[bufsize];
                    while (((so = linkedListIteratorNext(it)) != NULL) &&
                           (offset < (bufsize-1))) {
                        dumpFormatObject(buf2, bufsize, so, vc, stringfrompool);
                        if (offset > 1) {
                            offset += snprintf(buf + offset, bufsize - offset,
                                    ",");
                        }
                        if (offset >= bufsize) {
                            break;
                        }
                        offset += snprintf(buf + offset, bufsize - offset,
                                "%s", buf2);
                    }
                }
                if ((offset < (bufsize-1))) {
                    snprintf(buf + offset, bufsize - offset, "]");
                }
            } else {
                snprintf(buf, bufsize, "[#%d]", so->data.arrayIndex);
            }

            break;
        }
        case OPCODE_BOOLEAN:
            snprintf(buf, bufsize, defaultfmt, (so->data.booleanValue ? "true" : "false"));
            break;
        case OPCODE_INTERNALCALL:
            if (vc == NULL) {
                snprintf(buf, bufsize, "InternalCall 0x%04llx", so->data.functionOffset);
            } else {
                snprintf(buf, bufsize, "%s",
                        dumpFindInternalFunctionNameByOffset(vc, so->data.functionOffset));
            }
            break;
        case OPCODE_EXTERNALCALL:
            if (vc == NULL) {
                snprintf(buf, bufsize, "ExternalCall #%d", so->data.importIndex);
            } else {
                snprintf(buf, bufsize, "%s",
                        dumpFindExternalFunctionNameByIndex(vc, so->data.importIndex));
            }
            break;
        case OPCODE_NATIVECALL:
            if (vc == NULL) {
                snprintf(buf, bufsize, "NativeCall 0x%04x", so->data.nativeIndex);
            } else {
                snprintf(buf, bufsize, "%s",
                        dumpFindNativeFunctionNameByIndex(vc, so->data.nativeIndex));
            }
            break;
        case OPCODE_JUMP:
            snprintf(buf, bufsize, "jump %lld", so->data.jumpOffset);
            break;
        case OPCODE_JUMPIFNOT:
            snprintf(buf, bufsize, "jumpifnot %lld", so->data.jumpOffset);
            break;
        case OPCODE_JUMPIF:
            snprintf(buf, bufsize, "jumpif %lld", so->data.jumpOffset);
            break;
        case OPCODE_INTERNALCALL_REF:
            if (vc == NULL) {
                snprintf(buf, bufsize, "@InternalCall 0x%04llx", so->data.functionOffset);
            } else {
                snprintf(buf, bufsize, "@%s",
                        dumpFindInternalFunctionNameByOffset(vc, so->data.functionOffset));
            }
            break;
        case OPCODE_EXTERNALCALL_REF:
            if (vc == NULL) {
                snprintf(buf, bufsize, "@ExternalCall #%d", so->data.importIndex);
            } else {
                snprintf(buf, bufsize, "@%s",
                        dumpFindExternalFunctionNameByIndex(vc, so->data.importIndex));
            }
            break;
        case OPCODE_NATIVECALL_REF:
            if (vc == NULL) {
                snprintf(buf, bufsize, "@NativeCall 0x%04x", so->data.nativeIndex);
            } else {
                snprintf(buf, bufsize, "@%s",
                        dumpFindNativeFunctionNameByIndex(vc, so->data.nativeIndex));
            }
            break;        
        case OPCODE_PRIVATE:
            snprintf(buf, bufsize, "private data 0x%04llx", so->data.privateData);
            break;
        case OPCODE_DATA:
        {
            StackObjectData *data = so->data.privateData;
            snprintf(buf, bufsize, "binary datas 0x%04llx, %lld bytes",
                    data->data, data->nbytes);
            break;
        }
        case OPCODE_VAR_SET:
            snprintf(buf, bufsize, "var set #%d ", so->data.variableIndex);
            break;
        case OPCODE_VAR_GET:
            snprintf(buf, bufsize, "var set #%d ", so->data.variableIndex);
            break;
        case OPCODE_VAR_REF:
            snprintf(buf, bufsize, "@var @%d ", so->data.variableIndex);
            break;
        case OPCODE_CLASSREF:
        {
            if (vc == NULL) {
                snprintf(buf, bufsize, "@Class #%d", so->data.classIndex);
            } else {
                BinExecImg *img = vmContextGetCurrentImage(vc);
                ClassPointer *cp = linkedListGet(img->bef->classes, so->data.classIndex);
                snprintf(buf, bufsize, "@%s", cp->name);
            }
        }
            break;
        case OPCODE_INSTANCE:
        {
            ClassInstance *object = so->data.privateData;
            snprintf(buf, bufsize, "%s[0x%04llx]", object->cp->name, object->id);
            break;
        }
        case OPCODE_IFIELDCALL:
        {
            if (vc == NULL) {
                snprintf(buf, bufsize, ".#%d", so->data.stringIndex);
            } else {
                snprintf(buf, bufsize, ".%s", binexecimgGetString(vc->images[vc->imagesIndex],
                                                                 so->data.stringIndex));
            }
            break;
        }
        case OPCODE_IFIELD_SET:
        {
            if (vc == NULL) {
                snprintf(buf, bufsize, "->.#%d", so->data.stringIndex);
            } else {
                snprintf(buf, bufsize, "->.%s", binexecimgGetString(vc->images[vc->imagesIndex],
                                                              so->data.stringIndex));
            }
            break;
        }
        default:
            snprintf(buf, bufsize, "??? (0x%x)", so->opcode);
            break;
    }
}

void dumpObject(FILE *f, StackObject *so, VmContext *vc, bool stringfrompool) {
    char *buf = memAlloc(120);
    dumpFormatObject(buf, 120, so, vc, stringfrompool);
    fprintf(f, "%s", buf);
    strbufferDestroy(buf);
}

void dumpInternal(FILE *f, StackObject *so, VmContext *vc, bool stringfrompool) {
    fprintf(f, "-> ");
    dumpObject(f, so, vc, stringfrompool);
    fprintf(f, "\n");
}

void dumpCall(FILE *f, StackObject *so, VmContext *vc, bool stringfrompool) {
    if (so->opcode == OPCODE_INTERNALCALL) {
        fprintf(f, "-> int call to ");
    } else if (so->opcode == OPCODE_EXTERNALCALL) {
        fprintf(f, "-> ext call to ");
    } else {
        fprintf(f, "-> nat call to ");
    }
    dumpObject(f, so, vc, stringfrompool);
    fprintf(f, "\n");
}

void dumpJump(FILE *f, StackObject *so, VmContext *vc, bool stringfrompool) {
    if (so->opcode == OPCODE_JUMP) {
        fprintf(f, "-> relative jump to ");
    } else if (so->opcode == OPCODE_JUMPIFNOT) {
        fprintf(f, "-> relative jumpifnot to ");
    } else if (so->opcode == OPCODE_JUMPIF) {
        fprintf(f, "-> relative jumpif to ");
    }
    dumpObject(f, so, vc, stringfrompool);
    fprintf(f, "\n");
}

void dumpStack(FILE *f, VmContext *vc) {
    LinkedListIterator *lli = linkedListCreateIterator(vmContextGetStack(vc));
    StackObject *so = NULL;
    uint32_t i = (uint32_t) linkedListSize(vmContextGetStack(vc));
    while ((so = linkedListIteratorNext(lli)) != NULL) {
        fprintf(f, "%4d:", i--);
        dumpObject(f, so, vc, true);
        fprintf(f, "\n");
    }
    linkedListIteratorDestroy(lli);
}

void dumpContext(VmContext *vc) {
    ThreadMutexLock(&dump_mutex);
    fprintf(vc->stackDumpFile, "-----------------------------------------\n");
    dumpStack(vc->stackDumpFile, vc);
    fprintf(vc->stackDumpFile, "-----------------------------------------\n");
    fflush(vc->stackDumpFile);
    ThreadMutexUnlock(&dump_mutex);
}

void dumpStackObject(StackObject *so, VmContext *vc) {
    ThreadMutexLock(&dump_mutex);
    fprintf(vc->stackDumpFile, "\nThread #%d: ", vc->contextID);
    dumpObject(vc->stackDumpFile, so, vc, false);
    fprintf(vc->stackDumpFile, "\n");
    fflush(vc->stackDumpFile);
    ThreadMutexUnlock(&dump_mutex);
}
