package sublage.debugger.annotations;

import sublage.debugger.objects.SublageBreakpoint;
import sublage.lexer.SublageTokenId;
import javax.swing.text.Element;
import javax.swing.text.StyledDocument;
import org.netbeans.api.debugger.Breakpoint;
import org.netbeans.api.lexer.TokenHierarchy;
import org.netbeans.api.lexer.TokenId;
import org.netbeans.api.lexer.TokenSequence;
import org.openide.cookies.EditorCookie;
import org.openide.loaders.DataObject;
import org.openide.text.Annotatable;
import org.openide.text.DataEditorSupport;
import org.openide.text.Line;
import org.openide.text.NbDocument;
import sublage.lexer.LexicalSymbol;

public class SublageBreakpointAnnotation extends org.netbeans.spi.debugger.ui.BreakpointAnnotation {
    
    public static final String BREAKPOINT_ANNOTATION_TYPE = "Breakpoint";
    private static final String BREAKPOINT = "Breakpoint";
    private Breakpoint breakpoint;
    
    public SublageBreakpointAnnotation(Annotatable annotatable, Breakpoint breakpoint) {
        this.breakpoint = breakpoint;
        attach(annotatable);
    }

    /* (non-Javadoc)
     * @see org.openide.text.Annotation#getAnnotationType()
     */
    @Override
    public String getAnnotationType() {
        if (breakpoint instanceof SublageBreakpoint) {
            SublageBreakpoint lineBreakpoint = (SublageBreakpoint) breakpoint;
            Line line = lineBreakpoint.getLine();
            DataObject dataObject = DataEditorSupport.findDataObject(line);
            EditorCookie editorCookie = (EditorCookie) dataObject.getLookup().lookup(EditorCookie.class);
            final StyledDocument document = editorCookie.getDocument();
            if (document != null) {
                final boolean[] isValid = new boolean[1];
                isValid[0] = false;
                try {
                    int l = line.getLineNumber();
                    Element lineElem = NbDocument.findLineRootElement(document).getElement(l);
                    final int startOffset = lineElem.getStartOffset();
                    final int endOffset = lineElem.getEndOffset();
                    document.render(new Runnable() {
                        @Override
                        public void run() {
                            TokenHierarchy th = TokenHierarchy.get(document);
                            TokenSequence<TokenId> ts = th.tokenSequence();
                            if (ts != null) {
                                ts.move(startOffset);
                                boolean moveNext = ts.moveNext();
                                boolean startByIdentifier = false;
                                for (; moveNext && !isValid[0] && ts.offset() < endOffset;) {
                                    SublageTokenId id = (SublageTokenId) ts.token().id();
                                    if ((startByIdentifier)
                                            && (id.getSymbol() != LexicalSymbol.SPACE)) {
                                        isValid[0] = id.getSymbol() != LexicalSymbol.STARTFUNC;
                                        startByIdentifier = false;
                                        break;
                                    }
                                    if ((id.getSymbol() == LexicalSymbol.ONELINECOMMENT)
                                            || (id.getSymbol() == LexicalSymbol.STARTFUNC)
                                            || (id.getSymbol() == LexicalSymbol.ENDFUNC)) {
                                        break;
                                    }
                                    if (id.getSymbol() == LexicalSymbol.RESERVEDWORD) {
                                        String text = ts.token().text().toString();
                                        if (text.equals("import")
                                                || text.equals("native")) {
                                            break;
                                        }                                        
                                    }
                                    if (id.getSymbol() == LexicalSymbol.IDENTIFIER) {
                                        startByIdentifier = true;
                                    } else {
                                        isValid[0] = id.getSymbol() != LexicalSymbol.SEPARATOR
                                                && id.getSymbol() != LexicalSymbol.SPACE;
                                    }
                                    if (!ts.moveNext()) {
                                        break;
                                    }
                                }
                                if (startByIdentifier) {
                                    isValid[0] = true;
                                }
                            }
                        }
                    });
                } catch (IndexOutOfBoundsException ex) {
                    isValid[0] = false;
                }
                if (!isValid[0]) {
                    lineBreakpoint.setInvalid(null);
                } else {
                    lineBreakpoint.setValid(null);
                }
            }
        }
        return (breakpoint.getValidity() == Breakpoint.VALIDITY.INVALID)
                ? BREAKPOINT_ANNOTATION_TYPE + "_broken" : BREAKPOINT_ANNOTATION_TYPE;
    }
    
    @Override
    public String getShortDescription() {
        return BREAKPOINT;
    }
    
    @Override
    public Breakpoint getBreakpoint() {
        return breakpoint;
    }
}