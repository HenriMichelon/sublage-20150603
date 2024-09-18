#include <string.h>
#include "sublage/mem.h"
#include "sublage/strbuffer.h"
#include "sublage/loader.h"
#include "dump.h"
#include "syntax.h"
#include "errors.h"
#include "import.h"
#include "classes.h"

void syntaxCode(SyntaxContext *sc, LinkedList *list, bool inloop);

SyntaxContext* syntaxCreate(LexicalContext *lc) {
    SyntaxContext *sc = memAlloc(sizeof (SyntaxContext));
    sc->lexicalContext = lc;
    sc->currentFunction = NULL;
    sc->functions = linkedListCreate();
    sc->imports = linkedListCreate();
    sc->strings = linkedListCreate();
    sc->natives = linkedListCreate();
    sc->variables = linkedListCreate();
    sc->arrays = linkedListCreate();
    sc->classes = linkedListCreate();
    return sc;
}

SyntaxContext* syntaxInit(LexicalContext *lc) {
    SyntaxContext *sc =syntaxCreate(lc);
    lexicalNextToken(sc->lexicalContext, &sc->nextToken);
    memcpy(&sc->currentToken, &sc->nextToken, sizeof (LexicalToken));
    return sc;
}

void syntaxDone(SyntaxContext *sc) {
    LinkedListIterator *lli = linkedListCreateIterator(sc->functions);
    Function *f = NULL;
    while ((f = linkedListIteratorNext(lli)) != NULL) {
        functionDestroy(f);
    }
    linkedListIteratorDestroy(lli);
    linkedListDestroy(sc->functions, false);

    lli = linkedListCreateIterator(sc->imports);
    Import *i = NULL;
    while ((i = linkedListIteratorNext(lli)) != NULL) {
        importDestroy(i);
    }
    linkedListIteratorDestroy(lli);
    linkedListDestroy(sc->imports, false);

    linkedListDestroy(sc->natives, true);
    linkedListDestroy(sc->variables, true);

    lli = linkedListCreateIterator(sc->strings);
    char *s = NULL;
    while ((s = linkedListIteratorNext(lli)) != NULL) {
        strbufferDestroy(s);
    }
    linkedListIteratorDestroy(lli);
    lli = linkedListCreateIterator(sc->arrays);
    LinkedList *array = NULL;
    while ((array = linkedListIteratorNext(lli)) != NULL) {
        linkedListDestroy(array, true);
    }
    linkedListIteratorDestroy(lli);
    lli = linkedListCreateIterator(sc->classes);
    Class *c = NULL;
    while ((c = linkedListIteratorNext(lli)) != NULL) {
        classDestroy(c);
    }
    linkedListIteratorDestroy(lli);
    linkedListDestroy(sc->strings, false);
    linkedListDestroy(sc->arrays, false);
    linkedListDestroy(sc->classes, false);

    memFree(sc);
}

void syntaxRead(SyntaxContext *sc, LexicalSymbol symbol, ErrorCode err) {
    //dumpLexicalToken(stdout, &sc->nextToken);
    if (symbol == sc->nextToken.symbol) {
        memcpy(&sc->currentToken, &sc->nextToken, sizeof (LexicalToken));
        lexicalNextToken(sc->lexicalContext, &sc->nextToken);
    } else {
        asError(sc->lexicalContext, err);
    }
}

void syntaxIdentifier(SyntaxContext *sc) {
    syntaxRead(sc, LEXICALSYMBOL_IDENTIFIER, ERROR_EXPECTEDIDENTIFIER);
}

/* ctrl_if ::= "if" "<<" code ">>" [ "else" "<<" code ">>" ] */
void syntaxIf(SyntaxContext *sc, LinkedList *list, bool inloop) {
    LexicalToken *tokenIf = lexicalCloneToken(&sc->currentToken);
    tokenIf->options.code.code = linkedListCreate();
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, tokenIf->options.code.code, inloop);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    if ((sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD)
        && (sc->nextToken.data.word == RESERVEDWORD_ELSE)) {
        syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
        tokenIf->options.code.code2 = linkedListCreate();
        syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
        while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
            syntaxCode(sc, tokenIf->options.code.code2, inloop);
        }
        syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    } else {
        tokenIf->options.code.code2 = NULL;
    }
    linkedListAppend(list, lexicalCloneToken(tokenIf));
    lexicalDestroyToken(tokenIf);
}

/* ctrl_foreach ::= "foreach" "<<" code ">>" */
void syntaxForeach(SyntaxContext *sc, LinkedList *list) {
    LexicalToken *tokenForeach = lexicalCloneToken(&sc->currentToken);
    tokenForeach->options.code.code = linkedListCreate();
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    linkedListAppend(list, lexicalCloneToken(tokenForeach));
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, tokenForeach->options.code.code, true);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
    lexicalDestroyToken(tokenForeach);
}

