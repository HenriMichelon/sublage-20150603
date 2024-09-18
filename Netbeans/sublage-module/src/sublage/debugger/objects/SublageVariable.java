package sublage.debugger.objects;

import sublage.lang.Opcode;

public class SublageVariable {

    private String name = "(unknown)";
    private Opcode opcode = Opcode.NULL;
    private String value = "(unknown)";

    public SublageVariable(String state) {
        int pos = state.indexOf(':');
        if (pos >= 0) {
            name = state.substring(0, pos);
            state = state.substring(pos + 1);
            pos = state.indexOf(',');
            if (pos >= 0) {
                opcode = Opcode.toOpcode(Integer.parseInt(state.substring(0, pos)));
                value = state.substring(pos + 1);
            }
        }
    }

    public Opcode getOpcode() {
        return opcode;
    }


    public String getName() {
        return name;
    }

    public String getValue() {
        return value;
    }
}