package sublage.debugger;

import org.netbeans.api.debugger.DebuggerManagerAdapter;
import org.netbeans.api.debugger.Session;

public class SessionListener  extends DebuggerManagerAdapter {
   
    @Override
    public void sessionAdded(Session session) {
        super.sessionAdded(session);
    }

    @Override
    public void sessionRemoved(Session session) {
        super.sessionRemoved(session);
    }

}