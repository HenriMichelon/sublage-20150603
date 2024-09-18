package sublage.lexer;

import org.netbeans.api.lexer.TokenId;
import sublage.lexer.LexicalSymbol;
import sublage.lexer.LexicalToken;

public class SublageTokenId implements TokenId{
    
    private final LexicalSymbol token;
    private final String categorie;

    public SublageTokenId(LexicalSymbol token) {
        this.token = token;
        categorie = token.toString().toLowerCase();
    }

    public SublageTokenId(LexicalSymbol token, String categorie) {
        this.token = token;
        this.categorie = categorie;
    }

    @Override
    public String name() {
        return token.toString();
    }

    @Override
    public int ordinal() {
        return token.ordinal();
    }

    @Override
    public String primaryCategory() {
        return categorie;
    }

    @Override
    public String toString() {
        return name() + "(" + ordinal() + ")";
    }

    public LexicalSymbol getSymbol() {
        return token;
    }
    

    
    

}
