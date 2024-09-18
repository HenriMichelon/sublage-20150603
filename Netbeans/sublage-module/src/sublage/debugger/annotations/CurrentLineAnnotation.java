package sublage.debugger.annotations;

import org.openide.text.Annotatable;
import org.openide.util.NbBundle;

public class CurrentLineAnnotation extends DebuggerAnnotation {

    private static final String CURRENT_LINE = "Current Line";
    public static final String CURRENT_LINE_ANNOTATION_TYPE = "CurrentPC";

    public CurrentLineAnnotation(Annotatable annotatable) {
        super(annotatable);
    }

    @Override
    public String getAnnotationType() {
        return CURRENT_LINE_ANNOTATION_TYPE;
    }

    @Override
    public String getShortDescription() {
        return CURRENT_LINE;
    }
}