package sublage.parser;

import org.netbeans.api.editor.mimelookup.MimeRegistration;
import org.netbeans.spi.editor.bracesmatching.BracesMatcher;
import org.netbeans.spi.editor.bracesmatching.BracesMatcherFactory;
import org.netbeans.spi.editor.bracesmatching.MatcherContext;
import org.netbeans.spi.editor.bracesmatching.support.BracesMatcherSupport;

@MimeRegistration(mimeType = "text/x-sublagesource", service = BracesMatcherFactory.class)
public class SublageBracesMatcherFactory implements BracesMatcherFactory {

    @Override
    public BracesMatcher createMatcher(MatcherContext mc) {
        return BracesMatcherSupport.characterMatcher(mc, -1, -1,
                '<', '>', '[', ']');
    }
}
