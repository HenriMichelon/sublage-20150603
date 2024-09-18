package sublage.debugger.views;

import sublage.debugger.objects.SublageVariable;
import java.util.ArrayList;
import java.util.List;
import org.netbeans.spi.debugger.ui.Constants;
import org.netbeans.spi.viewmodel.NodeActionsProvider;
import org.netbeans.spi.viewmodel.NodeModel;
import org.netbeans.spi.viewmodel.TableModel;
import org.netbeans.spi.viewmodel.TreeModel;
import static org.netbeans.spi.viewmodel.TreeModel.ROOT;
import org.netbeans.spi.viewmodel.UnknownTypeException;

public class VariablesModel extends ViewModel
        implements TreeModel, TableModel, NodeModel, NodeActionsProvider {

    public static final String LOCAL_VARIABLE_ICON =
            "org/netbeans/modules/debugger/resources/localsView/LocalVariable";
    private List<SublageVariable> vars = new ArrayList<SublageVariable>();

    public void setVariabless(List<SublageVariable> vars) {
        this.vars = vars;
        refresh();
    }

    @Override
    public Object[] getChildren(Object parent, int from, int to) throws UnknownTypeException {
        if (parent == ROOT) {
            return vars.subList(from, to).toArray();
        }
        throw new UnknownTypeException(parent);
    }

    @Override
    public int getChildrenCount(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return vars.size();
        }
        throw new UnknownTypeException(node);
    }

    @Override
    public String getDisplayName(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return "Variables";
        }
        return ((SublageVariable) node).getName();
    }

    @Override
    public String getIconBase(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return null;
        }
        return LOCAL_VARIABLE_ICON;
    }

    @Override
    public String getShortDescription(Object node) throws UnknownTypeException {
        if (node == ROOT) {
            return null;
        }
        if (node instanceof SublageVariable) {
            return ((SublageVariable) node).getName();
        }
        throw new UnknownTypeException(node);
    }

    @Override
    public Object getValueAt(Object node, String columnID) throws UnknownTypeException {
        if (node == ROOT) {
            return null;
        }
        if (!(node instanceof SublageVariable)) {
            throw new UnknownTypeException(node);
        }
        SublageVariable var = (SublageVariable) node;
        if (Constants.LOCALS_TYPE_COLUMN_ID.equals(columnID)) {
            return var.getOpcode().toString();
        } else if (Constants.LOCALS_VALUE_COLUMN_ID.equals(columnID)) {
            return var.getValue();
        }
        return "";
    }
}
