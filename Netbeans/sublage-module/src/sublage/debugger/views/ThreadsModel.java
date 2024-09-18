package sublage.debugger.views;

import sublage.debugger.objects.SublageThread;
import java.io.File;
import java.util.ArrayList;
import java.util.Collection;
import org.netbeans.spi.viewmodel.NodeActionsProvider;
import org.netbeans.spi.viewmodel.NodeModel;
import org.netbeans.spi.viewmodel.TableModel;
import org.netbeans.spi.viewmodel.TreeModel;
import static org.netbeans.spi.viewmodel.TreeModel.ROOT;
import org.netbeans.spi.viewmodel.UnknownTypeException;

public class ThreadsModel extends ViewModel
        implements TreeModel, NodeModel, NodeActionsProvider, TableModel {

    private static final String RUNNING_STATE = "Running (%s line %d : %s)";
    private static final String PAUSED_STATE = "Paused (%s line %d : %s)";
    public static final String CURRENT_ICON =
            "org/netbeans/modules/debugger/resources/threadsView/CurrentThread";
    public static final String RUNNING_ICON =
            "org/netbeans/modules/debugger/resources/threadsView/RunningThread";
    public static final String PAUSED_ICON =
            "org/netbeans/modules/debugger/resources/threadsView/SuspendedThread";
    private Collection<SublageThread> threads = new ArrayList<SublageThread>();
    private int currentThread;

    public void setCurrentThread(int currentThread) {
        this.currentThread = currentThread;
    }

    public void setThreads(Collection<SublageThread> threads) {
        this.threads = threads;
        refresh();
    }

    @Override
    public Object[] getChildren(Object parent, int from, int to) throws UnknownTypeException {
        if (parent == ROOT) {
            return threads.toArray();//.subList(from, to).toArray();
        }
        throw new UnknownTypeException(parent);
    }

    @Override
    public int getChildrenCount(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return threads.size();
        }
        throw new UnknownTypeException(node);
    }

    @Override
    public String getDisplayName(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return "Threads";
        }
        return "Thread #" + ((SublageThread) node).getId();
    }

    @Override
    public String getIconBase(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return null;
        }
        SublageThread thread = (SublageThread)node;
        if (thread.getId() == currentThread) {
            return CURRENT_ICON;
        }
        return (thread.isRunning() ? RUNNING_ICON : PAUSED_ICON);
    }

    @Override
    public String getShortDescription(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return null;
        }
        return "#" + ((SublageThread) node).getId();
    }

    @Override
    public Object getValueAt(Object node, String columnID) throws UnknownTypeException {
        if (node == ROOT) {
            return null;
        }
        
        if (node instanceof SublageThread) {
            SublageThread thread = (SublageThread)node;
            File f = new File(thread.getSource());
            return (thread.isRunning() ? 
                    String.format(RUNNING_STATE, f.getName(), thread.getLine(), thread.getText()) : 
                    String.format(PAUSED_STATE, f.getName(), thread.getLine(), thread.getText()));
        }
        throw new UnknownTypeException(node);
    }

}