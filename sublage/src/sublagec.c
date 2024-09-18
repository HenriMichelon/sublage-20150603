#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>
#include <unistd.h>
#ifdef __linux
#include <getopt.h>
#endif
#include "assembler/syntax.h"
#include "assembler/dump.h"
#include "assembler/binexec.h"
#include "sublage/mem.h"
#include "sublage/strbuffer.h"
#include "sublage/context.h"

void version(char *name) {
    printf("version: %s r%d\n", basename(name), SUBLAGE_REVISION);
}

void usage(char *name) {
    printf("usage: %s [-d -v] input_source_file_name output_binary_file_name\n", basename(name));
}

FILE *in = NULL;
FILE *out = NULL;
FILE *debug = NULL;
LexicalContext *lc = NULL;
SyntaxContext *sc = NULL;
char *outputFileName;

void cleanupContext() {
    if (lc != NULL) {
        lexicalDone(lc);
    }
    if (sc != NULL) {
        syntaxDone(sc);
    }
    if (out != NULL) {
        fclose(out);
        remove(outputFileName);
    }
    if (in != NULL) {
        fclose(in);
    }
    if (debug != NULL) {
        fclose(debug);
    }
}

int main(int argc, char **argv) {
    const char *optString = "dv";
    bool debugon = false;

    int optch;
    while ((optch = getopt(argc, argv, optString)) != -1) {
        switch (optch) {
            case 'd':
                debugon = true;
                break;
            case 'v':
                version(argv[0]);
                return 0;
            case 'h':
            case '?':
                usage(argv[0]);
                return 2;
        }
    }
    if ((argc - optind) <= 1) {
        usage(argv[0]);
        return 2;
    }

    const char *inputFileName = argv[optind];
    char *outputFileName = argv[optind + 1];

    in = fopen(inputFileName, "r");
    if (in == NULL) {
        fprintf(stderr, "Error opening %s.\n", inputFileName);
        return 1;
    }
    out = fopen(outputFileName, "wb");
    if (out == NULL) {
        fclose(in);
        fprintf(stderr, "Error opening %s.\n", outputFileName);
        return 1;
    }
    if (debugon) {
        char *debugFileName = strbufferClone(outputFileName);
        debugFileName = strbufferAppendStr(debugFileName, ".debug", -1);
        debug = fopen(debugFileName, "wb");
        strbufferDestroy(debugFileName);
        if (debug == NULL) {
            fclose(in);
            fclose(out);
            fprintf(stderr, "Error opening %s.\n", debugFileName);
            return 1;
        }
    }

    lc = lexicalInit(in, inputFileName);
    atexit(cleanupContext);
    sc = syntaxInit(lc);

    syntaxAnalyse(sc);
    binexecWriteHeader(out, debug, sc);
    if (!binexecWriteContent(out, debug, sc)) {
        fprintf(stderr, "Error linking %s\n", outputFileName);
        return 1;
    }

    return 0;
}
