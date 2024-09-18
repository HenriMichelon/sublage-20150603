package sublage.parser;

import sublage.lexer.SublageTokenId;
import javax.swing.event.DocumentEvent;
import javax.swing.text.BadLocationException;
import javax.swing.text.Document;
import org.netbeans.api.editor.fold.Fold;
import org.netbeans.api.editor.fold.FoldHierarchy;
import org.netbeans.api.editor.fold.FoldType;
import org.netbeans.api.editor.mimelookup.MimeRegistration;
import org.netbeans.api.lexer.Token;
import org.netbeans.api.lexer.TokenHierarchy;
import org.netbeans.api.lexer.TokenSequence;
import org.netbeans.spi.editor.fold.FoldHierarchyTransaction;
import org.netbeans.spi.editor.fold.FoldManager;
import org.netbeans.spi.editor.fold.FoldManagerFactory;
import org.netbeans.spi.editor.fold.FoldOperation;
import org.openide.util.Exceptions;
import sublage.lexer.LexicalSymbol;

public class SublageFoldManager implements FoldManager {

    @MimeRegistration(mimeType = "text/x-sublagesource", service = FoldManagerFactory.class)
    public static class SublageFoldManagerFactory implements FoldManagerFactory {

        @Override
        public FoldManager createFoldManager() {
            return new SublageFoldManager();
        }
    }
    private FoldOperation operation;
    public static final FoldType FUNCTION_FOLD_TYPE = new FoldType("<<...>>");

    @Override
    public void init(FoldOperation operation) {
        this.operation = operation;
    }

    @Override
    public void initFolds(FoldHierarchyTransaction transaction) {
        FoldHierarchy hierarchy = operation.getHierarchy();
        Document document = hierarchy.getComponent().getDocument();
        TokenHierarchy<Document> hi = TokenHierarchy.get(document);
        TokenSequence<SublageTokenId> ts = (TokenSequence<SublageTokenId>) hi.tokenSequence();
        int start = -1;
        int offset = 0;
        while (ts.moveNext()) {
            offset = ts.offset();
            Token<SublageTokenId> token = ts.token();
            SublageTokenId id = token.id();
            if ((start == -1) && (id.getSymbol() == LexicalSymbol.STARTFUNC)) {
                start = offset;
            } else if ((start != -1) && (id.getSymbol() == LexicalSymbol.ENDFUNC)) {
                try {
                    operation.addToHierarchy(
                            FUNCTION_FOLD_TYPE,
                            FUNCTION_FOLD_TYPE.toString(),
                            false,
                            start,
                            offset + token.length(),
                            0,
                            0,
                            hierarchy,
                            transaction);
                    start = -1;
                } catch (BadLocationException ex) {
                    Exceptions.printStackTrace(ex);
                }
            }
        }
    }

    @Override
    public void insertUpdate(DocumentEvent de, FoldHierarchyTransaction fht) {
    }

    @Override
    public void removeUpdate(DocumentEvent de, FoldHierarchyTransaction fht) {
    }

    @Override
    public void changedUpdate(DocumentEvent de, FoldHierarchyTransaction fht) {
    }

    @Override
    public void removeEmptyNotify(Fold fold) {
    }

    @Override
    public void removeDamagedNotify(Fold fold) {
    }

    @Override
    public void expandNotify(Fold fold) {
    }

    @Override
    public void release() {
    }
}