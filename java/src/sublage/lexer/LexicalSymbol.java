package sublage.lexer;

public enum LexicalSymbol {
    UNKNOWN,
    DECIMAL,     
    INTERNAL,     
    RESERVEDWORD,     
    IDENTIFIER,     
    STARTFUNC,     
    ENDFUNC,     
    STRING,     
    REF,     
    VARSET,     
    STARTARRAY,     
    ENDARRAY,     
    EOF,
    // the following symbols are used 
    // only when LexicalAnalyzer.allTokens == true
    ONELINECOMMENT,
    SPACE,
    SEPARATOR,
}
