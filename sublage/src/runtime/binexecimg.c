#include <stdlib.h>
#include <stdio.h>
#include "sublage/mem.h"
#include "sublage/byteorder.h"
#include "sublage/strbuffer.h"
#include "sublage/binexecimg.h"
#include "sublage/loader.h"
#include <string.h>

BinExecImg* binexecimgCreate(BinExecFile *bef) {
    BinExecImg *img = memAlloc(sizeof (BinExecImg));
    img->bef = bef;
    img->importName = NULL;
    img->codeIndex = 0;
    img->externals = NULL;
    img->externalsCount = 0;
    img->natives = NULL;
    img->nativeCount = 0;
    img->variables = memAlloc(sizeof(Variable) * bef->header.numberOfVariables);
    for (int i = 0; i < img->bef->header.numberOfVariables; i++) {
        img->variables[i] = NULL;
    }
    return img;
}

BinExecImg* binexecimgClone(BinExecImg *bei) {
    BinExecImg *img = memAlloc(sizeof (BinExecImg));
    memcpy(img, bei, sizeof(BinExecImg));
    img->codeIndex = 0;
    return img;    
}

void binexecimgDestroy(BinExecImg *img) {
    if (img->externals != NULL) {
        for (int i = 0; i < img->externalsCount; i++) {
            memFree(img->externals[i]);
        }
        memFree(img->externals);
    }
    if (img->natives != NULL) {
        for (int i = 0; i < img->nativeCount; i++) {
            memFree(img->natives[i]);
        }
        memFree(img->natives);
    }
    if (img->variables != NULL) {
        for (int i = 0; i < img->bef->header.numberOfVariables; i++) {
            if (img->variables[i] != NULL) {
                memFree(img->variables[i]);
            }
        }
        memFree(img->variables);
    }
    if (img->importName != NULL) {
        strbufferDestroy(img->importName);
    }
    binexecDestroy(img->bef);
    memFree(img);
}

char* binexecimgGetString(BinExecImg *bei, uint32_t stringIndex) {
    return binexecGetString(bei->bef, stringIndex);
}

LinkedList* binexecimgGetArray(BinExecImg *bei, uint32_t arrayIndex) {
    return binexecGetArray(bei->bef, arrayIndex);
}

StackObject* binexecimgNextStackObject(BinExecImg *img) {
    img->lastCodeIndex = img->codeIndex;
    return loaderNextStackObject(img->bef, &img->codeIndex);
}

StackObject* binexecimgCurrentStackObject(BinExecImg *img) {
    return (StackObject*) (binexecGetCodeStack(img->bef) + img->lastCodeIndex);
}

void binexecimgJumpToOffset(BinExecImg *bef, uint64_t offset) {
    if (offset > binexecGetCodeSize(bef->bef)) {
        fprintf(stderr, "program halted : jumping out of code\n");
        exit(1);
    }
    bef->codeIndex = offset;
}

void binexecimgRelativeJumpToOffset(BinExecImg *bef, int64_t offset) {
    int64_t result = bef->codeIndex + offset;
    if ((result > binexecGetCodeSize(bef->bef)) ||
            (result < 0)) {
        fprintf(stderr, "program halted : jumping out of code\n");
        exit(1);
    }
    bef->codeIndex = result;
}
