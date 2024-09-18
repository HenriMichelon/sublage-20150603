package sublage.debugger.views;

import sublage.debugger.objects.SublageBreakpoint;
import org.netbeans.api.debugger.Breakpoint.VALIDITY;
import org.netbeans.spi.viewmodel.NodeModel;
import org.netbeans.spi.viewmodel.UnknownTypeException;
import org.openide.filesystems.FileObject;

public class BreakpointModel extends ViewModel
        implements NodeModel {

    public static final String LINE_BREAKPOINT =
            "org/netbeans/modules/debugger/resources/breakpointsView/Breakpoint";                   // NOI18N
    public static final String CURRENT_LINE_BREAKPOINT =
            "org/netbeans/modules/debugger/resources/breakpointsView/BreakpointHit";                // NOI18N
    public static final String DISABLED_LINE_BREAKPOINT =
            "org/netbeans/modules/debugger/resources/breakpointsView/DisabledBreakpoint";           // NOI18N
    public static final String DISABLED_CURRENT_LINE_BREAKPOINT =
            "org/netbeans/modules/debugger/resources/breakpointsView/DisabledBreakpointHit";        // NOI18N
    public static final String BROKEN_LINE_BREAKPOINT =
            "org/netbeans/modules/debugger/resources/breakpointsView/Breakpoint_broken";// NOI18N

    @Override
    public String getDisplayName(Object node) throws UnknownTypeException {
        if (node instanceof SublageBreakpoint) {
            SublageBreakpoint breakpoint = (SublageBreakpoint) node;
            FileObject fileObject = breakpoint.getLine().getLookup().
                    lookup(FileObject.class);
            return fileObject.getNameExt() + ":"
                    + (breakpoint.getLine().getLineNumber() + 1);
        }
        throw new UnknownTypeException(node);
    }

    @Override
    public String getIconBase(Object node) throws UnknownTypeException {
        if (node instanceof SublageBreakpoint) {
            SublageBreakpoint breakpoint = (SublageBreakpoint) node;
            if (!breakpoint.isEnabled()) {
                return DISABLED_LINE_BREAKPOINT;
            } else {
                VALIDITY validity = breakpoint.getValidity();
                if (validity.equals(VALIDITY.VALID) || validity.equals(VALIDITY.UNKNOWN)) {
                    return LINE_BREAKPOINT;
                } else {
                    return BROKEN_LINE_BREAKPOINT;
                }
            }
        }
        throw new UnknownTypeException(node);
    }

    @Override
    public String getShortDescription(Object node) throws UnknownTypeException {
        if (node instanceof SublageBreakpoint) {
            return ((SublageBreakpoint) node).getLine().getDisplayName();
        }
        throw new UnknownTypeException(node);
    }

}
