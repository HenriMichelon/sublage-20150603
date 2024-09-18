#include "sublage/mem.h"
#include <string.h>
#include <stdlib.h>
#include "errors.h"
#include "sublage/strbuffer.h"

//#define _LEXICAL_DEBUG(msg) printf(msg)
#define _LEXICAL_DEBUG(msg)


// spaces
const char lexicalSpaces[] = {' ', '\n', '\t', '\r', '\v', '\f', '\0'};
const char lexicalSeparators[] = {'"', '<', '>', '\0'};

bool lexicalIsSpace(char c) {
    const char *spaces = lexicalSpaces;
    /* traverse the spaces-type characters array
     *  until we found the NULL (0) character */
    while (*spaces) {
        if ((*spaces) == c) {
            return true;
        }
        ++spaces;
    }
    return c == '\0';
}

bool lexicalIsSeparator(char c) {
    if (lexicalIsSpace(c)) {
        return true;
    }
    const char *seps = lexicalSeparators;
    /* traverse the separator-type characters array
     *  until we found the NULL (0) character */
    while (*seps) {
        if ((*seps) == c) {
            return true;
        }
        ++seps;
    }
    return false;
}

bool lexicalIsDecimalDigit(char c) {
    return (c >= '0') && (c <= '9');
}

bool lexicalIsHexadecimentDigit(char c) {
    return ((c >= '0') && (c <= '9')) ||
            ((c >= 'a') && (c <= 'f')) ||
            ((c >= 'A') && (c <= 'F'));
}

void lexicalNextChar(LexicalContext *lc) {
    /* we reach the end of the line at last call,
     *  increment the current line number.
     *  Note this is not done at the end of the call
     *  to avoid a bad line number in case of an error
     *  on the last line of the source file */
    if (lc->currentChar == '\n') {
        ++lc->currentLine;
    }
    if (fread(&lc->currentChar, sizeof (char), 1, lc->file) != 1) {
        if (feof(lc->file)) {
            lc->currentChar = 0;
        } else {
            asError(lc, ERROR_IO);
        }
    }
    if (lc->currentChar == '#') {
        do {
            if (fread(&lc->currentChar, sizeof (char), 1, lc->file) != 1) {
                if (feof(lc->file)) {
                    lc->currentChar = 0;
                } else {
                    asError(lc, ERROR_IO);
                }
            }
        } while ((lc->currentChar != '\n') && (lc->currentChar != 0));
        lexicalNextChar(lc);
    }
}

void lexicalSkipSpace(LexicalContext *lc) {
    while (lexicalIsSpace(lc->currentChar) && (lc->currentChar != '\0')) {
        lexicalNextChar(lc);
    }
}

LexicalContext* lexicalInit(FILE* file, const char* fileName) {
    LexicalContext *lc = memAlloc(sizeof (LexicalContext));
    lc->file = file;
    lc->currentChar = 0;
    lc->currentLine = 0;
    lc->fileName = strbufferClone(fileName);
    lexicalNextChar(lc);
    if (lc->currentChar != 0) {
        lc->currentLine++;
    }
    return lc;
}

void lexicalDone(LexicalContext *lc) {
    strbufferDestroy(lc->fileName);
    memFree(lc);
}

void lexicalReadIdent(LexicalContext *lc, LexicalToken *lt, char*s) {
    _LEXICAL_DEBUG("ReadIdent\n");
    if (s == NULL) {
        s = strbufferCreate();
    }
    // An identifier is followed by a separator character
    while (!lexicalIsSeparator(lc->currentChar)) {
        s = strbufferAppendChar(s, lc->currentChar);
        lexicalNextChar(lc);
    }
    size_t len = strlen(s);
    if (len > 0) {
        /* check if we have a reserved word
         or a simple identifer */
        long id = asIdentier2Internal(s);
        if (id > -1) {
            lt->symbol = LEXICALSYMBOL_INTERNAL;
            lt->data.internal = (Internal) id;
        } else {
            id = asIdentier2ReservedWord(s);
            if (id > -1) {
                lt->symbol = LEXICALSYMBOL_RESERVEDWORD;
                lt->data.word = (Internal) id;
            } else {
                if (strbufferStrPos(s, "->") == 0) {
                    lt->symbol = LEXICALSYMBOL_VARSET;
                    char *ident = strbufferSubStr(s, 2, -1);
                    strncpy(lt->data.identifier, ident, IDENTIFER_MAX_SIZE - 1);
                    lt->data.identifier[IDENTIFER_MAX_SIZE - 1] = 0;
                    strbufferDestroy(ident);
                } else {
                    if (len >= IDENTIFER_MAX_SIZE) {
                        asError(lc, ERROR_IDENTIERTOOLONG, IDENTIFER_MAX_SIZE - 1);
                    }
                    lt->symbol = LEXICALSYMBOL_IDENTIFIER;
                    strncpy(lt->data.identifier, s, IDENTIFER_MAX_SIZE - 1);
                    lt->data.identifier[IDENTIFER_MAX_SIZE - 1] = 0;
                }
            }
        }
    }
    strbufferDestroy(s);
}

