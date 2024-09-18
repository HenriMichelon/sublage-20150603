#include "dump.h"

void dumpLexicalToken(FILE *f, LexicalToken *lt) {
    switch (lt->symbol) {
            break;
        case LEXICALSYMBOL_EOF:
            fprintf(f, "eof");
            break;
        case LEXICALSYMBOL_DECIMAL:
            if (lt->options.isDecimalFloat) {
                fprintf(f, "%f ", lt->data.floatValue);
            } else {
                fprintf(f, "%lld ", lt->data.intValue);
            }
            break;
        case LEXICALSYMBOL_STRING:
            fprintf(f, "\"%s\" ", lt->data.string);
            break;
        case LEXICALSYMBOL_INTERNAL:
            fprintf(f, "%s ", asOpcode2Identifier(lt->data.internal));
            break;
        case LEXICALSYMBOL_RESERVEDWORD:
            fprintf(f, "%s", asReservedWord2Identifier(lt->data.word));
            break;
        case LEXICALSYMBOL_IDENTIFIER:
            fprintf(f, "%s ", lt->data.identifier);
            break;
        case LEXICALSYMBOL_STARTFUNC:
            fprintf(f, "<< ");
            break;
        case LEXICALSYMBOL_ENDFUNC:
            fprintf(f, ">> ");
            break;
        case LEXICALSYMBOL_STARTARRAY:
            fprintf(f, "[ ");
            break;
         case LEXICALSYMBOL_ENDARRAY:
            fprintf(f, "] ");
            break;
         case LEXICALSYMBOL_VARSET:
            fprintf(f, "-> ");
            break;
      case LEXICALSYMBOL_UNKNOWN:
        default:
            fprintf(f, "??");
            break;
    }
}

void dumpLexicalContext(FILE *f, LexicalContext *lc) {
    fprintf(f, "LC: 0x%x, %lld", lc->currentChar, lc->currentLine);
}
