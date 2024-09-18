package sublage.lang;

public enum ReservedWord {
    
    NULL    ("null"),
    IMPORT  ("import"),
    AS      ("as"),
    TRUE    ("true"),
    FALSE   ("false"),
    RETURN  ("return"),
    IF      ("if"),
    ELSE    ("else"),
    WHILE   ("while"),
    DO      ("do"),
    NATIVE  ("native"),
    VAR     ("var"),
    BREAK   ("break"),
    CONTINUE("continue"),
    FOR     ("for"),
    FOREACH ("foreach"),
    CLASS   ("class"),
    READ    ("read"),
    WRITE   ("write"),
    EXTENDS ("extends");
    
    private final String identifier;

    private ReservedWord(String identifier) {
        this.identifier = identifier;
    }
    
    public static ReservedWord toReservedWord(String identifier) {
        for (ReservedWord word : ReservedWord.values()) {
            if (word.identifier.equals(identifier)) {
                return word;
            }
        }
        return null;
    }

    @Override
    public String toString() {
        return this.identifier;
    }   

}
