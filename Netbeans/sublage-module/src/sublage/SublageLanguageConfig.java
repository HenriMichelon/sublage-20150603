package sublage;

import sublage.lexer.SublageLanguageHierarchy;
import org.netbeans.api.lexer.Language;
import org.netbeans.modules.csl.spi.DefaultLanguageConfig;
import org.netbeans.modules.csl.spi.LanguageRegistration;

@LanguageRegistration(mimeType = "text/x-sublagesource")
public class SublageLanguageConfig extends DefaultLanguageConfig {

    @Override
    public Language getLexerLanguage() {
        return new SublageLanguageHierarchy().language();
    }

    @Override
    public String getDisplayName() {
        return "Sublage";
    }

}
