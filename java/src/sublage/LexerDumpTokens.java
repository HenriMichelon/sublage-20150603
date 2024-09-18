package sublage;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import sublage.lexer.*;

public class LexerDumpTokens {

    public static void usage() {
        System.out.println(LexerDumpTokens.class.getName() + " sourcefile");
        System.exit(1);
    }

    public static void main(String[] args) {
        if (args.length < 1) {
            usage();
        }
        String sourcefilename = args[0];
        try {
            FileInputStream input = new FileInputStream(sourcefilename);
            LexicalAnalyzer lexicalAnalyzer = new LexicalAnalyzer(
                    input,
                    true);
            LexicalToken token;
            try {
                while ((token = lexicalAnalyzer.nextToken()) != null) {
                    System.out.println(token);
                    if (token.getSymbol() == LexicalSymbol.EOF) {
                        break;
                    }
                }
            } catch (sublage.lexer.LexicalAnalyzer.SyntaxError ex) {
                System.err.println(ex.getMessage());
            }
        } catch (FileNotFoundException ex) {
            System.err.println("`" + sourcefilename + "` file not found ");
            System.exit(2);
        } catch (IOException ex) {
            System.err.println("error reading `" + sourcefilename + "`");
            System.exit(2);
        }
    }
}
