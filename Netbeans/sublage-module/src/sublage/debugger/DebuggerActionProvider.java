package sublage.debugger;

import sublage.project.AntHelper;
import sublage.project.SublageProject;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Future;
import org.netbeans.api.debugger.ActionsManager;
import org.netbeans.api.debugger.DebuggerManager;
import org.netbeans.api.debugger.Session;
import org.netbeans.spi.debugger.ActionsProviderSupport;
import org.netbeans.spi.debugger.ContextProvider;
import org.netbeans.spi.debugger.DebuggerEngineProvider;
import org.netbeans.spi.project.ActionProvider;
import org.openide.awt.StatusDisplayer;

public class DebuggerActionProvider extends ActionsProviderSupport {

    private static final Set actions = new HashSet();
    private static final StatusDisplayer statusBar = StatusDisplayer.getDefault();
    private final EngineProvider engineProvider;
    private AntHelper antHelper;
    private Future<Integer> exitCode;

    static {
        actions.add(ActionsManager.ACTION_START);
        actions.add(ActionsManager.ACTION_KILL);
        actions.add(ActionsManager.ACTION_PAUSE);
        actions.add(ActionsManager.ACTION_CONTINUE);
        actions.add(ActionsManager.ACTION_STEP_INTO);
        actions.add(ActionsManager.ACTION_STEP_OVER);
        actions.add(ActionsManager.ACTION_STEP_OUT);
    }

    public DebuggerActionProvider(ContextProvider contextProvider) {
        engineProvider = (EngineProvider) contextProvider.
                lookupFirst(null, DebuggerEngineProvider.class);
        setEnabled(ActionsManager.ACTION_START, true);
        setEnabled(ActionsManager.ACTION_KILL, true);
    }

    public void setActionsInPause(boolean inpause) {
        for (Object obj : actions) {
            if (obj == ActionsManager.ACTION_CONTINUE
                    || obj == ActionsManager.ACTION_STEP_INTO
                    || obj == ActionsManager.ACTION_STEP_OVER
                    || obj == ActionsManager.ACTION_STEP_OUT
                    || obj == ActionsManager.ACTION_RUN_TO_CURSOR) {
                setEnabled(obj, inpause);
            } else if (obj == ActionsManager.ACTION_PAUSE) {
                setEnabled(obj, !inpause);
            }
        }
        setEnabled(ActionsManager.ACTION_KILL, true);
    }

    @Override
    public void doAction(Object action) {
        Session session = DebuggerManager.getDebuggerManager().getCurrentSession();
        if (session == null) {
            return;
        }
        final SublageProject project = session.lookupFirst(null, SublageProject.class);
        final Debugger dbg = session.lookupFirst(null, Debugger.class);
        if ((dbg == null) || (project == null)) {
            return;
        }
        if ((action.equals(ActionsManager.ACTION_START) && (!dbg.isDebugging()))) {
            statusBar.setStatusText("debugger starting, waiting on TCP port "
                    + dbg.getPort() + " for sub process to attach.");
            new Thread(dbg).start();
            new Thread(new Runnable() {
                @Override
                public void run() {
                    antHelper = new AntHelper(project,
                            ActionProvider.COMMAND_DEBUG,
                            dbg.getPort());
                    if (!antHelper.searchForToolchain()) {
                        statusBar.setStatusText("Sublage toolchain not found");
                        stopDebug(dbg);
                        return;
                    }
                    try {
                        if (antHelper.runTask()) {
                            if (antHelper.result() != 0) {
                                statusBar.setStatusText(
                                        "Subprocess exited with exit code value "
                                        + antHelper.result());
                            }
                        } else {
                            statusBar.setStatusText("Error executing Ant debug task");
                        }
                    } catch (IOException ex) {
                        statusBar.setStatusText("Error while executing Ant debug task");
                    }
                    stopDebug(dbg);
                }
            }).start();
        } else if (action.equals(ActionsManager.ACTION_KILL)) {
            dbg.commandStop();
            stopDebug(dbg);
            if (exitCode != null) {
                exitCode.cancel(true);
            }
        } else if (action.equals(ActionsManager.ACTION_STEP_OVER)) {
            dbg.commandStepOver();
        } else if (action.equals(ActionsManager.ACTION_STEP_INTO)) {
            dbg.commandStepInto();
        } else if (action.equals(ActionsManager.ACTION_STEP_OUT)) {
            dbg.commandStepOut();
        } else if (action.equals(ActionsManager.ACTION_PAUSE)) {
            dbg.commandPause();
        } else if (action.equals(ActionsManager.ACTION_CONTINUE)) {
            dbg.commandRun();
        }
    }

    private void stopDebug(Debugger dbg) {
        if (antHelper != null) {
            antHelper.stop();
        }
        dbg.stop();
        engineProvider.getDestructor().killEngine();
        setEnabled(ActionsManager.ACTION_KILL, false);
        setEnabled(ActionsManager.ACTION_START, true);
    }

    @Override
    public Set getActions() {
        return actions;
    }
}