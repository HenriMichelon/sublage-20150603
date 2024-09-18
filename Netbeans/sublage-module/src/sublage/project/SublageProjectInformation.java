package sublage.project;

import java.beans.PropertyChangeListener;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import org.netbeans.api.annotations.common.StaticResource;
import org.netbeans.api.project.Project;
import org.netbeans.api.project.ProjectInformation;
import org.openide.util.ImageUtilities;

public class SublageProjectInformation implements ProjectInformation {

    private SublageProject project;

    public SublageProjectInformation(SublageProject project) {
        this.project = project;
    }
    @StaticResource()
    public static final String SUBLAGE_PROJECT_ICON = "sublage/sublage.png";

    @Override
    public Icon getIcon() {
        return new ImageIcon(ImageUtilities.loadImage(SUBLAGE_PROJECT_ICON));
    }

    @Override
    public String getName() {
        return project.getProjectDirectory().getName();
    }

    @Override
    public String getDisplayName() {
        return getName();
    }

    @Override
    public void addPropertyChangeListener(PropertyChangeListener pcl) {
    }

    @Override
    public void removePropertyChangeListener(PropertyChangeListener pcl) {
    }

    @Override
    public Project getProject() {
        return project;
    }
}
