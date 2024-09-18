package sublage.lang;

public enum Internal {
    NOP     ( 0x00, "nop"),
    ADD     ( 0x01, "+"),
    SUB     ( 0x02, "-"),
    MUL     ( 0x03, "*"),
    DIV     ( 0x04, "/"),
    SWAP    ( 0x05, "swap"),
    DROP    ( 0x06, "drop"),
    DUP     ( 0x07, "dup"),
    EQ      ( 0x08, "="),
    NEQ     ( 0x09, "!="),
    NOT     ( 0x0a, "not"),
    AND     ( 0x0b, "and"),
    OR      ( 0x0c, "or"),
    GT      ( 0x0d,">"),
    LT      ( 0x0e, "<"),
    GE      ( 0x0f, ">="),
    LE      ( 0x10, "<="),
    EXEC    ( 0x11, "exec"),
    DUPN    ( 0x12, "dupn"),
    OVER    ( 0x13, "over"),
    CSTR    ( 0x14, "->string"),
    CINT    ( 0x15, "->int"),
    CFLOAT  ( 0x16, "->float"),
    MODULO  ( 0x17, "%" ),
    DEPTH   ( 0x18, "depth" ),
    ROLL    ( 0x19, "roll" ),
    ROLLD   ( 0x1a, "rolld" ),
    ROLL3   ( 0x1b, "roll3" ),
    ROLLD3  ( 0x1c, "rolld3" ),
    PICK    ( 0x1d, "pick" ),
    PICK3   ( 0x1e, "pick3" ),
    DUP2    ( 0x1f, "dup2" ),
    DUP3    ( 0x20, "dup3" ),
    DROPN   ( 0x21, "dropn" ),
    DROP2   ( 0x22, "drop2" ),
    DROP3   ( 0x23, "drop3" ),
    CLEAR   ( 0x24, "clear" ),
    UNPICK  ( 0x25, "unpick" ),
    CARRAY  ( 0x26, "->[]" ),
    PICK2   ( 0x27, "pick2" ),
    ISNULL    ( 0x28, "isnull" ),
    ISNOTNULL ( 0x29, "isnotnull" ),
    EXPLODE   ( 0x2a, "explode" ),
    NEW       ( 0x2b, "new"),
    SELF      ( 0x2c, "self"),
    SUPER     ( 0x2d, "super");
    
    private final int opcode;
    private final String identifier;

    public int getOpcode() {
        return opcode;
    }

    private Internal(int opcode, String identifier) {
        this.opcode = opcode;
        this.identifier = identifier;
    }
    
    public static Internal toInternal(String identifier) {
        for (Internal internal : Internal.values()) {
            if (internal.identifier.equals(identifier)) {
                return internal;
            }
        }
        return null;
    }

    @Override
    public String toString() {
        return identifier;
    }
    
    
    

}
