package sublage.parser;

import javax.swing.text.BadLocationException;
import org.netbeans.api.editor.mimelookup.MimeRegistration;
import org.netbeans.modules.editor.indent.api.IndentUtils;
import org.netbeans.modules.editor.indent.spi.Context;
import org.netbeans.modules.editor.indent.spi.ExtraLock;
import org.netbeans.modules.editor.indent.spi.IndentTask;

public class SublageIndentTask implements IndentTask {

    @MimeRegistration(mimeType = "text/x-sublagesource", service = IndentTask.Factory.class)
    public static class SublageIndentTaskFactory implements IndentTask.Factory {

        @Override
        public IndentTask createTask(Context context) {
            return new SublageIndentTask(context);
        }
    }
    private Context context;

    SublageIndentTask(Context context) {
        this.context = context;
    }

    @Override
    public void reindent() throws BadLocationException {
        if (context.isIndent()) {
            int caretOffset = context.caretOffset();
            int lineOffset = context.lineStartOffset(caretOffset);
            if (lineOffset > 0) {
                int lastLineOffset = context.lineStartOffset(lineOffset - 1);
                int lastLineIndent = context.lineIndent(lastLineOffset);
                String lastLine = context.document().getText(lastLineOffset, lineOffset - lastLineOffset);
                int indent = lastLineIndent;
                if (isBlockOpener(lastLine)) {
                    indent += IndentUtils.indentLevelSize(context.document());
                } else if (isBlockCloser(lastLine)) {
                    indent -= IndentUtils.indentLevelSize(context.document());
                }
                context.modifyIndent(lineOffset, (indent > 0 ? indent : 0));
            }
        }
    }

    private boolean isBlockCloser(String line) {
        for (int i = line.length() - 1; i >= 0; i--) {
            char charAt = line.charAt(i);
            if (Character.isWhitespace(charAt)) {
                continue;
            } else if (charAt == '>') {
                if (i > 0) {
                    if (line.charAt(i - 1) == '>') {
                        return true;
                    }
                }
                return false;
            } else {
                return false;
            }
        }
        return false;
    }

    private boolean isBlockOpener(String line) {
        for (int i = line.length() - 1; i >= 0; i--) {
            char charAt = line.charAt(i);
            if (Character.isWhitespace(charAt)) {
                continue;
            } else if (charAt == '<') {
                if (i > 0) {
                    if (line.charAt(i - 1) == '<') {
                        return true;
                    }
                }
                return false;
            } else {
                return false;
            }
        }
        return false;
    }

    @Override
    public ExtraLock indentLock() {
        return null;
    }
}