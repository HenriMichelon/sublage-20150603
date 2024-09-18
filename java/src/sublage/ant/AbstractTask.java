package sublage.ant;

import java.io.BufferedReader;
import java.io.File;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.Map;
import org.apache.tools.ant.BuildException;
import org.apache.tools.ant.Task;

public abstract class AbstractTask extends Task {

    private String stdlib;

    public void setStdlib(String stdlib) {
        this.stdlib = stdlib;
    }

    public String getStdlib() {
        return stdlib;
    }

    boolean start(ProcessBuilder proc) {
        if (stdlib != null) {
            Map<String, String> env = proc.environment();
            env.put("LD_LIBRARY_PATH",
                    stdlib + File.pathSeparator + env.get("LD_LIBRARY_PATH"));
            env.put("LD_LIBRARY_PATH",
                    getProject().getBaseDir() + File.pathSeparator + env.get("LD_LIBRARY_PATH"));
        }
        try {
            Process p = proc.start();
            String line;
            BufferedReader out = new BufferedReader(new InputStreamReader(p.getInputStream()));
            while ((line = out.readLine()) != null) {
                System.out.println("\t" + line);
            }
            p.waitFor();
            BufferedReader err = new BufferedReader(new InputStreamReader(p.getErrorStream()));
            while ((line = err.readLine()) != null) {
                System.err.println("\t" + line);
            }
            return p.exitValue() == 0;
        } catch (IOException ex) {
            throw new BuildException(ex);
        } catch (InterruptedException ex) {
            throw new BuildException(ex);
        }
    }
}
