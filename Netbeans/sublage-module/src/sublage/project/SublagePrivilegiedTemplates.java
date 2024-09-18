package sublage.project;

import org.netbeans.spi.project.ui.PrivilegedTemplates;

public class SublagePrivilegiedTemplates implements PrivilegedTemplates {

    private final String[] PRIVILEGED_NAMES = new String[]{
        "Templates/Sublage/SourceTemplate.source",
        "Templates/Sublage/LibraryTemplate.library.source",};

    @Override
    public String[] getPrivilegedTemplates() {
        return PRIVILEGED_NAMES;
    }
}
