package sublage.lexer;

import java.io.IOException;
import org.netbeans.api.lexer.Token;
import org.netbeans.spi.lexer.Lexer;
import org.netbeans.spi.lexer.LexerRestartInfo;

public class SublageLexer implements Lexer<SublageTokenId> {

    private LexicalAnalyzer analyzer;
    private LexerRestartInfo<SublageTokenId> info;

    public SublageLexer(LexerRestartInfo<SublageTokenId> info) {
        this.info = info;
        analyzer = new LexerInputLexicalAnalyzer(info.input());
    }

    @Override
    public Token<SublageTokenId> nextToken() {
        try {
            LexicalToken token = analyzer.nextToken();
            if (token.getSymbol() == LexicalSymbol.EOF) {
                return null;
            }
            //System.out.println("lexer: " + token.toString());
            return info.tokenFactory().createToken(
                    SublageLanguageHierarchy.getTokenId(token.getSymbol()));
        } catch (LexicalAnalyzer.SyntaxError ex) {
            return info.tokenFactory().createToken(
                    SublageLanguageHierarchy.getTokenId(LexicalSymbol.UNKNOWN));
        } catch (IOException ex) {
            return null;
        }
    }

    @Override
    public Object state() {
        return null;
    }

    @Override
    public void release() {
    }
}
