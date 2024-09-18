package sublage.debugger;

import java.io.File;
import sublage.debugger.annotations.CurrentLineAnnotation;
import sublage.debugger.annotations.DebuggerAnnotation;
import sublage.project.SublageProject;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import javax.swing.SwingUtilities;
import org.netbeans.api.debugger.DebuggerEngine;
import org.netbeans.api.debugger.DebuggerManager;
import org.netbeans.spi.debugger.ActionsProvider;
import org.openide.awt.StatusDisplayer;
import org.openide.cookies.LineCookie;
import org.openide.filesystems.FileObject;
import org.openide.loaders.DataObject;
import org.openide.loaders.DataObjectNotFoundException;
import org.openide.text.Line;

public class IDEBridge {

    private final SublageProject project;
    private static final StatusDisplayer statusBar = StatusDisplayer.getDefault();
    private final Map<String, List<DebuggerAnnotation>> annotations =
            new HashMap<String, List<DebuggerAnnotation>>();

    public IDEBridge(SublageProject project) {
        this.project = project;
    }

    public void annotate(DebuggerAnnotation annotation) {
        String type = annotation.getAnnotationType();
        synchronized (annotations) {
            List<DebuggerAnnotation> list = annotations.get(type);
            if (list == null) {
                list = new LinkedList<DebuggerAnnotation>();
                annotations.put(type, list);
            }
            list.add(annotation);
        }
    }

    public void setActionsInPause(boolean inpause) {
        DebuggerEngine engine = DebuggerManager.getDebuggerManager().getCurrentEngine();
        if (engine == null) {
            return;
        }
        List providers = engine.lookup(null, ActionsProvider.class);
        if ((providers == null) || (providers.isEmpty())) {
            return;
        }
        for (Object object : providers) {
            if (object instanceof DebuggerActionProvider) {
                DebuggerActionProvider provider = (DebuggerActionProvider) object;
                provider.setActionsInPause(inpause);
            }
        }
    }

    private Line getLine(String source, int line) {
        if (line <= 0) {
            statusBar.setStatusText("debug: invalid line number " + line);
            return null;            
        }
        if (!source.contains(project.getProjectDirectory().getPath())) {
            /*statusBar.setStatusText("Not in project path : `" + source + "`");
            return null;*/
            source = project.getProjectDirectory().getPath() + File.separator +
                    source;
        }
        source = source.substring(project.getProjectDirectory().getPath().length());
        FileObject sourcefile = project.getProjectDirectory().getFileObject(source);
        if (sourcefile == null) {
            statusBar.setStatusText("Cannot find source file `" + source + "`");
            return null;
        }
        try {
            LineCookie lc = DataObject.find(sourcefile).getLookup().lookup(LineCookie.class);
            return lc.getLineSet().getCurrent(line - 1);
        } catch (DataObjectNotFoundException ex) {
            return null;
        }
    }

    private void removeAnnotations(List<DebuggerAnnotation> list) {
        if (list != null) {
            for (DebuggerAnnotation annotation : list) {
                annotation.detach();
            }
            list.clear();
        }
    }

    public void removeAnnotations(String type) {
        synchronized (annotations) {
            removeAnnotations(annotations.get(type));
        }
    }

    public void removeAnnotations() {
        synchronized (annotations) {
            for (List<DebuggerAnnotation> list : annotations.values()) {
                removeAnnotations(list);
            }
        }
    }

    public void showAndAnnotateCurrentLine(String source, int line) {
        Line l = getLine(source, line);
        if (l != null) {
            removeAnnotations(CurrentLineAnnotation.CURRENT_LINE_ANNOTATION_TYPE);
            annotate(new CurrentLineAnnotation(l));
            show(l);
        }
    }

    public void show(final Line line) {
        if (line != null) {
            SwingUtilities.invokeLater(new Runnable() {
                @Override
                public void run() {
                    line.show(Line.ShowOpenType.REUSE, Line.ShowVisibilityType.FOCUS);
                }
            });
        }
    }

    public void show(String source, int line) {
        show(getLine(source, line));
    }
}