/* ctrl_for ::= "for" "<<" code ">>" */
void syntaxFor(SyntaxContext *sc, LinkedList *list) {
    LexicalToken *tokenFor = lexicalCloneToken(&sc->currentToken);
    tokenFor->options.code.code = linkedListCreate();
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    linkedListAppend(list, lexicalCloneToken(tokenFor));
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, tokenFor->options.code.code, true);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
    lexicalDestroyToken(tokenFor);
}

/* ctrl_while ::= "while" "<<" code ">>" do "<<" code ">>" */
void syntaxWhile(SyntaxContext *sc, LinkedList *list) {
    LexicalToken *tokenWhile = lexicalCloneToken(&sc->currentToken);
    tokenWhile->options.code.code = linkedListCreate();
    tokenWhile->options.code.code2 = linkedListCreate();
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, tokenWhile->options.code.code2, false);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_DOEXPECTED);
    if (sc->nextToken.data.word != RESERVEDWORD_DO) {
        asError(sc->lexicalContext, ERROR_DOEXPECTED);
    }
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    linkedListAppend(list, lexicalCloneToken(tokenWhile));
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, tokenWhile->options.code.code, true);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
    lexicalDestroyToken(tokenWhile);
}

/* ctrl_do ::= "do" "<<" code ">>" "while" "<<" code ">>" */
void syntaxDo(SyntaxContext *sc, LinkedList *list) {
    LexicalToken *tokenDo = lexicalCloneToken(&sc->currentToken);
    tokenDo->options.code.code = linkedListCreate();
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, tokenDo->options.code.code, true);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_WHILEEXPECTED);
    if (sc->nextToken.data.word != RESERVEDWORD_WHILE) {
        asError(sc->lexicalContext, ERROR_WHILEEXPECTED);
    }
    tokenDo->options.code.code2 = linkedListCreate();
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, tokenDo->options.code.code2, false);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    linkedListAppend(list, lexicalCloneToken(tokenDo));
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
    lexicalDestroyToken(tokenDo);
}

/* code ::= empty_line | ... */
void syntaxCode(SyntaxContext *sc, LinkedList *list, bool inloop) {
    switch (sc->nextToken.symbol) {
        case LEXICALSYMBOL_IDENTIFIER:
            syntaxRead(sc, LEXICALSYMBOL_IDENTIFIER, ERROR_INTERNAL);
            break;
        case LEXICALSYMBOL_INTERNAL:
            syntaxRead(sc, LEXICALSYMBOL_INTERNAL, ERROR_INTERNAL);
            break;
        case LEXICALSYMBOL_DECIMAL:
            syntaxRead(sc, LEXICALSYMBOL_DECIMAL, ERROR_INTERNAL);
            break;
        case LEXICALSYMBOL_STRING:
            syntaxRead(sc, LEXICALSYMBOL_STRING, ERROR_INTERNAL);
            break;
        case LEXICALSYMBOL_REF:
            syntaxRead(sc, LEXICALSYMBOL_REF, ERROR_INTERNAL);
            syntaxIdentifier(sc);
            sc->currentToken.symbol = LEXICALSYMBOL_REF;
            break;
        case LEXICALSYMBOL_STARTARRAY:
        {
            syntaxRead(sc, LEXICALSYMBOL_STARTARRAY, ERROR_INTERNAL);
            LinkedList *array = linkedListCreate();
            sc->currentToken.data.array = array;
            linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
            while (sc->nextToken.symbol != LEXICALSYMBOL_ENDARRAY) {
                switch (sc->nextToken.symbol) {
                    case LEXICALSYMBOL_DECIMAL:
                    case LEXICALSYMBOL_STRING:
                        syntaxRead(sc, sc->nextToken.symbol, ERROR_INTERNAL);
                        break;
                    case LEXICALSYMBOL_RESERVEDWORD:
                        switch (sc->nextToken.data.word) {
                            case RESERVEDWORD_NULL:
                            case RESERVEDWORD_TRUE:
                            case RESERVEDWORD_FALSE:
                                syntaxRead(sc, sc->nextToken.symbol, ERROR_INTERNAL);
                                break;
                            default:
                                asError(sc->lexicalContext, ERROR_SYNTAX);
                                break;
                        }
                        break;
                    case LEXICALSYMBOL_REF:
                        syntaxRead(sc, LEXICALSYMBOL_REF, ERROR_INTERNAL);
                        syntaxIdentifier(sc);
                        sc->currentToken.symbol = LEXICALSYMBOL_REF;
                        break;
                    default:
                        asError(sc->lexicalContext, ERROR_SYNTAX);
                        break;
                }
                linkedListAppend(array, lexicalCloneToken(&sc->currentToken));
            }
            syntaxRead(sc, LEXICALSYMBOL_ENDARRAY, ERROR_INTERNAL);
        }
            return;
        case LEXICALSYMBOL_RESERVEDWORD:
            syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
            switch (sc->currentToken.data.word) {
                case RESERVEDWORD_IF:
                    syntaxIf(sc, list, inloop);
                    return;
                case RESERVEDWORD_WHILE:
                    syntaxWhile(sc, list);
                    return;
                case RESERVEDWORD_DO:
                    syntaxDo(sc, list);
                    return;
                case RESERVEDWORD_FOR:
                    syntaxFor(sc, list);
                    return;
                case RESERVEDWORD_FOREACH:
                    syntaxForeach(sc, list);
                    return;
                case RESERVEDWORD_BREAK:
                case RESERVEDWORD_CONTINUE:
                    if (!inloop) {
                        asError(sc->lexicalContext,
                            ERROR_BREAKCONTINUEOUTSIDELOOP,
                            asReservedWord2Identifier(sc->currentToken.data.word));
                    }
                    break;
                default:
                    break;
            }
            break;
        case LEXICALSYMBOL_VARSET:
            syntaxRead(sc, LEXICALSYMBOL_VARSET, ERROR_INTERNAL);
            break;
        default:
            asError(sc->lexicalContext, ERROR_SYNTAX);
    }
    linkedListAppend(list, lexicalCloneToken(&sc->currentToken));
}


