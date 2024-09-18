#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include "sublage/strbuffer.h"
#include "sublage/byteorder.h"
#include "sublage/loader.h"
#include "sublage/binexec.h"
#include "sublage/dump.h"

void usage(char *name) {
    printf("usage : %s binary\n", basename(name));
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

    printf("file: %s\n", inputFileName);
    printf("magic: 0x%04x\n", bef->header.magic);
    printf("version: %d.%d\n", bef->header.majorVersion, bef->header.minorVersion);
    uint64_t valueu64 = bef->header.tablesOffset;
    printf("tables offset: 0x%llx (%lld)\n", valueu64, valueu64);
    printf("code size in bytes: 0x%llx (%lld)\n", bef->codeSize, bef->codeSize);
    printf("variables count: %d\n", bef->header.numberOfVariables);
    printf("functions count: %lld\n", linkedListSize(bef->functions));
    printf("imports count: %lld\n", linkedListSize(bef->imports));
    printf("imported functions count: %lld\n", linkedListSize(bef->importedFunctions));
    printf("natives functions count: %lld\n", linkedListSize(bef->nativesFunctions));
    printf("strings count: %lld\n", linkedListSize(bef->strings));
    printf("arrays count: %lld\n", linkedListSize(bef->arrays));
    printf("classes count: %lld\n", linkedListSize(bef->classes));

    if (linkedListSize(bef->imports) > 0) {
        printf("imports:\n");
        LinkedListIterator *it = linkedListCreateIterator(bef->imports);
        char *import = NULL;
        int i = 0;
        while ((import = linkedListIteratorNext(it)) != NULL) {
            printf("\t#%d: %s\n", i++, import);
        }
        linkedListIteratorDestroy(it);
    }
    
    if (linkedListSize(bef->importedFunctions) > 0) {
        printf("imported functions:\n");
        LinkedListIterator *it = linkedListCreateIterator(bef->importedFunctions);
        FunctionPointer *fp = NULL;
        while ((fp = linkedListIteratorNext(it)) != NULL) {
            printf("\t%s : import #%lld\n", fp->name, fp->offset);
        }
        linkedListIteratorDestroy(it);
    }
    
    if (linkedListSize(bef->functions) > 0) {
        printf("functions:\n");
        LinkedListIterator *it = linkedListCreateIterator(bef->functions);
        FunctionPointer *fp = NULL;
        while ((fp = linkedListIteratorNext(it)) != NULL) {
            printf("\t%s : 0x%08llx\n", fp->name, fp->offset);
        }
        linkedListIteratorDestroy(it);
    }

    if (linkedListSize(bef->nativesFunctions) > 0) {
        printf("native functions:\n");
        LinkedListIterator *it = linkedListCreateIterator(bef->nativesFunctions);
        NativePointer *np = NULL;
        while ((np = linkedListIteratorNext(it)) != NULL) {
            printf("\t%s\n", np->name);
        }
        linkedListIteratorDestroy(it);
    }
    
    if (linkedListSize(bef->classes) > 0) {
        printf("classes:\n");
        LinkedListIterator *it = linkedListCreateIterator(bef->classes);
        ClassPointer *cp = NULL;
        int i = 0;
        while ((cp = linkedListIteratorNext(it)) != NULL) {
            if (cp->import == -1) {
                printf("\t#%d : %s (internal)\n", i++, cp->name);
            } else {
                printf("\t#%d : %s (import #%d)\n", i++, cp->name, cp->import);
            }
        }
        linkedListIteratorDestroy(it);
    }
    
    if (linkedListSize(bef->strings) > 0) {
        printf("strings:\n");
        LinkedListIterator *it = linkedListCreateIterator(bef->strings);
        char *str = NULL;
        int i = 0;
        while ((str = linkedListIteratorNext(it)) != NULL) {
            printf("\t#%d : %s\n", i++, str);
        }
        linkedListIteratorDestroy(it);
    }
    
    if (bef->codeSize > 0) {
        printf("code:\n");
        char *fmt = memAlloc(300);
        BinExecImg *img = binexecimgCreate(bef);
        StackObject *so = NULL;
        binexecimgJumpToOffset(img, 0);
        while ((so = binexecimgNextStackObject(img)) != NULL) {
            dumpFormatObject(fmt, 300, so, NULL, false);
            printf("\t0x%08llx : ", img->lastCodeIndex);
            printf("%s", fmt);
            printf("\n");
        }
        memFree(fmt);
        binexecimgDestroy(img);
    }

    //binexecDestroy(bef);

    return 0;
}
