package sublage.project.ui;

import sublage.project.SublageProject;
import java.util.ArrayList;
import java.util.List;
import javax.swing.event.ChangeListener;
import org.netbeans.api.project.Project;
import org.netbeans.spi.project.ui.support.NodeFactory;
import org.netbeans.spi.project.ui.support.NodeList;
import org.openide.filesystems.FileObject;
import org.openide.loaders.DataObject;
import org.openide.loaders.DataObjectNotFoundException;
import org.openide.nodes.FilterNode;
import org.openide.nodes.Node;
import org.openide.util.Exceptions;

@NodeFactory.Registration(projectType = "sublage-project", position = 10)
public class SublageNodeFactory implements NodeFactory {

    @Override
    public NodeList<?> createNodes(Project project) {
        SublageProject p = project.getLookup().lookup(SublageProject.class);
        assert p != null;
        return new SublageNodeList(p);
    }

    private class SublageNodeList implements NodeList<Node> {

        SublageProject project;

        public SublageNodeList(SublageProject project) {
            this.project = project;
        }

        @Override
        public List<Node> keys() {
            FileObject mainFolder = project.getProjectDirectory();
            List<Node> result = new ArrayList<Node>();
            if (mainFolder != null) {
                // TODO : subdirs
                for (FileObject file : mainFolder.getChildren()) {
                    if ((file.isFolder() && file.getName().equals(SublageProject.SOURCES_DIR))
                            || file.getMIMEType().equals("text/x-sublagesource")) {
                        try {
                            result.add(DataObject.find(file).getNodeDelegate());
                        } catch (DataObjectNotFoundException ex) {
                            Exceptions.printStackTrace(ex);
                        }
                    }
                }
            }
            return result;
        }

        @Override
        public Node node(Node node) {
            return new FilterNode(node);
        }

        @Override
        public void addNotify() {
        }

        @Override
        public void removeNotify() {
        }

        @Override
        public void addChangeListener(ChangeListener cl) {
        }

        @Override
        public void removeChangeListener(ChangeListener cl) {
        }
    }
}
