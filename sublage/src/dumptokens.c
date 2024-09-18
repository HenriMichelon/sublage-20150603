#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#ifdef __linux
#include <getopt.h>
#endif
#include "assembler/syntax.h"
#include "assembler/dump.h"
#include "sublage/mem.h"
#include "sublage/strbuffer.h"
#include "sublage/context.h"

void version(char *name) {
    printf("version: %s r%d\n", basename(name), SUBLAGE_REVISION);
}

void usage(char *name) {
    printf("usage: %s [-d -v] input_source_file_name\n", basename(name));
}

FILE *in = NULL;
LexicalContext *lc = NULL;

void cleanupContext() {
    if (lc != NULL) {
        lexicalDone(lc);
    }
    if (in != NULL) {
        fclose(in);
    }
}

int main(int argc, char **argv) {
    const char *optString = "v";

    int optch;
    while ((optch = getopt(argc, argv, optString)) != -1) {
        switch (optch) {
            case 'v':
                version(argv[0]);
                return 0;
            case 'h':
            case '?':
                usage(argv[0]);
                return 2;
        }
    }
    if ((argc - optind) <= 0) {
        usage(argv[0]);
        return 2;
    }

    const char *inputFileName = argv[optind];

    in = fopen(inputFileName, "r");
    if (in == NULL) {
        fprintf(stderr, "Error opening %s.\n", inputFileName);
        return 1;
    }

    lc = lexicalInit(in, inputFileName);
    atexit(cleanupContext);
    LexicalToken lt;
    do {
        lexicalNextToken(lc, &lt);
        dumpLexicalToken(stdout, &lt);
        printf("\n");
    } while (lt.symbol != LEXICALSYMBOL_EOF);
   
    return 0;
}
