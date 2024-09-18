package sublage.debugger;

import sublage.debugger.objects.SublageBreakpoint;
import sublage.debugger.annotations.DisabledBreakpointAnnotation;
import sublage.debugger.annotations.SublageBreakpointAnnotation;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Set;

import javax.swing.SwingUtilities;

import org.netbeans.api.debugger.ActionsManager;
import org.netbeans.api.debugger.Breakpoint;
import org.netbeans.api.debugger.DebuggerManager;
import org.netbeans.api.debugger.Session;
import org.netbeans.spi.debugger.ActionsProvider;
import org.netbeans.spi.debugger.ActionsProviderSupport;
import org.netbeans.spi.debugger.ui.EditorContextDispatcher;
import org.openide.text.Annotation;
import org.openide.text.Line;
import org.openide.util.WeakListeners;

@ActionsProvider.Registration(
        actions = {"toggleBreakpoint"},
        activateForMIMETypes = {BreakpointActionProvider.MIME_TYPE})
public class BreakpointActionProvider extends ActionsProviderSupport
        implements PropertyChangeListener {

    public static final String MIME_TYPE = "text/x-sublagesource";
    private Map<Breakpoint, Annotation> annotations = new HashMap<Breakpoint, Annotation>();

    public BreakpointActionProvider() {
        setEnabled(ActionsManager.ACTION_TOGGLE_BREAKPOINT, false);
        EditorContextDispatcher.getDefault().addPropertyChangeListener(
                MIME_TYPE,
                WeakListeners.propertyChange(this, EditorContextDispatcher.getDefault()));
    }

    @Override
    public void doAction(Object action) {
        if (SwingUtilities.isEventDispatchThread()) {
            addBreakpoints();
        } else {
            SwingUtilities.invokeLater(new Runnable() {
                @Override
                public void run() {
                    addBreakpoints();
                }
            });
        }
    }

    @Override
    public Set getActions() {
        return Collections.singleton(ActionsManager.ACTION_TOGGLE_BREAKPOINT);
    }

    private void addBreakpoints() {
        Line line = EditorContextDispatcher.getDefault().getCurrentLine();
        if (line == null) {
            return;
        }
        DebuggerManager manager = DebuggerManager.getDebuggerManager();

        Breakpoint[] breakpoints = manager.getBreakpoints();
        boolean add = true;
        for (Breakpoint breakpoint : breakpoints) {
            if (breakpoint instanceof SublageBreakpoint
                    && ((SublageBreakpoint) breakpoint).getLine().equals(line)) {
                manager.removeBreakpoint(breakpoint);
                removeAnnotation(breakpoint);
                Session session = manager.getCurrentSession();
                if (session != null) {
                    Debugger dbg = session.lookupFirst(null, Debugger.class);
                    dbg.commandBreakpointUnset((SublageBreakpoint) breakpoint);
                }
                add = false;
                break;
            }
        }

        if (add) {
            Breakpoint breakpoint = new SublageBreakpoint(line);
            manager.addBreakpoint(breakpoint);
            addAnnotation(breakpoint);
            Session session = manager.getCurrentSession();
            if (session != null) {
                Debugger dbg = session.lookupFirst(null, Debugger.class);
                dbg.commandBreakpointSet((SublageBreakpoint) breakpoint);
            }
        }
    }

    @Override
    public void propertyChange(PropertyChangeEvent evt) {
        boolean enabled = EditorContextDispatcher.getDefault().getCurrentLine() != null;
        setEnabled(ActionsManager.ACTION_TOGGLE_BREAKPOINT, enabled);
        if (evt.getPropertyName() != Breakpoint.PROP_ENABLED) {
            return;
        }
        removeAnnotation((Breakpoint) evt.getSource());
        addAnnotation((Breakpoint) evt.getSource());
    }

    private void addAnnotation(Breakpoint breakpoint) {
        Line line = ((SublageBreakpoint) breakpoint).getLine();
        Annotation annotation = breakpoint.isEnabled()
                ? new SublageBreakpointAnnotation(line, breakpoint)
                : new DisabledBreakpointAnnotation(line, breakpoint);
        annotations.put(breakpoint, annotation);
        breakpoint.addPropertyChangeListener(Breakpoint.PROP_ENABLED, this);
    }

    private void removeAnnotation(Breakpoint breakpoint) {
        Annotation annotation = annotations.remove(breakpoint);
        if (annotation == null) {
            return;
        }
        annotation.detach();
        breakpoint.removePropertyChangeListener(Breakpoint.PROP_ENABLED, this);
    }
}
