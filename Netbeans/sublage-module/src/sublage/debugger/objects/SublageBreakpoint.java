package sublage.debugger.objects;

import org.netbeans.api.debugger.Breakpoint;
import org.netbeans.api.debugger.DebuggerManager;
import org.openide.filesystems.FileChangeAdapter;
import org.openide.filesystems.FileChangeListener;
import org.openide.filesystems.FileEvent;
import org.openide.filesystems.FileObject;
import org.openide.loaders.DataObject;
import org.openide.text.Line;
import org.openide.util.WeakListeners;

public class SublageBreakpoint extends Breakpoint {

    private class FileRemoveListener extends FileChangeAdapter {

        @Override
        public void fileDeleted(FileEvent arg0) {
            DebuggerManager.getDebuggerManager().removeBreakpoint(
                    SublageBreakpoint.this);
        }
    }
    private Line line;
    private FileRemoveListener removeListener;
    private FileChangeListener changeListener;
    private String fileName;
    private String id;
    private boolean enabled;

    public SublageBreakpoint(Line line) {
        this.line = line;
        enabled = true;
        removeListener = new FileRemoveListener();
        FileObject fileObject = line.getLookup().lookup(FileObject.class);
        //DataObject dataObjectOrNull = (DataObject)line.getLookup ().lookup (DataObject.class);
        if (fileObject != null) {
            changeListener = WeakListeners.create(
                    FileChangeListener.class, removeListener, fileObject);
            fileObject.addFileChangeListener(changeListener);
            fileName = fileObject.getPath();
        } else {
            fileName = "";
        }
    }

    public final void setValid(String message) {
        setValidity(VALIDITY.VALID, message);
    }

    public final void setInvalid(String message) {
        setValidity(VALIDITY.INVALID, message);
    }

    public Line getLine() {
        return line;
    }

    public String getFilename() {
        return fileName;
    }

    public int isTemp() {
        return 0;
    }

    public void removed() {
        FileObject fileObject = getLine().getLookup().lookup(FileObject.class);
        if (fileObject != null) {
            fileObject.removeFileChangeListener(changeListener);
        }
    }

    @Override
    public void disable() {
        if (!enabled) {
            return;
        }
        enabled = false;
        firePropertyChange(PROP_ENABLED, Boolean.TRUE, Boolean.FALSE);
    }

    @Override
    public void enable() {
        if (enabled) {
            return;
        }
        enabled = true;
        firePropertyChange(PROP_ENABLED, Boolean.FALSE, Boolean.TRUE);
    }

    @Override
    public boolean isEnabled() {
        return enabled;
    }

    public void setBreakpointId(String id) {
        this.id = id;
    }

    public String getBreakpointId() {
        return id;
    }
}
