package sublage.debugger;

import sublage.project.SublageProject;
import java.io.IOException;
import org.netbeans.api.debugger.DebuggerInfo;
import org.netbeans.api.debugger.DebuggerManager;
import org.netbeans.api.debugger.Session;
import org.netbeans.spi.debugger.SessionProvider;

public class SessionManager {

    private static final String DEBUGGER_INFO = "SublageDebuggerInfo";
    private static final String SESSION = "SublageSession";

    public static void startDebug(SublageProject project) throws IOException {
        if (haveDebugSession(project)) {
            return;
        }
        IDEBridge ide = new IDEBridge(project);
        Debugger dbg = new Debugger(project, ide);
        
        DebuggerManager manager = DebuggerManager.getDebuggerManager();
        DebuggerInfo info = DebuggerInfo.create(DEBUGGER_INFO,
                new Object[]{
            new SessionProvider() {
                @Override
                public String getSessionName() {
                    return "Sublage Program";
                }

                @Override
                public String getLocationName() {
                    return "localhost";
                }

                @Override
                public String getTypeID() {
                    return SESSION;
                }

                @Override
                public Object[] getServices() {
                    return new Object[]{};
                }
            },
            project, dbg, ide
        });
        manager.startDebugging(info);
    }
    
    public static boolean haveDebugSession(SublageProject project) {
        for (Session session : DebuggerManager.getDebuggerManager().getSessions()) {
            if (session.lookupFirst(null, SublageProject.class) == project) {
                return true;
            }
        }
        return false;
    }
}