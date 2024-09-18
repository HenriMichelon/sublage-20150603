package sublage.project;

import sublage.project.properties.SublageCustomizerProvider;
import sublage.project.ui.SublageProjectLogicalView;
import java.util.prefs.Preferences;
import org.netbeans.api.project.Project;
import org.netbeans.spi.project.ProjectState;
import org.openide.filesystems.FileObject;
import org.openide.util.Lookup;
import org.openide.util.NbPreferences;
import org.openide.util.lookup.Lookups;

public class SublageProject implements Project {

    public static final String PROP_SUBLAGE_BINARY = "sublagebin";
    public static final String PROP_SUBLAGEC_BINARY = "sublagecbin";
    public static final String PROP_STDLIB_DIRECTORY = "stdlib";
    public static final String PROP_MAIN_BINARY = "main";
    public static final String PROP_STACK_DUMP = "stackdump";
    public static final String PROP_DEBUG = "debug";
    public static final String SOURCES_DIR = "sources";
    public static final String BINARIES_DIR = "binaries";
    private final FileObject projectDir;
    private Lookup lkp;
    private String mainbinary;
    private String stackdump;
    private boolean debugOn = true;
    private boolean cleanNeeded;
    
    public boolean isCleanNeeded() {
        return cleanNeeded;
    }

    public void setCleanNeeded(boolean cleanNeeded) {
        this.cleanNeeded = cleanNeeded;
    }

    public void setDebugOn(boolean debugOn) {
        cleanNeeded = debugOn != this.debugOn;
        this.debugOn = debugOn;
        Preferences prefs = NbPreferences.forModule(SublageProject.class);
        prefs.putBoolean(PROP_DEBUG, debugOn);
    }

    public boolean isDebugOn() {
        return debugOn;
    }

    public void setStackdump(String stackdump) {
        this.stackdump = stackdump;
        Preferences prefs = NbPreferences.forModule(SublageProject.class);
        if (stackdump != null) {
            prefs.put(PROP_STACK_DUMP, stackdump);
        } else {
            prefs.remove(PROP_STACK_DUMP);
        }
    }

    public String getStackdump() {
        return stackdump;
    }

    public String getMainbinary() {
        return mainbinary;
    }

    public void setMainbinary(String mainbinary) {
        this.mainbinary = mainbinary;
        Preferences prefs = NbPreferences.forModule(SublageProject.class);
        prefs.put(PROP_MAIN_BINARY, mainbinary);
    }

    SublageProject(FileObject dir, ProjectState state) {
        this.projectDir = dir;
        Preferences prefs = NbPreferences.forModule(SublageProject.class);
        mainbinary = prefs.get(PROP_MAIN_BINARY, "main.source");
        debugOn = prefs.getBoolean(PROP_DEBUG, true);
        stackdump = prefs.get(PROP_STACK_DUMP, null);
        FileObject sources = dir.getFileObject(SOURCES_DIR);
        if ((sources != null)
                && (sources.getFileObject(mainbinary) == null)) {
            mainbinary = null;
        } else {
            prefs.put(PROP_MAIN_BINARY, mainbinary);
        }
    }

    @Override
    public FileObject getProjectDirectory() {
        return projectDir;
    }

    @Override
    public Lookup getLookup() {
        if (lkp == null) {
            lkp = Lookups.fixed(new Object[]{
                this,
                new SublageProjectInformation(this),
                new SublageActionProvider(this),
                new SublagePrivilegiedTemplates(),
                new SublageProjectOperations(this),
                new SublageProjectLogicalView(this),
                new SublageCustomizerProvider(this),});
        }
        return lkp;
    }
}
