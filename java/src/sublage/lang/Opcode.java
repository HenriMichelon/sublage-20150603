package sublage.lang;

public enum Opcode {
    NULL        ( 0x00, "null"),
    BOOLEAN     ( 0x01, "boolean"),
    INT         ( 0x02, "integer"),
    FLOAT       ( 0x03, "floating point"),
    STRING      ( 0x04, "string"),
    ARRAY       ( 0x05, "array"),
    
    INTERNAL        ( 0x06, "internal"),
    INTERNAL_CALL   ( 0x07, "internal call"),
    EXTERNAL_CALL   ( 0x08, "external call"),
    RETURN          ( 0x09, "return"),
    JUMP            ( 0x0a, "jump"),
    JUMP_IF_NOT     ( 0x0b, "jump if not"),
    JUMP_IF         ( 0x0c, "jump if"),
    NATIVE_CALL     ( 0x0d, "native call"),
    INTERNAL_CALL_REF   ( 0x0e, "@internal call"),
    EXTERNAL_CALL_REF   ( 0x0f, "@external call"),
    NATIVE_CALL_REF     ( 0x10, "@native call"),
    PRIVATE             ( 0x11, "private datas"),
    VAR_SET             ( 0x12, "->var"),
    VAR_GET             ( 0x13, "var->"),
    VAR_REF             ( 0x14, "@var"),
    DATA                ( 0x15, "binary datas"),
    CLASS_REF           ( 0x16, "@class"),
    IFIELD_SET          ( 0x17, "->.ifield"),
    IFIELD_CALL         ( 0x18, ".ifield"),
    IFIELD_CALL_REF     ( 0x19, "@.ifield"),
    INSTANCE            ( 0x1a, "class instance");
    
    
    private final int opcode;
    private final String textual;

    public int getOpcode() {
        return opcode;
    }

    private Opcode(int opcode, String textual) {
        this.opcode = opcode;
        this.textual = textual;
    }
    
     public static Opcode toOpcode(int code) {
        for (Opcode opcode : Opcode.values()) {
            if (opcode.opcode == code) {
                return opcode;
            }
        }
        return null;
    }

    @Override
    public String toString() {
        return textual;
    }

}