void syntaxFunctionCode(SyntaxContext *sc) {
    sc->currentFunction = functionCreate(sc->currentToken.data.identifier);
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        syntaxCode(sc, sc->currentFunction->code, false);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    linkedListAppend(sc->currentFunction->code, lexicalCloneToken(&sc->currentToken));
}

/* function ::= name << { code } >> */
void syntaxFunction(SyntaxContext *sc) {
    syntaxIdentifier(sc); 
    syntaxFunctionCode(sc);
    linkedListAppend(sc->functions, sc->currentFunction);
    sc->currentFunction = NULL;
}

/* import_file ::= import "filename" [ as identifier ] */
void syntaxImport(SyntaxContext *sc) {
    syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
    syntaxRead(sc, LEXICALSYMBOL_IDENTIFIER, ERROR_EXPECTEDIDENTIFIER);
    char* name = strbufferClone(sc->currentToken.data.identifier);
    BinExecFile *bef = loaderLoadFileFromFileName(name);
    if (bef == NULL) {
        strbufferDestroy(name);
        asError(sc->lexicalContext, ERROR_CANTLOADIMPORT,
            sc->currentToken.data.identifier);
    }
    Import *i = importCreate(name, bef);
    importSetAlias(i, name);
    // alias
    /*if ((sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD)
            && (sc->nextToken.data.word == RESERVEDWORD_AS)) {
        syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
        syntaxRead(sc, LEXICALSYMBOL_IDENTIFIER, ERROR_EXPECTEDIDENTIFIER);
        importSetAlias(i, sc->currentToken.data.identifier);
    }*/
    linkedListAppend(sc->imports, i);
}

/*  native ::= native identifier */
void syntaxNative(SyntaxContext *sc) {
    syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
    syntaxIdentifier(sc);
    linkedListAppend(sc->natives, strbufferClone(sc->currentToken.data.identifier));
}

/*  var ::= var identifier */
void syntaxVariable(SyntaxContext *sc) {
    syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
    // TODO: 2^32 max variables check
    syntaxIdentifier(sc);
    linkedListAppend(sc->variables, strbufferClone(sc->currentToken.data.identifier));
}

/* method :: =  [ "(" ] identifier [ ")" ] "<<" { code } ">>" */
void syntaxMethod(SyntaxContext *sc, bool readIdentifier, Class *c) {
    if (readIdentifier) { 
        syntaxIdentifier(sc); 
    }
    syntaxFunctionCode(sc);
    classAddFunction(c, sc->currentFunction);
    sc->currentFunction = NULL;    
}