void lexicalReadDecimal(char **s, LexicalContext *lc, LexicalToken *lt) {
    _LEXICAL_DEBUG("ReadDecimal\n");
    lt->options.isDecimalFloat = false;
    int sign = 1;
    if (lc->currentChar == '-') {
        sign = -1;
        lexicalNextChar(lc);
        if (lexicalIsSpace(lc->currentChar)) {
            lt->symbol = LEXICALSYMBOL_INTERNAL;
            lt->data.internal = INTERNAL_SUB;
            return;
        } else if (lc->currentChar == '>') {
            lexicalNextChar(lc);
            lexicalReadIdent(lc, lt, strbufferClone("->"));
            return;
        }
    }
    /* read the source file until we found a
     *      non decimal digit character */
    while (lexicalIsDecimalDigit(lc->currentChar)) {
        *s = strbufferAppendChar(*s, lc->currentChar);
        lexicalNextChar(lc);
    }
    /* check if we have a floating point number */
    if (lc->currentChar == '.') {
        lt->options.isDecimalFloat = true;
        do {
            *s = strbufferAppendChar(*s, lc->currentChar);
            lexicalNextChar(lc);
        }        while (lexicalIsDecimalDigit(lc->currentChar));
    }
    /* decimal numbers MUST be followed by a separator or a space */
    if (!lexicalIsSeparator(lc->currentChar)) {
        strbufferDestroy(*s);
        asError(lc, ERROR_DECIMAL);
    }
    lt->symbol = LEXICALSYMBOL_DECIMAL;
    if (strlen(*s) > 0) {
        if (lt->options.isDecimalFloat) {
            lt->data.floatValue = strtod(*s, (char**) NULL) * sign;
        } else {
            lt->data.intValue = strtoll(*s, (char**) NULL, 10) * sign;
        }
    } else {
        lt->data.intValue = 0;
    }
}

void lexicalReadNumber(LexicalContext *lc, LexicalToken *lt) {
    _LEXICAL_DEBUG("ReadNumber\n");
    char *s = strbufferCreate();
    // check if we have an hexadecimal (start with 0x) or a decimal number
    if (lc->currentChar == '0') {
        lexicalNextChar(lc);
        // an hexadecimal number
        if (lc->currentChar == 'x') {
            _LEXICAL_DEBUG("ReadHexadecimal\n");
            lexicalNextChar(lc);
            /* read the source file until we found a
             non hexadecimal digit character */
            while (lexicalIsHexadecimentDigit(lc->currentChar)) {
                s = strbufferAppendChar(s, lc->currentChar);
                lexicalNextChar(lc);
            }
            /* hexadecimal numbers MUST be followed by a separator or a space */
            if (!lexicalIsSeparator(lc->currentChar)) {
                asError(lc, ERROR_HEXADECIMAL);
            }
            if (strlen(s) > 0) {
                lt->symbol = LEXICALSYMBOL_DECIMAL;
                lt->data.intValue = strtol(s, (char**) NULL, 16);
            } else {
                asError(lc, ERROR_INTERNAL);
            }
        }// a decimal number
        else {
            lexicalReadDecimal(&s, lc, lt);
        }
    }// a decimal number
    else {
        lexicalReadDecimal(&s, lc, lt);
    }
    strbufferDestroy(s);
}

void lexicalReadString(LexicalContext *lc, LexicalToken *lt) {
    _LEXICAL_DEBUG("ReadString\n");
    char *s = strbufferCreate();
    lexicalNextChar(lc);
    /* read the source file until end of string,
     *      or end of file (note : we can have multi lines strings) */
    while ((lc->currentChar != '"') &&
            (lc->currentChar != 0)) {
        // check if we have an escaped character
        if (lc->currentChar == '\\') {
            lexicalNextChar(lc);
            if (lc->currentChar != 0) {
                char char_to_add = lc->currentChar;
                /* we support escaped sequences defined in
                 ANSI X3.159-1989 (``ANSI C''), with extensions */
                // TODO: \num, \0num and \c
                switch (lc->currentChar) {
                    case 'a':
                        char_to_add = '\a';
                        break;
                    case 'b':
                        char_to_add = '\b';
                        break;
                    case 'f':
                        char_to_add = '\f';
                        break;
                    case 'n':
                        char_to_add = '\n';
                        break;
                    case 'r':
                        char_to_add = '\r';
                        break;
                    case 't':
                        char_to_add = '\t';
                        break;
                    case 'v':
                        char_to_add = '\v';
                        break;
                    default:
                        break;
                }
                s = strbufferAppendChar(s, char_to_add);
                lexicalNextChar(lc);
            } else {
                asError(lc, ERROR_BADESCAPESEQUENCE);
            }
        }// a simple character, just add it to the string
        else {
            s = strbufferAppendChar(s, lc->currentChar);
            lexicalNextChar(lc);
        }
    }
    // check if we have reached the end of file before the end of string
    if (lc->currentChar == '"') {
        lt->symbol = LEXICALSYMBOL_STRING;
        lt->data.string = s;
    } else {
        strbufferDestroy(s);
        asError(lc, ERROR_UNTERMINATEDSTRING);
    }
}

