package sublage.project;

import static sublage.project.SublageProject.PROP_STDLIB_DIRECTORY;
import static sublage.project.SublageProject.PROP_SUBLAGEC_BINARY;
import static sublage.project.SublageProject.PROP_SUBLAGE_BINARY;
import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Properties;
import java.util.prefs.Preferences;
import javax.swing.JOptionPane;
import org.apache.tools.ant.module.api.support.ActionUtils;
import org.netbeans.spi.project.ActionProvider;
import static org.netbeans.spi.project.ActionProvider.COMMAND_BUILD;
import static org.netbeans.spi.project.ActionProvider.COMMAND_CLEAN;
import static org.netbeans.spi.project.ActionProvider.COMMAND_DEBUG;
import static org.netbeans.spi.project.ActionProvider.COMMAND_REBUILD;
import static org.netbeans.spi.project.ActionProvider.COMMAND_RUN;
import org.openide.awt.StatusDisplayer;
import org.openide.execution.ExecutorTask;
import org.openide.filesystems.FileObject;
import org.openide.util.NbPreferences;

public class AntHelper implements Runnable {

    private Properties properties = new Properties();
    private String command;
    private int debuggerPort = -1;
    private ExecutorTask runTarget;
    private final SublageProject project;
    private final StatusDisplayer statusBar = StatusDisplayer.getDefault();

    private enum CommandTarget {

        CLEAN(COMMAND_CLEAN, "clean"),
        RUN(COMMAND_RUN, "run"),
        DEBUG(COMMAND_DEBUG, "debug"),
        BUILD(COMMAND_BUILD, "compile"),
        REBUILD(COMMAND_REBUILD, new String[]{"clean", "compile"});
        private final String command;
        private final String[] prjCmds;

        private CommandTarget(String command, String prjTarget) {
            this.command = command;
            this.prjCmds = new String[]{prjTarget};
        }

        public String getCommand() {
            return command;
        }

        private CommandTarget(String command, String[] prjTargets) {
            this.command = command;
            this.prjCmds = prjTargets;
        }

        public String[] getTargets() {
            String[] targets = new String[prjCmds.length];
            System.arraycopy(prjCmds, 0, targets, 0, prjCmds.length);
            return targets;
        }
    }

    public AntHelper(SublageProject project, String command) {
        this.project = project;
        this.command = command;
    }

    public AntHelper(SublageProject project, String command, int debuggerPort) {
        this(project, command);
        this.debuggerPort = debuggerPort;
    }

    public void setCommand(String command) {
        this.command = command;
    }

    private FileObject findBuildXml() {
        return project.getProjectDirectory().getFileObject("build.xml");
    }

    public void stop() {
        if (runTarget != null) {
            runTarget.stop();
        }
    }

    public int result() {
        return runTarget.result();
    }

    public Properties getProperties() {
        return properties;
    }

    String[] getTargetNames(String command) throws IllegalArgumentException {
        for (CommandTarget t : CommandTarget.values()) {
            if (t.getCommand().equals(command)) {
                return t.getTargets();
            }
        }
        return new String[0];
    }

    public boolean runTask() throws IOException {
        String[] targetNames = getTargetNames(command);
        if ((targetNames == null) || (targetNames.length == 0)) {
            return false;
        }
        if (project.isCleanNeeded()) {
            String[] cleantargets = getTargetNames(ActionProvider.COMMAND_CLEAN);
            String[] newtargets = Arrays.copyOf(cleantargets,
                    cleantargets.length + targetNames.length);
            int i = cleantargets.length;
            for (String target : targetNames) {
                newtargets[i++] = target;
            }
            targetNames = newtargets;
        }
        FileObject buildFo = findBuildXml();
        if (buildFo == null) {
            statusBar.setStatusText("build.xml not found");
            return false;
        }
        runTarget = ActionUtils.runTarget(buildFo, targetNames, properties);
        return true;
    }

    @Override
    public void run() {
        try {
            if (runTask()) {
                if (runTarget.result() != 0) {
                    statusBar.setStatusText("Error executing command " + command);
                } else {
                    project.setCleanNeeded(false);
                    statusBar.setStatusText("Command " + command + " executed succefully");
                }
            }
        } catch (IOException e) {
            statusBar.setStatusText("Error executing command " + command);
        }
    }

    private File searchInPath(String command, String variable) {
        String path = System.getenv(variable);
        for (String dir : path.split(File.pathSeparator)) {
            File file = new File(dir + File.separator + command);
            if (file.isFile()) {
                return file;
            }
        }
        return null;
    }

    private File searchForStdlib(File command, String library) {
        File stdlib;
        String base = command.getParent();
        if (base != null) {
            stdlib = new File(base.replaceAll(
                    File.separator + "bin$",
                    File.separator + "lib")
                    + File.separator + "sublage"
                    + File.separator + "stdlib"
                    + File.separator + library);
            if (!stdlib.isFile()) {
                return null;
            }
            return stdlib;
        }
        return null;
    }

    private boolean isWindows() {
        return System.getProperty("os.name").toLowerCase().indexOf("windows") >= 0;
    }

