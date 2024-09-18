package sublage.debugger.annotations;

import org.netbeans.api.debugger.Breakpoint;
import org.openide.text.Annotatable;
import org.openide.util.NbBundle;

public class DisabledBreakpointAnnotation extends SublageBreakpointAnnotation {

    public static final String DISABLED_BREAKPOINT_ANNOTATION_TYPE 
                                    = "DisabledBreakpoint";
    private static final String DISABLED_BREAKPOINT = "Disabled Breakpoint";

    public DisabledBreakpointAnnotation( Annotatable annotatable, Breakpoint breakpoint ) {
        super(annotatable, breakpoint);
    }

    @Override
    public String getAnnotationType() {
        return DISABLED_BREAKPOINT_ANNOTATION_TYPE;
    }

    @Override
    public String getShortDescription()  {
        return DISABLED_BREAKPOINT;
    }

}