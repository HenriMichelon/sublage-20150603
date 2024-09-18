#include <string.h>
//#include <float.h>
#include <libgen.h>
#include "sublage/types.h"
#include "sublage/mem.h"
#include "sublage/byteorder.h"
#include "sublage/strbuffer.h"
#include "sublage/stackobject.h"
#include "import.h"
#include "binexec.h"
#include "errors.h"
#include "dump.h"
#include "classes.h"

typedef struct {
    char *name;
    int32_t importIndex;
    uint32_t functionIndex;
    LinkedList *calls;
} NativeFunction;

typedef struct {
    char *name;
    uint32_t importIndex;
    uint32_t functionIndex;
    LinkedList *calls;
} ImportedFunction;

uint64_t debugSymbolCountOffset;
uint64_t debugSymbolCount;

bool binexecWriteToken(FILE *out, FILE *debug, LinkedList *code, SyntaxContext *sc,
    LinkedList *importedFunctions, LinkedList *nativeFunctions,
    LexicalToken *lt, uint64_t startOffset,
    LinkedList *breakList, LinkedList *continueList);

bool binexecWriteHeader(FILE *out, FILE *debug, SyntaxContext *sc) {
    BinExecHeader header;
    header.magic = htovms(BINEXEC_MAGIC);
    header.majorVersion = 0;
    header.minorVersion = 0;
    header.tablesOffset = 0;
    header.numberOfVariables = (uint32_t) linkedListSize(sc->variables);
    if (debug != NULL) {
        uint16_t magic = htovms(0xED0C);
        if (fwrite(&magic, sizeof (magic), 1, debug) != 1) {
            return false;
        }
        char* fileName = sc->lexicalContext->fileName;
        if (fwrite(fileName, strlen(fileName) + 1, 1, debug) != 1) {
            return false;
        }
        LinkedListIterator *it = linkedListCreateIterator(sc->variables);
        char *variable;
        while ((variable = linkedListIteratorNext(it)) != NULL) {
            if (fwrite(variable, strlen(variable) + 1, 1, debug) != 1) {
                return false;
            }
        }
        linkedListIteratorDestroy(it);
        debugSymbolCountOffset = ftell(debug);
        if (fwrite(&debugSymbolCount, sizeof (debugSymbolCount), 1, debug) != 1) {
            return false;
        }
    }
    return fwrite(&header, sizeof (BinExecHeader), 1, out) == 1;
}

bool binexecWriteBoolean(FILE *out, bool b) {
    uint8_t type = OPCODE_BOOLEAN;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint8_t i = (b ? 1 : 0);
    return fwrite(&i, sizeof (i), 1, out) == 1;
}

bool binexecWriteNull(FILE *out) {
    uint8_t type = OPCODE_NULL;
    return fwrite(&type, sizeof (type), 1, out) == 1;
}

bool binexecWriteReturn(FILE *out) {
    uint8_t type = OPCODE_RETURN;
    return fwrite(&type, sizeof (type), 1, out) == 1;
}

bool binexecWriteString(FILE *out, char *string, LinkedList *strings) {
    uint8_t type = OPCODE_STRING;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint32_t stringIndex = (uint32_t) linkedListSize(strings);
    // TODO : doublons
    linkedListAppend(strings, string);
    stringIndex = htovml(stringIndex);
    return fwrite(&stringIndex, sizeof (stringIndex), 1, out) == 1;
}

bool binexecWriteArray(FILE *out, LinkedList *array, LinkedList *arrays) {
    uint8_t type = OPCODE_ARRAY;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint32_t arrayIndex = (uint32_t) linkedListSize(arrays);
    linkedListAppend(arrays, array);
    arrayIndex = htovml(arrayIndex);
    return fwrite(&arrayIndex, sizeof (arrayIndex), 1, out) == 1;
}

bool binexecWriteInt(FILE *out, int64_t i) {
    uint8_t type = OPCODE_INT;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    i = htovmll(i);
    return fwrite(&i, sizeof (i), 1, out) == 1;
}

bool binexecWriteFloat(FILE *out, double64_t d) {
    uint8_t type = OPCODE_FLOAT;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    d = htovmd(d);
    return fwrite(&d, sizeof (d), 1, out) == 1;
}

bool binexecWriteInternal(FILE *out, Internal opcode) {
    uint8_t type = OPCODE_INTERNAL;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint8_t opcode_byte = (uint8_t) opcode;
    return fwrite(&opcode_byte, sizeof (opcode_byte), 1, out) == 1;
}

