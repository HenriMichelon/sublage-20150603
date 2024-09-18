package sublage.lexer;

import java.io.IOException;
import org.netbeans.spi.lexer.LexerInput;
import org.openide.util.Exceptions;
import sublage.lexer.LexicalAnalyzer;

public class LexerInputLexicalAnalyzer extends LexicalAnalyzer {
    
    private LexerInput input;

    public LexerInputLexicalAnalyzer(LexerInput input) {
        super(true);
        this.input = input;
    }

    @Override
    protected void prevChar() throws IOException {
        input.backup(1);
    }

    @Override
    protected int readChar() throws IOException {
        int c = input.read();
        if (c == LexerInput.EOF) {
            return -1;
        }
        return c;
    }
    
}
