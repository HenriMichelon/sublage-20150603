package sublage.debugger.objects;

public class SublageThread {
    
    private int id = -1;
    private boolean running = false;
    private String codeIndex = "(unknown)";
    private String source = "(unknown)";
    private int line = -1;
    private String text = "";

    public SublageThread(String state) {
        int pos = state.indexOf(':');
        if (pos >= 0) {
            id = Integer.parseInt(state.substring(0, pos));
            state = state.substring(pos + 1);
            String parts[] = state.split(",");
            if (parts.length >= 4) {
                running = parts[0].equals("running");
                codeIndex = parts[1];
                source = parts[2];
                line = Integer.parseInt(parts[3]);
            }
            if (parts.length >= 5) {
                text = parts[4];
            }
        }
    }

    public String getCodeIndex() {
        return codeIndex;
    }

    public void setRunning(boolean running) {
        this.running = running;
    }

    public boolean isRunning() {
        return running;
    }

    public int getId() {
        return id;
    }

    public int getLine() {
        return line;
    }

    public String getText() {
        return text;
    }

    public String getSource() {
        return source;
    }

}