bool binexecUpdateOffsets(FILE *out, LinkedList *list, int64_t dest) {
    uint64_t current = ftell(out);
    LinkedListIterator *it = linkedListCreateIterator(list);
    uint64_t *address = NULL;
    while ((address = linkedListIteratorNext(it)) != NULL) {
        if (fseek(out, *address, SEEK_SET) != -1) {
            int64_t offset = dest - *address - sizeof (uint64_t);
            offset = htovmll(offset);
            fwrite(&offset, sizeof (offset), 1, out);
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(list, true);
    return fseek(out, current, SEEK_SET) != -1;
}

bool binexecWriteForeach(FILE *out, FILE *debug, LinkedList *code, SyntaxContext *sc,
    LinkedList *importedFunctions, LinkedList *nativeFunctions,
    LexicalToken *lt, uint64_t startOffset) {

    // `explode`array (DebugSymbol : `foreach`)
    LexicalToken token;
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_EXPLODE;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }

    int64_t foreach_address = ftell(out);
    // test bloc
    token.symbol = LEXICALSYMBOL_DECIMAL;
    token.data.intValue = 1;
    token.options.isDecimalFloat = false;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_SUB;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_DUP;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_DECIMAL;
    token.data.intValue = 0;
    token.options.isDecimalFloat = false;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_GE;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }

    // `jumpIfNot` after test bloc
    int64_t currentTokenPos = linkedListIndexOf(code, lt);
    if (currentTokenPos == -1) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    LexicalToken *nextToken = linkedListGet(code, currentTokenPos + 1);
    if (nextToken == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    binexecWriteDebugSymbol(out, debug, nextToken, startOffset);
    uint8_t type = OPCODE_JUMPIFNOT;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint64_t jumpifnot_address = ftell(out);
    uint64_t dummy = 0;
    if (fwrite(&dummy, sizeof (dummy), 1, out) != 1) {
        return false;
    }

    // get current element
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_DUP;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_DECIMAL;
    token.data.intValue = 2;
    token.options.isDecimalFloat = false;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_ADD;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_ROLL;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }

    // code bloc
    LinkedList *breakList = linkedListCreate();
    LinkedList *continueList = linkedListCreate();
    LinkedListIterator *it = linkedListCreateIterator(lt->options.code.code);
    LexicalToken *tok = NULL;
    while ((tok = linkedListIteratorNext(it)) != NULL) {
        if (!binexecWriteToken(out, debug, code, sc, importedFunctions,
            nativeFunctions, tok, startOffset,
            breakList, continueList)) {
            linkedListIteratorDestroy(it);
            linkedListDestroy(breakList, true);
            linkedListDestroy(continueList, true);
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(lt->options.code.code, true);

    // `jump to` at end of for
    nextToken = linkedListGet(code, currentTokenPos + 2);
    if (nextToken == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    binexecWriteDebugSymbol(out, debug, nextToken, startOffset);
    type = OPCODE_JUMP;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }
    int64_t jump_offset = foreach_address - sizeof (foreach_address) - ftell(out);
    jump_offset = htovmll(jump_offset);
    if (fwrite(&jump_offset, sizeof (jump_offset), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }

    // update jumpIf
    uint64_t afterforeach_address = ftell(out);
    int64_t jumpif_offset = ftell(out) - jumpifnot_address - sizeof (jumpifnot_address);
    fseek(out, jumpifnot_address, SEEK_SET);
    jumpif_offset = htovmll(jumpif_offset);
    if (fwrite(&jumpif_offset, sizeof (jumpif_offset), 1, out) != 1) {
        return false;
    }
    fseek(out, afterforeach_address, SEEK_SET);

    // drop index
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_DROP;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }


    // update breaks & continues
    binexecUpdateOffsets(out, breakList, afterforeach_address);
    binexecUpdateOffsets(out, continueList, foreach_address);
    return true;
}

bool binexecWriteFor(FILE *out, FILE *debug, LinkedList *code, SyntaxContext *sc,
    LinkedList *importedFunctions, LinkedList *nativeFunctions,
    LexicalToken *lt, uint64_t startOffset) {

    // swap `for` parameters
    LexicalToken token;
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_SWAP;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }

    int64_t for_address = ftell(out);
    // test bloc
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_DUP2;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_LT;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }

    // `jumpIf` after test bloc
    int64_t currentTokenPos = linkedListIndexOf(code, lt);
    if (currentTokenPos == -1) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    LexicalToken *nextToken = linkedListGet(code, currentTokenPos + 1);
    if (nextToken == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    binexecWriteDebugSymbol(out, debug, nextToken, startOffset);
    uint8_t type = OPCODE_JUMPIF;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint64_t jumpif_address = ftell(out);
    uint64_t dummy = 0;
    if (fwrite(&dummy, sizeof (dummy), 1, out) != 1) {
        return false;
    }

    // code bloc
    LinkedList *breakList = linkedListCreate();
    LinkedList *continueList = linkedListCreate();
    LinkedListIterator *it = linkedListCreateIterator(lt->options.code.code);
    LexicalToken *tok = NULL;
    while ((tok = linkedListIteratorNext(it)) != NULL) {
        if (!binexecWriteToken(out, debug, lt->options.code.code,
            sc, importedFunctions,
            nativeFunctions, tok, startOffset,
            breakList, continueList)) {
            linkedListIteratorDestroy(it);
            linkedListDestroy(breakList, true);
            linkedListDestroy(continueList, true);
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(lt->options.code.code, true);

    // i++
    nextToken = linkedListGet(code, currentTokenPos + 2);
    if (nextToken == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    binexecWriteDebugSymbol(out, debug, nextToken, startOffset);
    uint64_t incr_address = ftell(out);
    token.symbol = LEXICALSYMBOL_DECIMAL;
    token.data.intValue = 1;
    token.options.isDecimalFloat = false;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_ADD;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }

    // `jump to` at end of for
    type = OPCODE_JUMP;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }
    int64_t jump_offset = for_address - sizeof (for_address) - ftell(out);
    jump_offset = htovmll(jump_offset);
    if (fwrite(&jump_offset, sizeof (jump_offset), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }

    // update jumpIf
    uint64_t afterfor_address = ftell(out);
    int64_t jumpif_offset = ftell(out) - jumpif_address - sizeof (jumpif_address);
    fseek(out, jumpif_address, SEEK_SET);
    jumpif_offset = htovmll(jumpif_offset);
    if (fwrite(&jumpif_offset, sizeof (jumpif_offset), 1, out) != 1) {
        return false;
    }
    fseek(out, afterfor_address, SEEK_SET);

    // drop parameters
    token.symbol = LEXICALSYMBOL_INTERNAL;
    token.data.internal = INTERNAL_DROP2;
    if (!binexecWriteToken(out, NULL, code, sc, importedFunctions, nativeFunctions,
        &token, startOffset, NULL, NULL)) {
        return false;
    }

    // update breaks & continues
    binexecUpdateOffsets(out, breakList, afterfor_address);
    binexecUpdateOffsets(out, continueList, incr_address);
    return true;
}

bool binexecWriteWhile(FILE *out, FILE *debug, LinkedList *code, SyntaxContext *sc,
    LinkedList *importedFunctions, LinkedList *nativeFunctions,
    LexicalToken *lt, uint64_t startOffset) {

    // `test` code bloc
    int64_t while_address = ftell(out);
    LinkedListIterator *it = linkedListCreateIterator(lt->options.code.code2);
    LexicalToken *tok = NULL;
    while ((tok = linkedListIteratorNext(it)) != NULL) {
        if (!binexecWriteToken(out, debug, code, sc, importedFunctions, nativeFunctions,
            tok, startOffset, NULL, NULL)) {
            linkedListIteratorDestroy(it);
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(lt->options.code.code2, true);

    // `jumpIfNot` after test bloc
    int64_t currentTokenPos = linkedListIndexOf(code, lt);
    if (currentTokenPos == -1) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    LexicalToken *nextToken = linkedListGet(code, currentTokenPos + 1);
    if (nextToken == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    binexecWriteDebugSymbol(out, debug, nextToken, startOffset);
    uint8_t type = OPCODE_JUMPIFNOT;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint64_t jumpifnot_address = ftell(out);
    uint64_t dummy = 0;
    if (fwrite(&dummy, sizeof (dummy), 1, out) != 1) {
        return false;
    }

    // `do` code bloc
    LinkedList *breakList = linkedListCreate();
    LinkedList *continueList = linkedListCreate();
    it = linkedListCreateIterator(lt->options.code.code);
    tok = NULL;
    while ((tok = linkedListIteratorNext(it)) != NULL) {
        if (!binexecWriteToken(out, debug, code, sc, importedFunctions,
            nativeFunctions, tok, startOffset,
            breakList, continueList)) {
            linkedListIteratorDestroy(it);
            linkedListDestroy(breakList, true);
            linkedListDestroy(continueList, true);
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(lt->options.code.code, true);

    // `jump to` at end of while
    nextToken = linkedListGet(code, currentTokenPos + 2);
    if (nextToken == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    binexecWriteDebugSymbol(out, debug, nextToken, startOffset);
    type = OPCODE_JUMP;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }
    int64_t jump_offset = while_address - sizeof (while_address) - ftell(out);
    jump_offset = htovmll(jump_offset);
    if (fwrite(&jump_offset, sizeof (jump_offset), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }

    // update jumpIfNot
    uint64_t afterwhile_address = ftell(out);
    int64_t jumpifnot_offset = ftell(out) - jumpifnot_address - sizeof (jumpifnot_address);
    fseek(out, jumpifnot_address, SEEK_SET);
    jumpifnot_offset = htovmll(jumpifnot_offset);
    if (fwrite(&jumpifnot_offset, sizeof (jumpifnot_offset), 1, out) != 1) {
        return false;
    }
    fseek(out, afterwhile_address, SEEK_SET);

    // update breaks & continues
    binexecUpdateOffsets(out, breakList, afterwhile_address);
    binexecUpdateOffsets(out, continueList, while_address);
    return true;
}

bool binexecWriteDo(FILE *out, FILE *debug, LinkedList *code, SyntaxContext *sc,
    LinkedList *importedFunctions, LinkedList *nativeFunctions,
    LexicalToken *lt, uint64_t startOffset) {
    // `do` code bloc
    int64_t do_address = ftell(out);
    LinkedList *breakList = linkedListCreate();
    LinkedList *continueList = linkedListCreate();
    LinkedListIterator *it = linkedListCreateIterator(lt->options.code.code);
    LexicalToken *tok = NULL;
    while ((tok = linkedListIteratorNext(it)) != NULL) {
        if (!binexecWriteToken(out, debug, code, sc, importedFunctions, nativeFunctions,
            tok, startOffset, breakList, continueList)) {
            linkedListIteratorDestroy(it);
            linkedListDestroy(breakList, true);
            linkedListDestroy(continueList, true);
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(lt->options.code.code, true);

    // test code bloc
    it = linkedListCreateIterator(lt->options.code.code2);
    tok = NULL;
    while ((tok = linkedListIteratorNext(it)) != NULL) {
        if (!binexecWriteToken(out, debug, code, sc, importedFunctions, nativeFunctions,
            tok, startOffset, NULL, NULL)) {
            linkedListIteratorDestroy(it);
            linkedListDestroy(breakList, true);
            linkedListDestroy(continueList, true);
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(lt->options.code.code2, true);

    // while test
    int64_t currentTokenPos = linkedListIndexOf(code, lt);
    if (currentTokenPos == -1) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    LexicalToken *nextToken = linkedListGet(code, currentTokenPos + 1);
    if (nextToken == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    binexecWriteDebugSymbol(out, debug, nextToken, startOffset);
    uint8_t type = OPCODE_JUMPIF;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }
    int64_t jump_offset = do_address - sizeof (do_address) - ftell(out);
    jump_offset = htovmll(jump_offset);
    if (fwrite(&jump_offset, sizeof (jump_offset), 1, out) != 1) {
        linkedListDestroy(breakList, true);
        linkedListDestroy(continueList, true);
        return false;
    }
    uint64_t after_do_address = ftell(out);

    // update breaks & continues
    binexecUpdateOffsets(out, breakList, after_do_address);
    binexecUpdateOffsets(out, continueList, do_address);
    return true;
}

bool binexecWriteIf(FILE *out, FILE *debug, LinkedList *code, SyntaxContext *sc,
    LinkedList *importedFunctions, LinkedList *nativeFunctions,
    LexicalToken *lt, uint64_t startOffset,
    LinkedList *breakList, LinkedList *continueList) {
    uint8_t type = OPCODE_JUMPIFNOT;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint64_t offset = ftell(out);
    uint64_t dummy = 0;
    if (fwrite(&dummy, sizeof (dummy), 1, out) != 1) {
        return false;
    }

    LinkedListIterator *it = linkedListCreateIterator(lt->options.code.code);
    LexicalToken *tok = NULL;
    while ((tok = linkedListIteratorNext(it)) != NULL) {
        if (!binexecWriteToken(out, debug, code, sc, importedFunctions, nativeFunctions,
            tok, startOffset, breakList, continueList)) {
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(lt->options.code.code, true);

    // jump after `else` bloc
    uint64_t offset_else;
    if (lt->options.code.code2 != NULL) {
        type = OPCODE_JUMP;
        if (fwrite(&type, sizeof (type), 1, out) != 1) {
            return false;
        }
        offset_else = (uint64_t) ftell(out);
        dummy = 0;
        if (fwrite(&dummy, sizeof (dummy), 1, out) != 1) {
            return false;
        }
    }

    // update `if` jump offset
    int64_t jump_offset = ftell(out) - offset - sizeof (offset);
    fseek(out, offset, SEEK_SET);
    jump_offset = htovmll(jump_offset);
    if (fwrite(&jump_offset, sizeof (jump_offset), 1, out) != 1) {
        return false;
    }
    jump_offset = vmtohll(jump_offset);
    fseek(out, jump_offset, SEEK_CUR);

    // `else` bloc
    if (lt->options.code.code2 != NULL) {
        LinkedListIterator *it = linkedListCreateIterator(lt->options.code.code2);
        LexicalToken *tok = NULL;
        while ((tok = linkedListIteratorNext(it)) != NULL) {
            if (!binexecWriteToken(out, debug, code, sc, importedFunctions, nativeFunctions,
                tok, startOffset, breakList, continueList)) {
                return false;
            }
        }
        linkedListIteratorDestroy(it);
        linkedListDestroy(lt->options.code.code2, true);

        jump_offset = ftell(out) - offset_else - sizeof (offset_else);
        fseek(out, offset_else, SEEK_SET);
        jump_offset = htovmll(jump_offset);
        if (fwrite(&jump_offset, sizeof (jump_offset), 1, out) != 1) {
            return false;
        }
        jump_offset = vmtohll(jump_offset);
        fseek(out, jump_offset, SEEK_CUR);
    }
    return true;
}

bool binexecWriteBreakContinue(SyntaxContext *sc, FILE *out, LinkedList *list) {
    if (list == NULL) {
        asErrorLink(sc->currentFunction, ERROR_INTERNAL);
        return false;
    }
    uint8_t type = OPCODE_JUMP;
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    int64_t *offset = memAlloc(sizeof (int64_t));
    *offset = ftell(out);
    linkedListAppend(list, offset);
    uint64_t dummy = 0;
    return fwrite(&dummy, sizeof (dummy), 1, out) == 1;
}

FunctionPointer* binexecFindExternalFunction(LinkedList *functions, char *name) {
    LinkedListIterator *itf = linkedListCreateIterator(functions);
    FunctionPointer *fp = NULL;
    FunctionPointer *found = NULL;
    while ((fp = linkedListIteratorNext(itf)) != NULL) {
        if (strcmp(fp->name, name) == 0) {
            found = fp;
            break;
        }
    }
    linkedListIteratorDestroy(itf);
    return found;
}

ClassPointer* binexecFindExternalClass(LinkedList *classes, char *name) {
    LinkedListIterator *itf = linkedListCreateIterator(classes);
    ClassPointer *cp = NULL;
    ClassPointer *found = NULL;
    while ((cp = linkedListIteratorNext(itf)) != NULL) {
        if (strcmp(cp->name, name) == 0) {
            found = cp;
            break;
        }
    }
    linkedListIteratorDestroy(itf);
    return found;
}

NativePointer* binexecFindExternalNativeFunction(LinkedList *functions, char *name) {
    LinkedListIterator *itf = linkedListCreateIterator(functions);
    NativePointer *np = NULL;
    NativePointer *found = NULL;
    while ((np = linkedListIteratorNext(itf)) != NULL) {
        if ((strcmp(np->name, name) == 0) && (np->import == -1)) {
            found = np;
            break;
        }
    }
    linkedListIteratorDestroy(itf);
    return found;
}

bool binexecWriteBlankInstanceFieldCall(FILE *out,
    SyntaxContext *sc, char *name, bool byref) {
    if (strlen(name) < 2) {
        return false;
    }
    if (name[0] != '.') {
        asError(sc->lexicalContext, ERROR_UNKNOWIDENTIFIER, name);
        return false;
    }
    uint8_t type = OPCODE_IFIELDCALL;
    if (byref) {
        type = OPCODE_IFIELDCALL_REF;
    }
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    uint32_t stringIndex = (uint32_t) linkedListSize(sc->strings);
    // TODO : doublons
    linkedListAppend(sc->strings, strbufferClone(name + 1));
    stringIndex = htovml(stringIndex);
    return fwrite(&stringIndex, sizeof (stringIndex), 1, out) == 1;
}

bool binexecWriteBlankClasseCall(FILE *out,
    SyntaxContext *sc, char *name, bool byref) {
    
    // searching for class in source file
    LinkedListIterator *it = linkedListCreateIterator(sc->classes);
    Class *c = NULL;
    while ((c = linkedListIteratorNext(it)) != NULL) {
        if ((strcmp(c->name, name) == 0)) {
            break;
        }
    }
    linkedListIteratorDestroy(it);
    
    // searching for class in imports
    if (c == NULL) {
        int importIndex = 0;
        Import *i = NULL;
        ClassPointer *cp = NULL;
        char *realname = strchr(name, ':');
        LinkedListIterator *it = linkedListCreateIterator(sc->imports);
        if (realname == NULL) {
            while ((i = linkedListIteratorNext(it)) != NULL) {
                if ((cp = binexecFindExternalClass(importGetClasses(i), name))
                    != NULL) {
                    break;
                }
                importIndex++;
            }
        } else {
            realname[0] = '\0';
            realname++;
            while ((i = linkedListIteratorNext(it)) != NULL) {
                if (importGetAlias(i) != NULL) {
                    if (strcmp(importGetAlias(i), name) == 0) {
                        if ((cp = binexecFindExternalClass(
                                  importGetClasses(i), realname)) != NULL) {
                            break;
                        }
                    }
                }
                importIndex++;
            }
        }
        linkedListIteratorDestroy(it);
        if ((i != NULL) && (cp != NULL)) {
            c = classCreate(name, importIndex);
            linkedListAppend(sc->classes, c);
        }
    }
    
    if ((c != NULL) && (byref)) {
        uint8_t type = OPCODE_CLASSREF;
        if (fwrite(&type, sizeof (type), 1, out) != 1) {
            return false;
        }
        uint64_t *offset = memAlloc(sizeof (uint64_t));
        *offset = ftell(out);
        linkedListAppend(c->refCalls, offset);
        uint32_t dummy = 0;
        return fwrite(&dummy, sizeof (dummy), 1, out) == 1;
    }
    return binexecWriteBlankInstanceFieldCall(out, sc, name, byref);
}

bool binexecWriteBlankExternalNativeFunctionCall(FILE *out,
    SyntaxContext *sc, LinkedList *nativeFunctions, char *name, bool byref) {
    LinkedListIterator *it = linkedListCreateIterator(sc->imports);
    int importIndex = 0;
    Import *i = NULL;
    NativePointer *np = NULL;
    char *realname = strchr(name, ':');
    if (realname == NULL) {
        while ((i = linkedListIteratorNext(it)) != NULL) {
            if ((np = binexecFindExternalNativeFunction(importGetNativeFunctions(i), name))
                != NULL) {
                break;
            }
            importIndex++;
        }
    } else {
        realname[0] = '\0';
        realname++;
        while ((i = linkedListIteratorNext(it)) != NULL) {
            if (importGetAlias(i) != NULL) {
                if (strcmp(importGetAlias(i), name) == 0) {
                    if ((np = binexecFindExternalNativeFunction(
                        importGetNativeFunctions(i), realname)) != NULL) {
                        break;
                    }
                }
            }
            importIndex++;
        }
    }
    linkedListIteratorDestroy(it);

    if ((i != NULL) && (np != NULL)) {
        LinkedListIterator *it = linkedListCreateIterator(nativeFunctions);
        NativeFunction *nf = NULL;
        while ((nf = linkedListIteratorNext(it)) != NULL) {
            if ((nf->importIndex == importIndex) && (strcmp(nf->name, np->name)
                == 0)) {
                break;
            }
        }
        linkedListIteratorDestroy(it);
        if (nf == NULL) {
            nf = memAlloc(sizeof (NativeFunction));
            nf->name = np->name;
            nf->importIndex = importIndex;
            nf->calls = linkedListCreate();
            linkedListAppend(nativeFunctions, nf);
        }
        uint8_t type = OPCODE_NATIVECALL;
        if (byref) {
            type = OPCODE_NATIVECALL_REF;
        }
        if (fwrite(&type, sizeof (type), 1, out) != 1) {
            return false;
        }
        uint64_t *offset = memAlloc(sizeof (uint64_t));
        *offset = ftell(out);
        linkedListAppend(nf->calls, offset);
        uint32_t dummy = 0;
        return fwrite(&dummy, sizeof (dummy), 1, out) == 1;
    }
    if (realname != NULL) {
        realname--;
        realname[0] = ':';
    }
    return binexecWriteBlankClasseCall(out, sc, name, byref);
}

bool binexecWriteBlankExternalFunctionCall(FILE *out,
    SyntaxContext *sc, LinkedList *importedFunctions,
    LinkedList *nativeFunctions, char *name, bool byref) {
    int importIndex = 0;
    Import *i = NULL;
    FunctionPointer *fp = NULL;
    char *realname = strchr(name, ':');
    LinkedListIterator *it = linkedListCreateIterator(sc->imports);
    if (realname == NULL) {
        while ((i = linkedListIteratorNext(it)) != NULL) {
            if ((fp = binexecFindExternalFunction(importGetFunctions(i), name))
                != NULL) {
                break;
            }
            importIndex++;
        }
    } else {
        realname[0] = '\0';
        realname++;
        while ((i = linkedListIteratorNext(it)) != NULL) {
            if (importGetAlias(i) != NULL) {
                if (strcmp(importGetAlias(i), name) == 0) {
                    if ((fp = binexecFindExternalFunction(
                        importGetFunctions(i), realname)) != NULL) {
                        break;
                    }
                }
            }
            importIndex++;
        }
    }
    linkedListIteratorDestroy(it);
    if ((i != NULL) && (fp != NULL)) {
        LinkedListIterator *it = linkedListCreateIterator(importedFunctions);
        ImportedFunction *ii = NULL;
        while ((ii = linkedListIteratorNext(it)) != NULL) {
            if ((ii->importIndex == importIndex) && (strcmp(ii->name, fp->name)
                == 0)) {
                break;
            }
        }
        linkedListIteratorDestroy(it);
        if (ii == NULL) {
            ii = memAlloc(sizeof (ImportedFunction));
            ii->name = fp->name;
            ii->importIndex = importIndex;
            ii->calls = linkedListCreate();
            linkedListAppend(importedFunctions, ii);
        }
        uint8_t type = OPCODE_EXTERNALCALL;
        if (byref) {
            type = OPCODE_EXTERNALCALL_REF;
        }
        if (fwrite(&type, sizeof (type), 1, out) != 1) {
            return false;
        }
        uint64_t *offset = memAlloc(sizeof (uint64_t));
        *offset = ftell(out);
        linkedListAppend(ii->calls, offset);
        uint32_t dummy = 0;
        return fwrite(&dummy, sizeof (dummy), 1, out) == 1;
    }
    if (realname != NULL) {
        realname--;
        realname[0] = ':';
    }
    return binexecWriteBlankExternalNativeFunctionCall(out, sc,
        nativeFunctions, name, byref);
}

bool binexecWriteBlankNativeFunctionCall(FILE *out,
    SyntaxContext *sc, LinkedList *importedFunctions,
    LinkedList *nativeFunctions, char *name, bool byref) {

    if (!byref) {
        LinkedListIterator *it = linkedListCreateIterator(nativeFunctions);
        NativeFunction *nf = NULL;
        while ((nf = linkedListIteratorNext(it)) != NULL) {
            if ((nf->importIndex == -1) && (strcmp(name, nf->name) == 0)) {
                break;
            }
        }
        linkedListIteratorDestroy(it);

        if (nf != NULL) {
            uint8_t type = OPCODE_NATIVECALL;
            if (byref) {
                type = OPCODE_NATIVECALL_REF;
            }
            if (fwrite(&type, sizeof (type), 1, out) != 1) {
                return false;
            }
            uint64_t *offset = memAlloc(sizeof (uint64_t));
            *offset = ftell(out);
            linkedListAppend(nf->calls, offset);
            uint32_t dummy = 0;
            return fwrite(&dummy, sizeof (dummy), 1, out) == 1;
        }
    }
    return binexecWriteBlankExternalFunctionCall(out, sc,
        importedFunctions, nativeFunctions, name, byref);
}

bool binexecWriteBlankFunctionCall(FILE *out,
    SyntaxContext *sc, LinkedList *importedFunctions,
    LinkedList *nativeFunctions, char *name, bool byref) {
    // find function in internal function list
    LinkedListIterator *it = linkedListCreateIterator(sc->functions);
    Function *f = NULL;
    while ((f = linkedListIteratorNext(it)) != NULL) {
        if (strcmp(functionGetName(f), name) == 0) {
            break;
        }
    }
    linkedListIteratorDestroy(it);
    if (f == NULL) {
        return binexecWriteBlankNativeFunctionCall(out, sc,
            importedFunctions, nativeFunctions, name, byref);
    }
    // write offset value 0
    uint8_t type = OPCODE_INTERNALCALL;
    if (byref) {
        type = OPCODE_INTERNALCALL_REF;
    }
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    // get actual position
    uint64_t offset = (uint64_t) ftell(out);
    //printf("%s 0x%x 0x%x\n", name, functionGetOffset(f), offset);
    // add position to function call list
    functionAppendCall(f, offset);
    offset = 0;
    return fwrite(&offset, sizeof (offset), 1, out) == 1;
}

bool binexecWriteVariable(FILE *out,
    SyntaxContext *sc, LinkedList *importedFunctions,
    LinkedList *nativeFunctions, char *name, bool byref,
    bool varset) {
    // find variable
    uint32_t varIndex = 0;
    LinkedListIterator *it = linkedListCreateIterator(sc->variables);
    char *v = NULL;
    while ((v = linkedListIteratorNext(it)) != NULL) {
        if (strcmp(v, name) == 0) {
            break;
        }
        varIndex++;
    }
    linkedListIteratorDestroy(it);
    if (v == NULL) {
        if (varset) {
            if ((strlen(name) < 2) || (name[0] != '.')) {
                asError(sc->lexicalContext, ERROR_UNKNOWIDENTIFIER, name);
                return false;
            }
            uint8_t type = OPCODE_IFIELD_SET;
            if (fwrite(&type, sizeof (type), 1, out) != 1) {
                return false;
            }
            uint32_t stringIndex = (uint32_t) linkedListSize(sc->strings);
            // TODO : doublons
            linkedListAppend(sc->strings, strbufferClone(name+1));
            stringIndex = htovml(stringIndex);
            return fwrite(&stringIndex, sizeof (stringIndex), 1, out) == 1;
        }
        return binexecWriteBlankFunctionCall(out, sc,
            importedFunctions, nativeFunctions,
            name, byref);
    }
    // write offset value 0
    uint8_t type = OPCODE_VAR_GET;
    if (byref) {
        type = OPCODE_VAR_REF;
    } else if (varset) {
        type = OPCODE_VAR_SET;
    }
    if (fwrite(&type, sizeof (type), 1, out) != 1) {
        return false;
    }
    varIndex = htovml(varIndex);
    return fwrite(&varIndex, sizeof (varIndex), 1, out) == 1;
}

bool binexecWriteDebugSymbol(FILE *out, FILE *debug, LexicalToken *lt,
    uint64_t startOffset) {
    if (debug != NULL) {
        if (lt->symbol == LEXICALSYMBOL_RESERVEDWORD) {
            if ((lt->data.word == RESERVEDWORD_WHILE) ||
                (lt->data.word == RESERVEDWORD_DO)) {
                return true;
            }
        }
        DebugSymbol symbol;
        symbol.symbol = htovml(lt->symbol);
        symbol.codeOffset = htovmll(ftell(out) - startOffset);
        symbol.lineNumber = htovmll(lt->lineNumber);
        symbol.startingChar = htovmll(lt->startingChar);
        symbol.endingChar = htovmll(lt->endingChar);
        if (fwrite(&symbol, sizeof (symbol), 1, debug) != 1) {
            return false;
        }
        debugSymbolCount++;
    }
    return true;
}

bool binexecWriteToken(FILE *out, FILE *debug, LinkedList *code, SyntaxContext *sc,
    LinkedList *importedFunctions, LinkedList *nativeFunctions,
    LexicalToken *lt, uint64_t startOffset,
    LinkedList *breakList, LinkedList *continueList) {
    if ((code != NULL) &&
        (lt->symbol != LEXICALSYMBOL_STARTFUNC) &&
        (lt->symbol != LEXICALSYMBOL_ENDFUNC)) {
        if (!binexecWriteDebugSymbol(out, debug, lt, startOffset)) {
            return false;
        }
    }
    /*printf("\t%d 0x%llx : line %lld\n", lt->symbol,
                    ftell(out), lt->lineNumber);*/
    switch (lt->symbol) {
        case LEXICALSYMBOL_RESERVEDWORD:
            switch (lt->data.word) {
                case RESERVEDWORD_NULL:
                    return binexecWriteNull(out);
                case RESERVEDWORD_TRUE:
                case RESERVEDWORD_FALSE:
                    return binexecWriteBoolean(out,
                        lt->data.word == RESERVEDWORD_TRUE);
                case RESERVEDWORD_RETURN:
                    return binexecWriteReturn(out);
                case RESERVEDWORD_IF:
                    return binexecWriteIf(out, debug, code, sc, importedFunctions,
                        nativeFunctions, lt, startOffset, breakList, continueList);
                case RESERVEDWORD_WHILE:
                    return binexecWriteWhile(out, debug, code, sc, importedFunctions,
                        nativeFunctions, lt, startOffset);
                case RESERVEDWORD_DO:
                    return binexecWriteDo(out, debug, code, sc, importedFunctions,
                        nativeFunctions, lt, startOffset);
                case RESERVEDWORD_FOR:
                    return binexecWriteFor(out, debug, code, sc, importedFunctions,
                        nativeFunctions, lt, startOffset);
                case RESERVEDWORD_FOREACH:
                    return binexecWriteForeach(out, debug, code, sc, importedFunctions,
                        nativeFunctions, lt, startOffset);
                case RESERVEDWORD_BREAK:
                    return binexecWriteBreakContinue(sc, out, breakList);
                case RESERVEDWORD_CONTINUE:
                    return binexecWriteBreakContinue(sc, out, continueList);
                default:
                    asErrorLink(sc->currentFunction, ERROR_UNKNOWNRESERVEDWORD,
                        asReservedWord2Identifier(lt->data.word));
                    break;
            }
            break;
        case LEXICALSYMBOL_DECIMAL:
            if (lt->options.isDecimalFloat) {
                return binexecWriteFloat(out, lt->data.floatValue);
            }
            return binexecWriteInt(out, lt->data.intValue);
        case LEXICALSYMBOL_STRING:
            return binexecWriteString(out, lt->data.string, sc->strings);
        case LEXICALSYMBOL_STARTARRAY:
            return binexecWriteArray(out, lt->data.array, sc->arrays);
        case LEXICALSYMBOL_INTERNAL:
            return binexecWriteInternal(out, lt->data.internal);
        case LEXICALSYMBOL_VARSET:
            return binexecWriteVariable(out,
                sc, importedFunctions, nativeFunctions,
                lt->data.identifier, false, true);
        case LEXICALSYMBOL_REF:
            if (strchr(lt->data.identifier, ':') == NULL) {
                return binexecWriteVariable(out,
                    sc, importedFunctions, nativeFunctions,
                    lt->data.identifier, true, false);
            }
            return binexecWriteBlankExternalFunctionCall(out,
                sc, importedFunctions, nativeFunctions,
                lt->data.identifier, true);
        case LEXICALSYMBOL_IDENTIFIER:
            if (strchr(lt->data.identifier, ':') == NULL) {
                return binexecWriteVariable(out,
                    sc, importedFunctions, nativeFunctions,
                    lt->data.identifier, false, false);
            }
            return binexecWriteBlankExternalFunctionCall(out,
                sc, importedFunctions, nativeFunctions,
                lt->data.identifier, false);
        case LEXICALSYMBOL_STARTFUNC:
        case LEXICALSYMBOL_ENDFUNC:
            return true;
        default:
            break;
    }
    return false;
}

bool binexecWriteEffectiveClassCall(FILE *out, Class *class, uint32_t index) {
    LinkedListIterator *it = linkedListCreateIterator(class->refCalls);
    uint64_t *callOffset = NULL;
    index = htovml(index);
    while ((callOffset = linkedListIteratorNext(it)) != NULL) {
        if (fseek(out, *callOffset, SEEK_SET) == -1) {
            return false;
        }
        if (fwrite(&index, sizeof (index), 1, out) != 1) {
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    return true;
}

bool binexecWriteEffectiveFunctionCall(FILE *out, Function *function) {
    LinkedListIterator *it =
        linkedListCreateIterator(functiongetGetCallsList(function));
    uint64_t *callOffset = NULL;
    uint64_t functionOffset = htovmll(functionGetOffset(function));
    while ((callOffset = linkedListIteratorNext(it)) != NULL) {
        if (fseek(out, *callOffset, SEEK_SET) == -1) {
            return false;
        }
        if (fwrite(&functionOffset, sizeof (functionOffset), 1, out) != 1) {
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    return true;
}

bool binexecWriteEffectiveExternalFunctionCall(FILE *out,
    ImportedFunction *function) {
    LinkedListIterator *it = linkedListCreateIterator(function->calls);
    uint64_t *callOffset = NULL;
    uint32_t functionIndex = htovml(function->functionIndex);
    while ((callOffset = linkedListIteratorNext(it)) != NULL) {
        if (fseek(out, *callOffset, SEEK_SET) == -1) {
            return false;
        }
        if (fwrite(&functionIndex, sizeof (functionIndex), 1, out) != 1) {
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    return true;
}

bool binexecWriteEffectiveNativeFunctionCall(FILE *out,
    NativeFunction *function) {
    LinkedListIterator *it = linkedListCreateIterator(function->calls);
    uint64_t *callOffset = NULL;
    uint32_t functionIndex = htovml(function->functionIndex);
    while ((callOffset = linkedListIteratorNext(it)) != NULL) {
        if (fseek(out, *callOffset, SEEK_SET) == -1) {
            return false;
        }
        if (fwrite(&functionIndex, sizeof (functionIndex), 1, out) != 1) {
            return false;
        }
    }
    linkedListIteratorDestroy(it);
    return true;
}

bool binexecWriteFunctions(FILE *out, FILE *debug, uint64_t startOffset,
    LinkedList *functions, LinkedList *importedFunctions, LinkedList *nativeFunctions,
    SyntaxContext *sc, Class *c) {
    LinkedListIterator *it = linkedListCreateIterator(functions);
    Function *f = NULL;
    while ((f = linkedListIteratorNext(it)) != NULL) {
        LinkedListIterator *itf =
            linkedListCreateIterator(functionGetCodeList(f));
        LexicalToken *lt = NULL;
        LexicalToken *last = NULL;
        functionSetOffset(f, ftell(out) - startOffset);
        sc->currentFunction = f;
        /*printf("%s 0x%llx 0x%llx\n", functionGetName(f), functionGetOffset(f),
            ftell(out) - startOffset);*/
        if ((c != NULL) && (c->parent != NULL) && strbufferEquals(f->name, "oncreate") ) {
            if (!binexecWriteInternal(out, INTERNAL_SUPER)) {
                return false;
            }
            binexecWriteBlankInstanceFieldCall(out, sc, ".oncreate", false);
        }
        while ((lt = linkedListIteratorNext(itf)) != NULL) {
            if (!binexecWriteToken(out, debug, functionGetCodeList(f),
                sc, importedFunctions,
                nativeFunctions, lt, startOffset, NULL, NULL)) {
                linkedListIteratorDestroy(itf);
                linkedListIteratorDestroy(it);
                ImportedFunction *ii = NULL;
                LinkedListIterator *itt = linkedListCreateIterator(importedFunctions);
                while ((ii = linkedListIteratorNext(itt)) != NULL) {
                    linkedListDestroy(ii->calls, true);
                }
                linkedListIteratorDestroy(itt);
                linkedListDestroy(importedFunctions, true);
                NativeFunction *nf = NULL;
                itt = linkedListCreateIterator(nativeFunctions);
                while ((nf = linkedListIteratorNext(itt)) != NULL) {
                    linkedListDestroy(nf->calls, true);
                }
                linkedListIteratorDestroy(itt);
                linkedListDestroy(nativeFunctions, true);
                return false;
            }
            last = lt;
        }
        if ((last == NULL) ||
            (last->symbol == LEXICALSYMBOL_ENDFUNC)) {
            last->symbol = LEXICALSYMBOL_RESERVEDWORD;
            last->data.word = RESERVEDWORD_RETURN;
            if (!binexecWriteDebugSymbol(out, debug, last, startOffset)) {
                linkedListIteratorDestroy(it);
                linkedListIteratorDestroy(itf);
                return false;
            }
            if (!binexecWriteReturn(out)) {
                linkedListIteratorDestroy(it);
                linkedListIteratorDestroy(itf);
                return false;
            }
        }
        linkedListIteratorDestroy(itf);
    }
    linkedListIteratorDestroy(it);
    return true;
}

bool binexecWriteContent(FILE *out, FILE *debug, SyntaxContext *sc) {
    if (debug != NULL) {
        debugSymbolCount = 0;
    }
    // write code
    uint64_t startOffset = ftell(out);
    //printf("start: 0x%llx\n", startOffset);
    LinkedList *importedFunctions = linkedListCreate();

    LinkedList *nativeFunctions = linkedListCreate();
    LinkedListIterator *it = linkedListCreateIterator(sc->natives);
    char *native = NULL;
    while ((native = linkedListIteratorNext(it)) != NULL) {
        NativeFunction *nf = memAlloc(sizeof (NativeFunction));
        nf->importIndex = -1;
        nf->name = native;
        nf->calls = linkedListCreate();
        linkedListAppend(nativeFunctions, nf);
    }
    linkedListIteratorDestroy(it);

    if (!binexecWriteFunctions(out, debug, startOffset, sc->functions,
        importedFunctions, nativeFunctions, sc, NULL)) {
        return false;
    }
    it = linkedListCreateIterator(sc->classes);
    Class *c = NULL;
    while ((c = linkedListIteratorNext(it)) != NULL) {
        if (c->import == -1) {
            bool haveOnCreate = false;
            LinkedListIterator *itf = linkedListCreateIterator(c->functions);
            Function *ifp = NULL;
            while ((ifp = linkedListIteratorNext(itf)) != NULL) {
                if (strbufferEquals("oncreate", ifp->name)) {
                    haveOnCreate = true;
                }
            }
            linkedListIteratorDestroy(itf);
            if (!haveOnCreate) {
                ifp = functionCreate("oncreate");
                LexicalToken *lt = memAlloc(sizeof(LexicalToken));
                lt->symbol = LEXICALSYMBOL_ENDFUNC;
                lt->lineNumber = 0;
                lt->startingChar = 0;
                lt->endingChar = 0;
                linkedListAppend(ifp->code, lt);
                linkedListAppend(c->functions, ifp);
            }
            if (!binexecWriteFunctions(out, debug, startOffset, c->functions,
                                       importedFunctions, nativeFunctions, sc,
                                       c)) {
                return false;
            }
        }
    }
    linkedListIteratorDestroy(it);

    // write blank offsets to other tables
    uint64_t tableOffsets = ftell(out);
    uint64_t blankOffset = 0;
    // 6 tables offsets : 
    // imports, imported functions, native functions, arrays, classes, strings
    for (int i = 0; i < 6; i++) {
        if (fwrite(&blankOffset, sizeof (blankOffset), 1, out) != 1) {
            return false;
        }
    }

    // write internal functions table
    it = linkedListCreateIterator(sc->functions);
    Function *f = NULL;
    while ((f = linkedListIteratorNext(it)) != NULL) {
        char *name = functionGetName(f);
        if (fwrite(name, strlen(name) + 1, 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
        uint64_t offset = htovmll(functionGetOffset(f));
        if (fwrite(&offset, sizeof (offset), 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
    }
    linkedListIteratorDestroy(it);

    // write imports table
    uint64_t importsTableOffset = ftell(out);
    it = linkedListCreateIterator(sc->imports);
    Import *i = NULL;
    while ((i = linkedListIteratorNext(it)) != NULL) {
        char *name = importGetName(i);
        if (fwrite(name, strlen(name) + 1, 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
    }
    linkedListIteratorDestroy(it);

    // write imported functions table
    uint64_t importedFunctionsTableOffset = ftell(out);
    it = linkedListCreateIterator(importedFunctions);
    ImportedFunction *ii = NULL;
    uint32_t functionIndex = 0;
    while ((ii = linkedListIteratorNext(it)) != NULL) {
        ii->functionIndex = functionIndex++;
        ii->importIndex = htovml(ii->importIndex);
        if (fwrite(&ii->importIndex, sizeof (ii->importIndex), 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
        if (fwrite(ii->name, strlen(ii->name) + 1, 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
    }
    linkedListIteratorDestroy(it);

    // write native functions table
    uint64_t nativeFunctionsTableOffset = ftell(out);
    functionIndex = 0;
    it = linkedListCreateIterator(nativeFunctions);
    NativeFunction *nf = NULL;
    while ((nf = linkedListIteratorNext(it)) != NULL) {
        nf->functionIndex = functionIndex++;
        nf->importIndex = htovml(nf->importIndex);
        if (fwrite(&nf->importIndex, sizeof (nf->importIndex), 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
        if (fwrite(nf->name, strlen(nf->name) + 1, 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
    }
    linkedListIteratorDestroy(it);

    // write arrays table
    uint64_t arraysTableOffset = ftell(out);
    it = linkedListCreateIterator(sc->arrays);
    LinkedList *array = NULL;
    while ((array = linkedListIteratorNext(it)) != NULL) {
        uint32_t size = (uint32_t) linkedListSize(array);
        size = htovml(size);
        if (fwrite(&size, sizeof (size), 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
        LinkedListIterator *subit = linkedListCreateIterator(array);
        LexicalToken *lt = NULL;
        while ((lt = linkedListIteratorNext(subit))) {
            binexecWriteToken(out, debug, NULL, sc, importedFunctions,
                nativeFunctions, lt, startOffset, NULL, NULL);
        }
        linkedListIteratorDestroy(subit);
    }
    linkedListIteratorDestroy(it);

    // write classes table
    uint64_t classesTableOffset = ftell(out);
    it = linkedListCreateIterator(sc->classes);
    c = NULL;
    while ((c = linkedListIteratorNext(it)) != NULL) {
        if (fwrite(c->name, strlen(c->name) + 1, 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
        uint32_t importEntry = htovml(c->import);
        if (fwrite(&importEntry, sizeof (importEntry), 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
        if (c->import == -1) {
            // super class
            importEntry = -1;
            if (c->parent != NULL) {
                LinkedListIterator *itp = linkedListCreateIterator(sc->classes);
                Class *p = NULL;
                while ((p = linkedListIteratorNext(itp)) != NULL) {
                    if ((p != c) &&
                        (strbufferEquals(p->name, c->parent))) {
                        break;
                    }
                }
                linkedListIteratorDestroy(itp);
                if (p == NULL) {
                    importEntry = 0;
                    Import *imp = NULL;
                    ClassPointer *cp = NULL;
                    char *realname = strchr(c->parent, ':');
                    itp = linkedListCreateIterator(sc->imports);
                    if (realname == NULL) {
                        while ((imp = linkedListIteratorNext(itp)) != NULL) {
                            if ((cp = binexecFindExternalClass(importGetClasses(imp),
                                                               c->parent))
                                != NULL) {
                                break;
                            }
                            importEntry++;
                        }
                    } else {
                        realname[0] = '\0';
                        realname++;
                        while ((i = linkedListIteratorNext(itp)) != NULL) {
                            if (importGetAlias(imp) != NULL) {
                                if (strcmp(importGetAlias(imp), c->parent) == 0) {
                                    if ((cp = binexecFindExternalClass(
                                            importGetClasses(imp), realname)) != NULL) {
                                        break;
                                    }
                                }
                            }
                            importEntry++;
                        }
                    }
                    linkedListIteratorDestroy(itp);
                    if (cp == NULL) {
                        asError(sc->lexicalContext, ERROR_UNKNOWNSUPERCLASS,
                                c->parent);
                        linkedListIteratorDestroy(it);
                        return false;
                    }
                }
            } else {
                c->parent = strbufferClone("");
            }
            importEntry = htovml(importEntry);
            if (fwrite(&importEntry, sizeof (importEntry), 1, out) != 1) {
                linkedListIteratorDestroy(it);
                return false;
            }
            if (fwrite(c->parent, strlen(c->parent) + 1, 1, out) != 1) {
                linkedListIteratorDestroy(it);
                return false;
            }
            // instance variables
            uint32_t nivar = htovml(linkedListSize(c->variables));
            if (fwrite(&nivar, sizeof (nivar), 1, out) != 1) {
                linkedListIteratorDestroy(it);
                return false;
            }
            LinkedListIterator *itt = linkedListCreateIterator(c->variables);
            InstanceVariable *ivar = NULL;
            while ((ivar = linkedListIteratorNext(itt)) != NULL) {
                if (fwrite(ivar->name, strlen(ivar->name) + 1, 1, out) != 1) {
                    linkedListIteratorDestroy(itt);
                    linkedListIteratorDestroy(it);
                    return false;
                }
                uint8_t access = (ivar->readAccess ? 1 : 0);
                fwrite(&access, sizeof (access), 1, out);
                fwrite(ivar->readMethod, strlen(ivar->readMethod) + 1, 1, out);
                access = (ivar->writeAccess ? 1 : 0);
                fwrite(&access, sizeof (access), 1, out);
                fwrite(ivar->writeMethod, strlen(ivar->writeMethod) + 1, 1, out);
            }
            linkedListIteratorDestroy(itt);
            // instance methods
            uint32_t nfunc = htovml(linkedListSize(c->functions));
            if (fwrite(&nfunc, sizeof (nfunc), 1, out) != 1) {
                linkedListIteratorDestroy(it);
                return false;
            }
            itt = linkedListCreateIterator(c->functions);
            Function *f = NULL;
            while ((f = linkedListIteratorNext(itt)) != NULL) {
                if (fwrite(f->name, strlen(f->name) + 1, 1, out) != 1) {
                    linkedListIteratorDestroy(itt);
                    linkedListIteratorDestroy(it);
                    return false;
                }
                uint8_t access = (f->private ? 1 : 0);
                fwrite(&access, sizeof (access), 1, out);
                uint64_t offset = htovmll(f->offset);
                fwrite(&offset, sizeof (offset), 1, out);
            }
            linkedListIteratorDestroy(itt);
        }
    }
    linkedListIteratorDestroy(it);

    // write strings table
    uint64_t stringsTableOffset = ftell(out);
    it = linkedListCreateIterator(sc->strings);
    char *s = NULL;
    while ((s = linkedListIteratorNext(it)) != NULL) {
        if (fwrite(s, strlen(s) + 1, 1, out) != 1) {
            linkedListIteratorDestroy(it);
            return false;
        }
    }
    linkedListIteratorDestroy(it);

    // update internal function calls
    it = linkedListCreateIterator(sc->functions);
    while ((f = linkedListIteratorNext(it)) != NULL) {
        binexecWriteEffectiveFunctionCall(out, f);
    }
    linkedListIteratorDestroy(it);

    // update external functions calls
    it = linkedListCreateIterator(importedFunctions);
    while ((ii = linkedListIteratorNext(it)) != NULL) {
        binexecWriteEffectiveExternalFunctionCall(out, ii);
    }
    linkedListIteratorDestroy(it);

    // update native functions calls
    it = linkedListCreateIterator(nativeFunctions);
    while ((nf = linkedListIteratorNext(it)) != NULL) {
        binexecWriteEffectiveNativeFunctionCall(out, nf);
    }
    linkedListIteratorDestroy(it);

    // update classes calls
    it = linkedListCreateIterator(sc->classes);
    c = NULL;
    uint32_t index = 0;
    while ((c = linkedListIteratorNext(it)) != NULL) {
        binexecWriteEffectiveClassCall(out, c, index++);
    }
    linkedListIteratorDestroy(it);


    // update tables pointers
    if (fseek(out, tableOffsets, SEEK_SET) == -1) {
        return false;
    }
    importsTableOffset = htovmll(importsTableOffset);
    if (fwrite(&importsTableOffset, sizeof (importsTableOffset), 1, out) != 1) {
        return false;
    }
    importedFunctionsTableOffset = htovmll(importedFunctionsTableOffset);
    if (fwrite(&importedFunctionsTableOffset,
        sizeof (importedFunctionsTableOffset), 1, out) != 1) {
        return false;
    }
    nativeFunctionsTableOffset = htovmll(nativeFunctionsTableOffset);
    if (fwrite(&nativeFunctionsTableOffset,
        sizeof (nativeFunctionsTableOffset), 1, out) != 1) {
        return false;
    }
    arraysTableOffset = htovmll(arraysTableOffset);
    if (fwrite(&arraysTableOffset, sizeof (arraysTableOffset), 1, out) != 1) {
        return false;
    }
    classesTableOffset = htovmll(classesTableOffset);
    if (fwrite(&classesTableOffset, sizeof (classesTableOffset), 1, out) != 1) {
        return false;
    }
    stringsTableOffset = htovmll(stringsTableOffset);
    if (fwrite(&stringsTableOffset, sizeof (stringsTableOffset), 1, out) != 1) {
        return false;
    }

    // update header table pointer 
    if (fseek(out, sizeof (BinExecHeader) - sizeof (uint64_t), SEEK_SET) == -1) {
        return false;
    }
    tableOffsets = htovmll(tableOffsets);
    if (fwrite(&tableOffsets, sizeof (tableOffsets), 1, out) != 1) {
        return false;
    }

    it = linkedListCreateIterator(importedFunctions);
    while ((ii = linkedListIteratorNext(it)) != NULL) {
        linkedListDestroy(ii->calls, true);
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(importedFunctions, true);

    it = linkedListCreateIterator(nativeFunctions);
    while ((nf = linkedListIteratorNext(it)) != NULL) {
        linkedListDestroy(nf->calls, true);
    }
    linkedListIteratorDestroy(it);
    linkedListDestroy(nativeFunctions, true);
    if (debug != NULL) {
        if (fseek(debug, debugSymbolCountOffset, SEEK_SET) != -1) {
            debugSymbolCount = htovmll(debugSymbolCount);
            fwrite(&debugSymbolCount, sizeof (debugSymbolCount), 1, debug);
        }
    }
    return true;
}