void lexicalNextToken(LexicalContext *lc, LexicalToken *lt) {
    _LEXICAL_DEBUG("NextToken\n");
    lexicalSkipSpace(lc);
    // reset the value of the previous symbol
    lt->symbol = LEXICALSYMBOL_UNKNOWN;
    lt->lineNumber = lc->currentLine;
    lt->startingChar = ftell(lc->file) - 1;
    // find the first character of the next symbol
    switch (lc->currentChar) {
        case 0:
            _LEXICAL_DEBUG("EOF\n");
            lt->symbol = LEXICALSYMBOL_EOF;
            break;
        case '!':
            _LEXICAL_DEBUG("!\n");
            lexicalNextChar(lc);
            if (lc->currentChar == '=') {
                long id = asIdentier2Internal("!=");
                if (id > -1) {
                    lt->symbol = LEXICALSYMBOL_INTERNAL;
                    lt->data.internal = (Internal) id;
                    break;
                }
            }
            break;
        case '<':
            _LEXICAL_DEBUG("<\n");
            lexicalNextChar(lc);
            if (lc->currentChar == '<') {
                lt->symbol = LEXICALSYMBOL_STARTFUNC;
                break;
            } else if (lc->currentChar == '=') {
                long id = asIdentier2Internal("<=");
                if (id > -1) {
                    lt->symbol = LEXICALSYMBOL_INTERNAL;
                    lt->data.internal = (Internal) id;
                    break;
                }
            } else if (lexicalIsSeparator(lc->currentChar)) {
                long id = asIdentier2Internal("<");
                if (id > -1) {
                    lt->symbol = LEXICALSYMBOL_INTERNAL;
                    lt->data.internal = (Internal) id;
                    break;
                }
            }
            asError(lc, ERROR_SYNTAX);
        case '>':
            _LEXICAL_DEBUG(">\n");
            lexicalNextChar(lc);
            if (lc->currentChar == '>') {
                lt->symbol = LEXICALSYMBOL_ENDFUNC;
                break;
            } else if (lc->currentChar == '=') {
                long id = asIdentier2Internal(">=");
                if (id > -1) {
                    lt->symbol = LEXICALSYMBOL_INTERNAL;
                    lt->data.internal = (Internal) id;
                    break;
                }
            } else if (lexicalIsSeparator(lc->currentChar)) {
                long id = asIdentier2Internal(">");
                if (id > -1) {
                    lt->symbol = LEXICALSYMBOL_INTERNAL;
                    lt->data.internal = (Internal) id;
                    break;
                }
            }
            asError(lc, ERROR_SYNTAX);
        case '"':
            _LEXICAL_DEBUG("\"\n");
            lexicalReadString(lc, lt);
            break;
        case '[':
            _LEXICAL_DEBUG("[\n");
            
            lt->symbol = LEXICALSYMBOL_STARTARRAY;
            break;
        case ']':
            _LEXICAL_DEBUG("]\n");
            lt->symbol = LEXICALSYMBOL_ENDARRAY;
            break;
        case '@':
            _LEXICAL_DEBUG("@\n");
            lt->symbol = LEXICALSYMBOL_REF;
            break;
        default:
            _LEXICAL_DEBUG("OTHER\n");
            /* decimal and hexadecimal (0x) numbers
             starts with a decimal digit */
            if (lexicalIsDecimalDigit(lc->currentChar) ||
                    (lc->currentChar == '-')) {
                lexicalReadNumber(lc, lt);
                lt->endingChar = ftell(lc->file) - 1;
                return;
            }
            /* all unknown symbols are identifiers */
            lexicalReadIdent(lc, lt, NULL);
            lt->endingChar = ftell(lc->file) - 1;
            return;
    }
    if (lc->currentChar == '\n') {
        lt->endingChar = ftell(lc->file) - 1;
    } else {
        lt->endingChar = ftell(lc->file);
    }
    lexicalNextChar(lc);
}

uint64_t lexicalCurrentLine(LexicalContext *lc) {
    return lc->currentLine;
}

LexicalToken* lexicalCloneToken(LexicalToken *lt) {
    LexicalToken *t = memAlloc(sizeof (LexicalToken));
    memcpy(t, lt, sizeof (LexicalToken));
    return t;
}

void lexicalDestroyToken(LexicalToken *lt) {
    memFree(lt);
}
