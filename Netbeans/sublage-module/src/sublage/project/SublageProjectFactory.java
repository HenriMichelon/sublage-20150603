package sublage.project;

import java.io.IOException;
import org.netbeans.api.project.Project;
import org.netbeans.spi.project.ProjectFactory;
import org.netbeans.spi.project.ProjectState;
import org.openide.filesystems.FileObject;
import org.openide.util.lookup.ServiceProvider;

@ServiceProvider(service= ProjectFactory.class)
public class SublageProjectFactory implements ProjectFactory{
    
    public static final String PROJECT_FILE = "nbsublage";

    @Override
    public boolean isProject(FileObject projectDirectory) {
        return projectDirectory.getFileObject(PROJECT_FILE) != null;
    }

    @Override
    public Project loadProject(FileObject projectDirectory, ProjectState state) throws IOException {
        return isProject(projectDirectory) ? new SublageProject(projectDirectory, state) : null;
    }

    @Override
    public void saveProject(Project project) throws IOException, ClassCastException {
    }
    
}
