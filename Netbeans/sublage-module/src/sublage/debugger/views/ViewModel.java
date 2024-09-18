package sublage.debugger.views;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;
import javax.swing.Action;
import org.netbeans.spi.viewmodel.ModelEvent;
import org.netbeans.spi.viewmodel.ModelListener;
import static org.netbeans.spi.viewmodel.TreeModel.ROOT;
import org.netbeans.spi.viewmodel.UnknownTypeException;

public class ViewModel {

    private List<ModelListener> listeners = new CopyOnWriteArrayList<ModelListener>();

    public void refresh() {
        ModelEvent evt = new ModelEvent.TreeChanged(this);
        for (ModelListener l : listeners) {
            l.modelChanged(evt);
        }
    }

    public void addModelListener(ModelListener l) {
        listeners.add(l);
    }

    public void removeModelListener(ModelListener l) {
        listeners.remove(l);
    }
    
    public Object getRoot() {
        return ROOT;
    }

    public boolean isLeaf(Object node) {
        return node != ROOT;
    }

    public boolean isReadOnly(Object node, String columnID) throws UnknownTypeException {
        return false;
    }

    public void setValueAt(Object node, String columnID, Object value) throws UnknownTypeException {
    }

    public void performDefaultAction(Object node) throws UnknownTypeException {
    }

    public Action[] getActions(Object node) throws UnknownTypeException {
        return new Action[]{};
    }


}
