package sublage.project;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import org.netbeans.api.project.Project;
import org.netbeans.spi.project.CopyOperationImplementation;
import org.netbeans.spi.project.DeleteOperationImplementation;
import org.netbeans.spi.project.MoveOperationImplementation;
import org.openide.filesystems.FileObject;

public class SublageProjectOperations implements DeleteOperationImplementation,
        CopyOperationImplementation, MoveOperationImplementation {

    private SublageProject project;

    public SublageProjectOperations(SublageProject project) {
        this.project = project;
    }

    private void addFile(String fileName, List<FileObject> result) {
        FileObject file = project.getProjectDirectory().getFileObject(fileName);
        if (file != null) {
            result.add(file);
        }
    }

    @Override
    public void notifyDeleting() throws IOException {
    }

    @Override
    public void notifyDeleted() throws IOException {
    }

    @Override
    public List<FileObject> getMetadataFiles() {
        List<FileObject> l = new ArrayList<FileObject>();
        addFile("nbsublage", l);
        return l;
    }

    @Override
    public List<FileObject> getDataFiles() {
        List<FileObject> l = new ArrayList<FileObject>();
        addFile("build.xml", l);
        addFile(SublageProject.SOURCES_DIR, l);
        addFile("lib", l);
        addFile(SublageProject.BINARIES_DIR, l);
        return l;
    }

    @Override
    public void notifyCopying() throws IOException {
    }

    @Override
    public void notifyCopied(Project original, File originalPath, String nueName) throws IOException {
    }

    @Override
    public void notifyMoving() throws IOException {
    }

    @Override
    public void notifyMoved(Project original, File originalPath, String nueName) throws IOException {
    }
}