package sublage.lexer;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import org.netbeans.spi.lexer.LanguageHierarchy;
import org.netbeans.spi.lexer.Lexer;
import org.netbeans.spi.lexer.LexerRestartInfo;
import sublage.lexer.LexicalSymbol;

public class SublageLanguageHierarchy extends LanguageHierarchy<SublageTokenId> {
    
    private static List<SublageTokenId> tokenids = new ArrayList<SublageTokenId>();
    
    public static SublageTokenId getTokenId(LexicalSymbol symbol) {
        for (SublageTokenId tokenid : tokenids) {
            if (tokenid.getSymbol() == symbol) {
                return tokenid;
            }
        }
        return tokenids.get(0);
    }

    @Override
    protected Collection<SublageTokenId> createTokenIds() {
        for (LexicalSymbol ls : LexicalSymbol.values()) {
            tokenids.add(new SublageTokenId(ls));
        }
        return tokenids;
    }

    @Override
    protected Lexer<SublageTokenId> createLexer(LexerRestartInfo<SublageTokenId> info) {
        return new SublageLexer(info);
    }

    @Override
    protected String mimeType() {
        return "text/x-sublagesource";
    }

}
