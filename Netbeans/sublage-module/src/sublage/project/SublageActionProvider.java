package sublage.project;

import sublage.debugger.SessionManager;
import java.io.IOException;
import org.netbeans.spi.project.ActionProvider;
import org.netbeans.spi.project.ui.support.DefaultProjectOperations;
import org.openide.awt.StatusDisplayer;
import org.openide.util.Lookup;

public class SublageActionProvider implements ActionProvider {

    private final SublageProject project;
    private final String[] supported = new String[]{
        ActionProvider.COMMAND_RENAME,
        ActionProvider.COMMAND_DELETE,
        ActionProvider.COMMAND_COPY,
        ActionProvider.COMMAND_BUILD,
        ActionProvider.COMMAND_CLEAN,
        ActionProvider.COMMAND_RUN,
        ActionProvider.COMMAND_REBUILD,
        ActionProvider.COMMAND_DEBUG,};

    public SublageActionProvider(SublageProject project) {
        this.project = project;
    }

    @Override
    public String[] getSupportedActions() {
        return supported;
    }

    @Override
    public void invokeAction(final String command, final Lookup lookup)
            throws IllegalArgumentException {
        if (command.equals(ActionProvider.COMMAND_DELETE)) {
            DefaultProjectOperations.performDefaultDeleteOperation(project);
            return;
        }
        if (command.equals(ActionProvider.COMMAND_COPY)) {
            DefaultProjectOperations.performDefaultCopyOperation(project);
            return;
        }
        if (command.equals(ActionProvider.COMMAND_RENAME)) {
            DefaultProjectOperations.performDefaultRenameOperation(project, null);
            return;
        }
        if (command.equals(ActionProvider.COMMAND_DEBUG)) {
            try {
                SessionManager.startDebug(project);
            } catch (IOException ex) {
                StatusDisplayer.getDefault().
                        setStatusText("Error starting debugger server on port 9876 : "
                        + ex.getLocalizedMessage());

            }
            return;
        }
        AntHelper helper = new AntHelper(project, command);
        if (!helper.searchForToolchain()) {
            return;
        }
        new Thread(helper).start();
    }

    @Override
    public boolean isActionEnabled(String command, Lookup lookup)
            throws IllegalArgumentException {
        if (command.equals(ActionProvider.COMMAND_DEBUG)) {
            return !SessionManager.haveDebugSession(project);
        }
        return ((command.equals(ActionProvider.COMMAND_DELETE))
                || command.equals(ActionProvider.COMMAND_RENAME)
                || command.equals(ActionProvider.COMMAND_COPY)
                || command.equals(ActionProvider.COMMAND_BUILD)
                || command.equals(ActionProvider.COMMAND_CLEAN)
                || command.equals(ActionProvider.COMMAND_RUN)
                || command.equals(ActionProvider.COMMAND_REBUILD));
    }
}