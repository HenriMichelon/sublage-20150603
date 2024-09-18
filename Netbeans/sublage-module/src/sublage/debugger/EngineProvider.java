package sublage.debugger;

import org.netbeans.api.debugger.DebuggerEngine;
import org.netbeans.spi.debugger.DebuggerEngineProvider;

public class EngineProvider extends DebuggerEngineProvider {
    
    private DebuggerEngine.Destructor destructor;

    @Override
    public String[] getLanguages() {
        return new String[] { "Sublage" };
    }

    @Override
    public String getEngineTypeID() {
        return "SublageDebuggerEngine";
    }

    @Override
    public Object[] getServices() {
        return new Object[]{};
    }

    public DebuggerEngine.Destructor getDestructor() {
        return destructor;
    }

    @Override
    public void setDestructor(DebuggerEngine.Destructor destructor) {
        this.destructor = destructor;
    }

}