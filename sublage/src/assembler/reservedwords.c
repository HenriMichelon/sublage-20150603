#include <string.h>
#include "sublage/internalsIdentifiers.h"
#include "sublage/reservedwords.h"

ReservedWordIdentifier asReservedWords[] = {
    { RESERVEDWORD_NULL,  "null" },
    { RESERVEDWORD_IMPORT,  "import" },
    { RESERVEDWORD_AS,  "as" },
    { RESERVEDWORD_TRUE,  "true" },
    { RESERVEDWORD_FALSE,  "false" },
    { RESERVEDWORD_RETURN, "ret" },
    { RESERVEDWORD_IF, "if" },
    { RESERVEDWORD_ELSE, "else" },
    { RESERVEDWORD_WHILE, "while" },
    { RESERVEDWORD_DO, "do" },
    { RESERVEDWORD_NATIVE, "native" },
    { RESERVEDWORD_VAR, "var" },
    { RESERVEDWORD_BREAK, "break" },
    { RESERVEDWORD_CONTINUE, "continue" },
    { RESERVEDWORD_FOR, "for" },
    { RESERVEDWORD_FOREACH, "foreach" },
    { RESERVEDWORD_CLASS, "class" },
    { RESERVEDWORD_READ, "read" },
    { RESERVEDWORD_WRITE, "write" },
    { RESERVEDWORD_EXTENDS, "extends" },
    { RESERVEDWORD_STOPFLAG,  NULL },
};

const char* asOpcode2Identifier(Internal opcode) {
    return internalsIdentifiers[opcode].mnemonic;
}

const char* asReservedWord2Identifier(ReservedWord word) {
    int i = 0;
    while (asReservedWords[i].word != RESERVEDWORD_STOPFLAG) {
        if (asReservedWords[i].word == word) {
            return asReservedWords[i].identifier;
        }
        i++;
    }
    return NULL;
}

long asIdentier2Internal(const char*identifier) {
    int i = 0;
    while (internalsIdentifiers[i].internal != INTERNAL_STOPFLAG) {
        if (strcmp(identifier, internalsIdentifiers[i].mnemonic) == 0) {
            return internalsIdentifiers[i].internal;
        }
        i++;
    }
    return -1;
}

long asIdentier2ReservedWord(const char*identifier) {
    int i = 0;
    while (asReservedWords[i].word != RESERVEDWORD_STOPFLAG) {
        if (strcmp(identifier, asReservedWords[i].identifier) == 0) {
            return asReservedWords[i].word;
        }
        i++;
    }
    return -1;
}