    private void loadPreferences() {
        Preferences prefs = NbPreferences.forModule(SublageProject.class);
        String sublage = prefs.get(SublageProject.PROP_SUBLAGE_BINARY, null);
        String sublagec = prefs.get(SublageProject.PROP_SUBLAGEC_BINARY, null);
        String stdlib = prefs.get(SublageProject.PROP_STDLIB_DIRECTORY, null);
        String main = prefs.get(SublageProject.PROP_MAIN_BINARY, null);
        String stackdump = prefs.get(SublageProject.PROP_STACK_DUMP, null);
        boolean debugOn = prefs.getBoolean(SublageProject.PROP_DEBUG, true);
        if (sublage != null) {
            properties.setProperty(SublageProject.PROP_SUBLAGE_BINARY, sublage);
        } else {
            properties.remove(SublageProject.PROP_SUBLAGE_BINARY);
        }
        if (sublagec != null) {
            properties.setProperty(SublageProject.PROP_SUBLAGEC_BINARY, sublagec);
        } else {
            properties.remove(SublageProject.PROP_SUBLAGEC_BINARY);
        }
        if (stdlib != null) {
            properties.setProperty(SublageProject.PROP_STDLIB_DIRECTORY, stdlib);
        } else {
            properties.remove(SublageProject.PROP_STDLIB_DIRECTORY);
        }
        if (stackdump != null) {
            properties.setProperty(SublageProject.PROP_STACK_DUMP, stackdump);
        } else {
            properties.remove(SublageProject.PROP_STACK_DUMP);
        }
        if (main != null) {
            properties.setProperty(SublageProject.PROP_MAIN_BINARY,
                    SublageProject.BINARIES_DIR +
                    File.separator +
                    main.replaceAll("\\.source$", ".binary"));
        } else {
            properties.remove(SublageProject.PROP_MAIN_BINARY);
        }
        if (debugOn) {
            properties.setProperty(SublageProject.PROP_DEBUG, "on");
        } else {
            properties.remove(SublageProject.PROP_DEBUG);
        }
        if (debuggerPort != -1) {
            properties.setProperty("debugger", "localhost");
            properties.setProperty("debuggerport", Integer.toString(debuggerPort));
        } else {
            properties.remove("debugger");
        }
    }

    public boolean searchForToolchain() {
        loadPreferences();
        String ext = "";
        if (isWindows()) {
            ext = ".exe";
        }
        Preferences prefs = NbPreferences.forModule(SublageProject.class);

        File sublage;

        if (properties.getProperty(PROP_SUBLAGE_BINARY) != null) {
            sublage = new File(properties.getProperty(PROP_SUBLAGE_BINARY));
            if (!sublage.canExecute()) {
                sublage = null;
                properties.remove(PROP_SUBLAGE_BINARY);
            }
        } else {
            sublage = searchInPath("sublage" + ext, "PATH");
            if ((sublage != null) && sublage.canExecute()) {
                properties.setProperty(PROP_SUBLAGE_BINARY, sublage.getAbsolutePath());
                prefs.put(SublageProject.PROP_SUBLAGE_BINARY,
                        sublage.getAbsolutePath());
            } else {
                sublage = null;
                properties.remove(PROP_SUBLAGE_BINARY);
            }
        }
        File sublagec;

        if (properties.getProperty(PROP_SUBLAGEC_BINARY) != null) {
            sublagec = new File(properties.getProperty(PROP_SUBLAGEC_BINARY));
            if (!sublagec.canExecute()) {
                sublagec = null;
                properties.remove(PROP_SUBLAGEC_BINARY);
            }
        } else {
            sublagec = searchInPath("sublagec" + ext, "PATH");
            if ((sublagec != null) && sublagec.canExecute()) {
                properties.setProperty(PROP_SUBLAGEC_BINARY, sublagec.getAbsolutePath());
                prefs.put(SublageProject.PROP_SUBLAGEC_BINARY,
                        sublagec.getAbsolutePath());
            } else {
                sublagec = null;
                properties.remove(PROP_SUBLAGEC_BINARY);
            }
        }
        File stdlib;

        if (properties.getProperty(PROP_STDLIB_DIRECTORY) != null) {
            stdlib = new File(properties.getProperty(PROP_STDLIB_DIRECTORY)
                    + File.separator + "console.library");
            if (!stdlib.isFile()) {
                properties.remove(PROP_STDLIB_DIRECTORY);
            }
        } else {
            if (isWindows()) {
                stdlib = searchInPath("console.library", "PATH");
            } else {
                stdlib = searchInPath("console.library", "LD_LIBRARY_PATH");
            }
            if ((stdlib == null) && (sublagec != null)) {
                stdlib = searchForStdlib(sublagec, "console.library");
            }
            if ((stdlib == null) && (sublage != null)) {
                stdlib = searchForStdlib(sublage, "console.library");
            }
            if (stdlib != null) {
                properties.setProperty(PROP_STDLIB_DIRECTORY, stdlib.getParent());
                prefs.put(SublageProject.PROP_STDLIB_DIRECTORY,
                        stdlib.getParent());
            } else {
                properties.remove(PROP_STDLIB_DIRECTORY);
            }
        }

        if (!properties.containsKey(SublageProject.PROP_SUBLAGE_BINARY)
                || !properties.containsKey(SublageProject.PROP_SUBLAGEC_BINARY)
                || !properties.containsKey(SublageProject.PROP_STDLIB_DIRECTORY)) {
            JOptionPane.showMessageDialog(null,
                    "Sublage toolchain not found or not properly configured.\nPlease configure it manually in NetBeans preferences.",
                    "Sublage",
                    JOptionPane.ERROR_MESSAGE);
            return false;
        }

        if (command.equals(ActionProvider.COMMAND_RUN)
                || command.equals(ActionProvider.COMMAND_DEBUG)) {
            if (!properties.containsKey(SublageProject.PROP_MAIN_BINARY)) {
                JOptionPane.showMessageDialog(null,
                        "No main program selected.\nPlease configure it manually in project properties.",
                        "Sublage",
                        JOptionPane.ERROR_MESSAGE);
                return false;
            }
        }

        if (command.equals(ActionProvider.COMMAND_RUN)) {
            properties.remove("debugger");
        }

        return true;
    }
}