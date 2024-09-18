#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include "sublage/strbuffer.h"
#include "sublage/byteorder.h"
#include "sublage/loader.h"
#include "sublage/binexec.h"

void usage(char *name) {
    printf("usage : %s binary\n", basename(name));
}

const char* LexicalSymbolTextual[] = {
 "Unkown", "Decimal", "Internal", "ReservedWord", "Identifier",
 "StartFunc", "EndFunc", "String", "Ref", "VarSet",
 "StartArray", "EndArray", "EOF"
};


int main(int argc, char **argv) {
    if (argc != 2) {
        usage(argv[0]);
        return 0;
    }
    char *inputFileName = argv[1];
    
    BinExecFile *bef = loaderLoadFileFromFileName(inputFileName);
    if (bef == NULL) {
        fprintf(stderr, "Error opening %s: %s.\n", inputFileName,
                loaderGetErrorMessage());
        return 1;
    }
    debugLoadFile(bef);
    if (bef->sourceFileName == NULL) {
        fprintf(stderr, "No debug file found for %s.\n", inputFileName);
        return 2;
    }
    
    printf("source file name: %s\n", bef->sourceFileName);    
    printf("%d variable(s):\n", bef->header.numberOfVariables);
    LinkedListIterator *it = linkedListCreateIterator(bef->variablesName);
    char* string = NULL;
    while ((string = linkedListIteratorNext(it)) != NULL) {
        printf("\t%s\n", string);
    }
    linkedListIteratorDestroy(it);
    printf("%lld symbols:\n", bef->debugSymbolsCount);
    for (uint64_t i = 0; i < bef->debugSymbolsCount; i++) {
        DebugSymbol *ds = &bef->debugSymbols[i];
        printf("\t%lld: 0x%08llx, %s `%s`, line %lld, start %lld, end %lld\n", i+1,
               ds->codeOffset,
               LexicalSymbolTextual[ds->symbol],
               debugGetText(bef, ds->startingChar, ds->endingChar),
               ds->lineNumber, ds->startingChar, ds->endingChar);
    }
    //binexecDestroy(bef);
    
    return 0;
}
