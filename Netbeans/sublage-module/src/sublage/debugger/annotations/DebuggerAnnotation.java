package sublage.debugger.annotations;

import org.openide.text.Annotatable;
import org.openide.text.Annotation;

public abstract class DebuggerAnnotation extends Annotation {

    private String message;

    public DebuggerAnnotation(Annotatable annotatable) {
        attach(annotatable);
    }

    public DebuggerAnnotation(Annotatable annotatable, String message) {
        this(annotatable);
        this.message = message;
    }

    @Override
    public abstract String getAnnotationType();

    @Override
    public String getShortDescription() {
        return message;
    }
}