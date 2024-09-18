#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <string.h>
#include "sublage/strbuffer.h"
#include "sublage/loader.h"
#include "sublage/binexec.h"
#include "sublage/internalsIdentifiers.h"

void usage(char *name) {
    printf("usage: %s binary_file_name\n", basename(name));
}

LinkedList *importsAliases;

char* findImportByIndex(uint32_t index) {
    LinkedListIterator *it_fp = linkedListCreateIterator(importsAliases);
    char* import = NULL;
    int i = 0;
    while ((import = linkedListIteratorNext(it_fp)) != NULL) {
        if (i++ == index) {
            linkedListIteratorDestroy(it_fp);
            return import;
        }
    }
    linkedListIteratorDestroy(it_fp);
    return "?";
}

char* findInternalFunctionNameByOffset(uint64_t offset, BinExecFile *bef) {
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

ClassPointer* findClassByOffset(uint64_t offset, BinExecFile *bef) {
    LinkedListIterator *it = linkedListCreateIterator(bef->classes);
    ClassPointer *c = NULL;
    while ((c = linkedListIteratorNext(it)) != NULL) {
        LinkedListIterator *it_fp = linkedListCreateIterator(c->functions);
        FunctionPointer *fp = NULL;
        while ((fp = linkedListIteratorNext(it_fp)) != NULL) {
            if (fp->offset == offset) {
                break;
            }
        }
        linkedListIteratorDestroy(it_fp);
        if (fp != NULL) {
            return c;
        }
    }
    return NULL;
}

ClassPointer* findClassByIndex(uint32_t index, BinExecFile *bef) {
    return linkedListGet(bef->classes, index);
}

char* findClassMethodNameByOffset(uint64_t offset, BinExecFile *bef) {
    LinkedListIterator *it = linkedListCreateIterator(bef->classes);
    ClassPointer *c = NULL;
    while ((c = linkedListIteratorNext(it)) != NULL) {
        if (c->import != -1) { continue; }
        LinkedListIterator *it_fp = linkedListCreateIterator(c->functions);
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
    }
    return NULL;
}

char* findExternalFunctionNameByIndex(uint32_t index, BinExecFile *bef) {
    LinkedListIterator *it_fp = linkedListCreateIterator(binexecGetImportedFunctions(bef));
    FunctionPointer *fp = NULL;
    int i = 0;
    while ((fp = linkedListIteratorNext(it_fp)) != NULL) {
        if (i++ == index) {
            linkedListIteratorDestroy(it_fp);
            char* completeName = strbufferCreate();
            completeName = strbufferAppendStr(completeName, findImportByIndex((uint32_t)fp->offset), -1);
            completeName = strbufferAppendChar(completeName, ':');
            completeName = strbufferAppendStr(completeName, fp->name, -1);
            return completeName;
        }
    }
    linkedListIteratorDestroy(it_fp);
    return "?";
}

char* findNativeFunctionNameByIndex(uint32_t index, BinExecFile *bef) {
    LinkedListIterator *it_fp = linkedListCreateIterator(binexecGetNativeFunctions(bef));
    NativePointer *np = NULL;
    int i = 0;
    while ((np = linkedListIteratorNext(it_fp)) != NULL) {
        if (i++ == index) {
            linkedListIteratorDestroy(it_fp);
            char* completeName = strbufferCreate();
            if (np->import != -1) {
                completeName = strbufferAppendStr(completeName, findImportByIndex(np->import), -1);
                completeName = strbufferAppendChar(completeName, ':');
            }
            completeName = strbufferAppendStr(completeName, np->name, -1);
            return completeName;
        }
    }
    linkedListIteratorDestroy(it_fp);
    return "?";
}

void printStackObject(BinExecFile *bef, StackObject *so) {
    switch (so->opcode) {
        case OPCODE_NULL:
            printf("null");
            break;
        case OPCODE_INT:
            printf("%lld", so->data.intValue);
            break;
        case OPCODE_FLOAT:
            printf("%f", so->data.floatValue);
            break;
        case OPCODE_BOOLEAN:
            printf("%s", (so->data.booleanValue ? "true" : "false"));
            break;
        case OPCODE_STRING:
            printf("\"%s\"", binexecGetString(bef, so->data.stringIndex));
            break;
        case OPCODE_ARRAY:
        {
            LinkedList *array = binexecGetArray(bef, so->data.arrayIndex);
            printf("[ ");
            if (array != NULL) {
                LinkedListIterator *it = linkedListCreateIterator(array);
                StackObject *s = NULL;
                while ((s = linkedListIteratorNext(it)) != NULL) {
                    printStackObject(bef, s);
                    printf(" ");
                }
            } else {
                printf("--- UNKNONW ARRAY INDEX");
            }
            printf(" ]");
        }
            break;
        case OPCODE_RETURN:
            printf("%s", "ret");
            return;
        case OPCODE_INTERNAL:
            printf("%s", internalsIdentifiers[so->data.internal].mnemonic);
            break;
        case OPCODE_INTERNALCALL:
            printf("%s", findInternalFunctionNameByOffset(so->data.functionOffset, bef));
            break;
        case OPCODE_EXTERNALCALL:
        {
            char* name = findExternalFunctionNameByIndex(so->data.importIndex, bef);
            if (name != NULL) {
                printf("%s", name);
                strbufferDestroy(name);
            } else {
                printf("--- UNKNOWN EXTERNAL FUNCTION INDEX");
            }
            break;
        }
        case OPCODE_NATIVECALL:
        {
            char* name = findNativeFunctionNameByIndex(so->data.nativeIndex, bef);
            if (name != NULL) {
                printf("%s", name);
                strbufferDestroy(name);
            } else {
                printf("--- UNKNOWN NATIVE FUNCTION INDEX");
            }
            break;
        }
        case OPCODE_JUMP:
            printf("jump(%lld)", so->data.jumpOffset);
            break;
        case OPCODE_JUMPIFNOT:
            printf("jumpIfNot(%lld)", so->data.jumpOffset);
            break;
        case OPCODE_JUMPIF:
            printf("jumpIf(%lld)", so->data.jumpOffset);
            break;
        case OPCODE_NATIVECALL_REF:
        {
            char* name =  findNativeFunctionNameByIndex(so->data.nativeIndex, bef);
            if (name != NULL) {
                printf("@%s", name);
                strbufferDestroy(name);
            } else {
                printf("--- UNKNOWN NATIVE FUNCTION INDEX");
            }
            break;
        }
        case OPCODE_INTERNALCALL_REF:
            printf("@%s", findInternalFunctionNameByOffset(so->data.functionOffset, bef));
            break;
        case OPCODE_EXTERNALCALL_REF:
        {
            char* name = findExternalFunctionNameByIndex(so->data.importIndex, bef);
            if (name != NULL) {
                printf("@%s", name);
                strbufferDestroy(name);
            } else {
                printf("--- UNKNOWN EXTERNAL FUNCTION INDEX");
            }
            break;
        }
        case OPCODE_VAR_GET:
            printf("var%d", so->data.variableIndex);
            break;
        case OPCODE_VAR_REF:
            printf("@var%d", so->data.variableIndex);
            break;
        case OPCODE_VAR_SET:
            printf("->var%d", so->data.variableIndex);
            break;
        case OPCODE_CLASSREF:
        {
            ClassPointer *c = findClassByIndex(so->data.classIndex, bef);
            if (c != NULL) {
                char* completeName = strbufferCreate();
                if (c->import != -1) {
                    completeName = strbufferAppendStr(completeName, findImportByIndex(c->import), -1);
                    completeName = strbufferAppendChar(completeName, ':');
                }
                completeName = strbufferAppendStr(completeName, c->name, -1);
                printf("@%s", completeName);
                strbufferDestroy(completeName);
            } else {
                printf("--- UNKNOWN CLASS INDEX");
            }
            break;
        }
        case OPCODE_IFIELD_SET:
            printf("->.%s", binexecGetString(bef, so->data.stringIndex));
            break;
        case OPCODE_IFIELDCALL:
            printf(".%s", binexecGetString(bef, so->data.stringIndex));
            break;
        case OPCODE_IFIELDCALL_REF:
            printf("@.%s", binexecGetString(bef, so->data.stringIndex));
            break;
        default:
            printf("--- UNKNOWN OPCODE");
            break;
    }    
}

void printFunction(FunctionPointer *fp, BinExecFile *bef, int tab) {
    uint64_t index = fp->offset;
    StackObject *so = NULL;
    while ((so = loaderNextStackObject(bef, &index)) != NULL) {
        if ((findInternalFunctionNameByOffset(index, bef) != NULL) ||
            (findClassMethodNameByOffset(index, bef)) ||
            (index >= bef->codeSize)) {
            break;
        }
        for (int i = 0; i < tab; i++) {
            printf("    ");
        }
        printStackObject(bef, so);
        printf("\n");
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return 0;
    }
    char *inputFileName = strbufferClone(argv[1]);

    BinExecFile *bef = loaderLoadFileFromFileName(inputFileName);
    if (bef == NULL) {
        fprintf(stderr, "Error opening %s: %s.\n", inputFileName,
                loaderGetErrorMessage());
        return 1;
    }
    VmContext *vc = vmContextCreate(NULL, NULL, NULL, 0);

    importsAliases = linkedListCreate();
    LinkedListIterator *it_import = linkedListCreateIterator(binexecGetImports(bef));
    char *import;
    while ((import = linkedListIteratorNext(it_import)) != NULL) {
        printf("import %s\n", import);
        linkedListAppend(importsAliases, import);
    }
    linkedListIteratorDestroy(it_import);
    printf("\n");
    
    LinkedListIterator *it_nf = linkedListCreateIterator(binexecGetNativeFunctions(bef));
    NativePointer *np;
    while ((np = linkedListIteratorNext(it_nf)) != NULL) {
        if (np->import == -1) {
            printf("native %s\n", np->name);
        }
    }
    linkedListIteratorDestroy(it_nf);
    
    for (uint32_t nvar = 0; nvar < bef->header.numberOfVariables; nvar++) {
        printf("var var%d\n", nvar);
    }

    LinkedListIterator *it_fp = linkedListCreateIterator(binexecGetFunctions(bef));
    FunctionPointer *fp;
    while ((fp = linkedListIteratorNext(it_fp)) != NULL) {
        printf("\n%s <<\n", fp->name);
        printFunction(fp, bef, 1);
        printf(">>\n");
    }
    linkedListIteratorDestroy(it_fp);
    
    LinkedListIterator *it_c = linkedListCreateIterator(bef->classes);
    ClassPointer *cp = NULL;
    while ((cp = linkedListIteratorNext(it_c)) != NULL) {
        if (cp->import != -1) {
            continue;
        }
        printf("\nclass %s ", cp->name);
        if (strlen(cp->parentClassName) > 0) {
            printf("extends %s ", cp->parentClassName);
        }
        printf("<<\n");
        it_fp = linkedListCreateIterator(cp->variables);
        InstanceVariablePointer *ivar;
        while ((ivar = linkedListIteratorNext(it_fp)) != NULL) {
            printf("    var %s", ivar->name);
            if (ivar->readAccess) {
                printf(" read");
                if (strlen(ivar->readMethod) > 0) {
                    printf(" %s", ivar->readMethod);
                }
            }
            if (ivar->writeAccess) {
                printf(" write");
                if (strlen(ivar->writeMethod) > 0) {
                    printf(" %s", ivar->writeMethod);
                }
            }
            printf("\n");
        }
        linkedListIteratorDestroy(it_fp);
        it_fp = linkedListCreateIterator(cp->functions);
        InstanceFunctionPointer *fp;
        while ((fp = linkedListIteratorNext(it_fp)) != NULL) {
            if (fp->private) {
                printf("\n    (%s) <<\n", fp->name);                
            } else {
                printf("\n    %s <<\n", fp->name);
            }
            printFunction((FunctionPointer*)fp, bef, 2);
            printf("    >>\n");
        }
        linkedListIteratorDestroy(it_fp);
        printf(">>\n");
    }
    linkedListIteratorDestroy(it_c);

    linkedListDestroy(importsAliases, false);
    binexecDestroy(bef);
    vmContextDestroy(vc);

    return 0;
}
