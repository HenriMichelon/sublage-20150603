package sublage.project.properties;

import sublage.project.SublageProject;
import java.awt.Dialog;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.HashMap;
import java.util.Map;
import javax.swing.JComponent;
import javax.swing.JPanel;
import org.netbeans.api.project.ProjectUtils;
import org.netbeans.spi.project.ui.CustomizerProvider;
import org.netbeans.spi.project.ui.support.ProjectCustomizer;

public class SublageCustomizerProvider implements CustomizerProvider {

    private static class PanelProvider implements ProjectCustomizer.CategoryComponentProvider {

        private Map<ProjectCustomizer.Category, JPanel> panels;
        private JPanel EMPTY_PANEL = new JPanel();

        public PanelProvider(Map<ProjectCustomizer.Category, JPanel> panels) {
            this.panels = panels;
        }

        @Override
        public JComponent create(ProjectCustomizer.Category category) {
            JComponent panel = (JComponent) panels.get(category);
            return panel == null ? EMPTY_PANEL : panel;
        }
    }

    private class OptionListener implements ActionListener {
        @Override
        public void actionPerformed(ActionEvent e) {
        }
    }
    public final SublageProject project;
    private ProjectCustomizer.Category[] categories;
    public static final String CUSTOMIZER_FOLDER_PATH =
            "Projects/sublage-project/Customizer";
    public static final String RUN_CATEGORY = "Run";
    public static final String DEBUG_CATEGORY = "Debug";

    public SublageCustomizerProvider(SublageProject project) {
        this.project = project;
    }

    @Override
    public void showCustomizer() {
        ProjectCustomizer.Category runCategory = ProjectCustomizer.Category.create(
                RUN_CATEGORY, RUN_CATEGORY, null);
        ProjectCustomizer.Category debugCategory = ProjectCustomizer.Category.create(
                DEBUG_CATEGORY, DEBUG_CATEGORY, null);
        categories = new ProjectCustomizer.Category[]{ runCategory, debugCategory };
        
        Map<ProjectCustomizer.Category, JPanel> panels =
                new HashMap<ProjectCustomizer.Category, JPanel>();
        panels.put(runCategory, new PanelRunProperties(project));
        panels.put(debugCategory, new PanelDebugProperties(project));

        OptionListener listener = new OptionListener();
        Dialog dialog = ProjectCustomizer.createCustomizerDialog(
                categories,
                new PanelProvider(panels),
                null, listener, null);
        dialog.setTitle(ProjectUtils.getInformation(project).getDisplayName());
        dialog.setVisible(true);
    }

}
