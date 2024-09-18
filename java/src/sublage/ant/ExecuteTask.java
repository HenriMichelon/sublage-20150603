package sublage.ant;

import java.io.File;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import org.apache.tools.ant.BuildException;

public class ExecuteTask extends AbstractTask {

    private String binary;
    private String binariespath = "binaries";
    private String sublage = "sublage";
    private String stackdump;
    private String stackdumpsocket;
    private String debugger;
    private String debuggerport;

    public String getDebuggerport() {
        return debuggerport;
    }

    public void setDebuggerport(String debuggerport) {
        this.debuggerport = debuggerport;
    }

    public void setDebugger(String debugger) {
        this.debugger = debugger;
    }

    public String getDebugger() {
        return debugger;
    }

    public void setStackdump(String stackdump) {
        this.stackdump = stackdump;
    }

    public void setStackdumpsocket(String stackdumpsocket) {
        this.stackdumpsocket = stackdumpsocket;
    }

    public void setBinary(String binary) {
        this.binary = binary;
    }

    public void setSublage(String sublage) {
        this.sublage = sublage;
    }

    public void setBinariespath(String binariespath) {
        this.binariespath = binariespath;
    }

    public @Override
    void execute() throws BuildException {
        if ((binary == null) || (binary.length() == 0)) {
            throw new BuildException("`binary` attribute is required.");
        }

        List<String> args = new ArrayList<String>();
        args.add(sublage);
        if ((debugger != null) && (!debugger.isEmpty())) {
            args.add("-d");
            args.add(debugger);
            if (debuggerport != null) {
                args.add("-p");
                args.add(debuggerport);
            }
        }
        if ((stackdump != null) && (!stackdump.isEmpty())) {
            args.add("-s");
            args.add(stackdump);
            if (stackdumpsocket != null) {
                args.add("-q");
                args.add(stackdumpsocket);
            }
        }
        args.add(binary);
        ProcessBuilder proc = new ProcessBuilder(args);
        proc.directory(getProject().getBaseDir());
        Map<String, String> env = proc.environment();
        if (binariespath != null) {
            env.put("PATH",
                    binariespath + File.pathSeparator + env.get("PATH"));
            env.put("LD_LIBRARY_PATH",
                    binariespath + File.pathSeparator + env.get("LD_LIBRARY_PATH"));
        }        
        start(proc);
    }
}
