package sublage.project.ui;

import sublage.project.SublageActionProvider;
import sublage.project.SublageProject;
import static sublage.project.ui.SublageProjectLogicalView.CUSTOMER_ICON;
import java.awt.Image;
import javax.swing.Action;
import org.netbeans.spi.project.ui.support.CommonProjectActions;
import org.netbeans.spi.project.ui.support.NodeFactorySupport;
import org.netbeans.spi.project.ui.support.ProjectSensitiveActions;
import org.openide.loaders.DataObjectNotFoundException;
import org.openide.nodes.FilterNode;
import org.openide.nodes.Node;
import org.openide.util.ImageUtilities;
import org.openide.util.Lookup;
import org.openide.util.lookup.Lookups;
import org.openide.util.lookup.ProxyLookup;

public class SublageProjectNode extends FilterNode {

        private final SublageProject project;

        public SublageProjectNode(Node node, SublageProject project)
                throws DataObjectNotFoundException {
            super(node,
                    NodeFactorySupport.createCompositeChildren(
                    project,
                    "Projects/sublage-project/Nodes"),
                    new ProxyLookup(
                    new Lookup[]{
                Lookups.singleton(project),
                node.getLookup()
            }));
            this.project = project;
        }

        @Override
        public Action[] getActions(boolean arg0) {
            return new Action[]{
                CommonProjectActions.newFileAction(),
                null,
                ProjectSensitiveActions.projectCommandAction(
                    SublageActionProvider.COMMAND_BUILD, "Build", null),
                ProjectSensitiveActions.projectCommandAction(
                    SublageActionProvider.COMMAND_REBUILD, "Clean and Build ", null),
                ProjectSensitiveActions.projectCommandAction(
                    SublageActionProvider.COMMAND_CLEAN, "Clean", null),
                ProjectSensitiveActions.projectCommandAction(
                    SublageActionProvider.COMMAND_RUN, "Run", null),
                ProjectSensitiveActions.projectCommandAction(
                    SublageActionProvider.COMMAND_DEBUG, "Debug", null),
                null,
                CommonProjectActions.setAsMainProjectAction(),
                CommonProjectActions.closeProjectAction(),
                null,
                CommonProjectActions.renameProjectAction(),
                CommonProjectActions.copyProjectAction(),
                CommonProjectActions.deleteProjectAction(),
                null,
                CommonProjectActions.customizeProjectAction()};
        }

        @Override
        public Image getIcon(int type) {
            return ImageUtilities.loadImage(CUSTOMER_ICON);
        }

        @Override
        public Image getOpenedIcon(int type) {
            return getIcon(type);
        }

        @Override
        public String getDisplayName() {
            return project.getProjectDirectory().getName();
        }
    }