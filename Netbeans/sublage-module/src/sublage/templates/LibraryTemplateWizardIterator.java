package sublage.templates;

import org.netbeans.api.templates.TemplateRegistration;
import org.netbeans.spi.project.ui.templates.support.Templates;
import org.openide.WizardDescriptor;

@TemplateRegistration(folder = "Sublage",
        displayName = "Sublage library source",
        iconBase = "sublage/source.png",
        content = "LibraryTemplate.library.source")
public final class LibraryTemplateWizardIterator extends SourceTemplateWizardIterator {

    @Override
    public void initialize(WizardDescriptor wizard) {
        super.initialize(wizard);
        Templates.setTargetName(wizard, "newfile.library");
    }
    
}