/* ivar::= ivar identifier [read [ identifier ] ] [write [ identifier ] ] */
void syntaxInstanceVariable(SyntaxContext *sc, Class *c) {
    syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
    // TODO: 2^32 max variables check
    syntaxIdentifier(sc);
    InstanceVariable *iv = ivarCreate(sc->currentToken.data.identifier);
    bool haveAccessLevel1 = false;
    if (sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD) {
        bool read = false;
        if (sc->nextToken.data.word == RESERVEDWORD_READ) {
            haveAccessLevel1 = true;
            iv->readAccess = true;
            read = true;
        } else if (sc->nextToken.data.word == RESERVEDWORD_WRITE) {
            haveAccessLevel1 = true;
            iv->writeAccess = true;
        }
        if (haveAccessLevel1) {
            syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
            if (sc->nextToken.symbol == LEXICALSYMBOL_IDENTIFIER) {
                syntaxIdentifier(sc);
                if (sc->nextToken.symbol == LEXICALSYMBOL_STARTFUNC) {
                    classAddVariable(c, iv);
                    syntaxMethod(sc, false, c);
                    return;
                }
                if (read) {
                    strbufferDestroy(iv->readMethod);
                    iv->readMethod = strbufferClone(sc->currentToken.data.identifier);
                } else {
                    strbufferDestroy(iv->writeMethod);
                    iv->writeMethod = strbufferClone(sc->currentToken.data.identifier);
                }
            }
        }
    }
    bool haveAccessLevel2 = false;
    if (sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD) {
        bool read = false;
        if ((sc->nextToken.data.word == RESERVEDWORD_READ) &&
            (!iv->readAccess)) {
            haveAccessLevel2 = true;
            iv->readAccess = true;
            read = true;
        } else if ((sc->nextToken.data.word == RESERVEDWORD_WRITE) &&
                    (!iv->writeAccess)) {
            haveAccessLevel2 = true;
            iv->writeAccess = true;
        }
        if (haveAccessLevel2) {
            syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
            if (sc->nextToken.symbol == LEXICALSYMBOL_IDENTIFIER) {
                syntaxIdentifier(sc);
                if (sc->nextToken.symbol == LEXICALSYMBOL_STARTFUNC) {
                    classAddVariable(c, iv);
                    syntaxMethod(sc, false, c);
                    return;
                }
                if (read) {
                    strbufferDestroy(iv->readMethod);
                    iv->readMethod = strbufferClone(sc->currentToken.data.identifier);
                } else {
                    strbufferDestroy(iv->writeMethod);
                    iv->writeMethod = strbufferClone(sc->currentToken.data.identifier);
                }
            }
        }
    }
    if ((!haveAccessLevel1) && (!haveAccessLevel2)) {
        asError(sc->lexicalContext, ERROR_EXPECTEDREADORWRITE);
    }
    classAddVariable(c, iv);
}

/* class ::= class [ extends identifier ] { ivar | method } */
void syntaxClass(SyntaxContext *sc) {
    syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_INTERNAL);
    syntaxIdentifier(sc);
    Class *c = classCreate(sc->currentToken.data.identifier, -1);
    if (sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD) {
        syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_SYNTAX);
        if (sc->currentToken.data.word != RESERVEDWORD_EXTENDS) {
            syntaxRead(sc, LEXICALSYMBOL_RESERVEDWORD, ERROR_SYNTAX);
        }
        syntaxIdentifier(sc);
        classSetParent(c, sc->currentToken.data.identifier);
    }
    syntaxRead(sc, LEXICALSYMBOL_STARTFUNC, ERROR_EXPECTEDSTARTFUNC);
    while (sc->nextToken.symbol != LEXICALSYMBOL_ENDFUNC) {
        if (sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD) {
            if (sc->nextToken.data.word == RESERVEDWORD_VAR) {
                syntaxInstanceVariable(sc, c);
                continue;
            }
        } else if (sc->nextToken.symbol == LEXICALSYMBOL_EOF) {
            asError(sc->lexicalContext, ERROR_UNEXPECTEDEOFINCLASS);
            classDestroy(c);
            return;
        }
        syntaxMethod(sc, true, c);
    }
    syntaxRead(sc, LEXICALSYMBOL_ENDFUNC, ERROR_INTERNAL);
    linkedListAppend(sc->classes, c);
}

/* asm_source ::= { import_file } { function | native | var | class } */
void syntaxAnalyse(SyntaxContext *sc) {
    while ((sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD)
        && (sc->nextToken.data.word == RESERVEDWORD_IMPORT)) {
        syntaxImport(sc);
    }
    while (sc->nextToken.symbol != LEXICALSYMBOL_EOF) {
        if (sc->nextToken.symbol == LEXICALSYMBOL_RESERVEDWORD) {
            if (sc->nextToken.data.word == RESERVEDWORD_NATIVE) {
                syntaxNative(sc);
                continue;
            } else if (sc->nextToken.data.word == RESERVEDWORD_VAR) {
                syntaxVariable(sc);
                continue;
            } else if (sc->nextToken.data.word == RESERVEDWORD_CLASS) {
                syntaxClass(sc);
                continue;
            }
        }
        syntaxFunction(sc);
    }
}
