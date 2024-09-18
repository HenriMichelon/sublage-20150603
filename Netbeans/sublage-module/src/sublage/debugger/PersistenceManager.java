package sublage.debugger;

import sublage.debugger.objects.SublageBreakpoint;
import java.beans.PropertyChangeEvent;
import java.util.ArrayList;
import java.util.List;

import org.netbeans.api.debugger.Breakpoint;
import org.netbeans.api.debugger.DebuggerManager;
import org.netbeans.api.debugger.DebuggerManagerAdapter;
import org.netbeans.api.debugger.Properties;

public class PersistenceManager extends DebuggerManagerAdapter {

    private static final String DEBUGGER = "debugger";
    private static final String SUBLAGE_DEBUGGER_BRAKPOINTS = "SUBLAGE-DBGP";

    @Override
    public Breakpoint[] initBreakpoints() {
        Properties p = Properties.getDefault().getProperties(DEBUGGER).
                getProperties(DebuggerManager.PROP_BREAKPOINTS);
        Breakpoint[] breakpoints = (Breakpoint[]) p.getArray(
                SUBLAGE_DEBUGGER_BRAKPOINTS, new Breakpoint[0]);
        List<Breakpoint> validBreakpoints = new ArrayList<Breakpoint>();
        for (Breakpoint breakpoint : breakpoints) {
            if (breakpoint != null) {
                breakpoint.addPropertyChangeListener(this);
                validBreakpoints.add(breakpoint);
            }
        }
        return validBreakpoints.toArray(new Breakpoint[validBreakpoints.size()]);
    }

    @Override
    public String[] getProperties() {
        return new String[]{
            DebuggerManager.PROP_BREAKPOINTS_INIT,
            DebuggerManager.PROP_BREAKPOINTS,};
    }

    @Override
    public void breakpointAdded(Breakpoint breakpoint) {
        Properties properties = Properties.getDefault().getProperties(DEBUGGER).
                getProperties(DebuggerManager.PROP_BREAKPOINTS);
        properties.setArray(SUBLAGE_DEBUGGER_BRAKPOINTS, getBreakpoints());
        breakpoint.addPropertyChangeListener(this);
    }

    @Override
    public void breakpointRemoved(Breakpoint breakpoint) {
        Properties properties = Properties.getDefault().getProperties(DEBUGGER).
                getProperties(DebuggerManager.PROP_BREAKPOINTS);
        properties.setArray(SUBLAGE_DEBUGGER_BRAKPOINTS, getBreakpoints());
        breakpoint.removePropertyChangeListener(this);
    }

    @Override
    public void propertyChange(PropertyChangeEvent evt) {
        /*
         * Breakpoint could be disabled/enabled.
         * This notification are got in the case changing this property.
         */
        if (evt.getSource() instanceof Breakpoint) {
            Properties.getDefault().getProperties(DEBUGGER).
                    getProperties(DebuggerManager.PROP_BREAKPOINTS).setArray(
                    SUBLAGE_DEBUGGER_BRAKPOINTS, getBreakpoints());
        }
    }

    private Breakpoint[] getBreakpoints() {
        Breakpoint[] bpoints = DebuggerManager.getDebuggerManager().getBreakpoints();
        List<Breakpoint> result = new ArrayList<Breakpoint>();
        for (Breakpoint breakpoint : bpoints) {
            if (breakpoint instanceof SublageBreakpoint) {
                result.add(breakpoint);
            }
        }
        return result.toArray(new Breakpoint[result.size()]);
    }
}
