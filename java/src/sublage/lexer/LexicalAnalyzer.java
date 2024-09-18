package sublage.lexer;

import sublage.lang.Internal;
import sublage.lang.ReservedWord;
import java.io.IOException;
import java.io.InputStream;
import java.io.PushbackInputStream;

public class LexicalAnalyzer {

    public class SyntaxError extends Exception {

        public SyntaxError(String message) {
            super(message);
        }
    }
    private static final int[] spaces = {' ', '\n', '\t', '\r', '\f'};
    private static final int[] separators = {'"', '<', '>'};
    private PushbackInputStream input;
    protected int currentChar;
    protected long currentLine;
    private boolean allTokens = false;

    public LexicalAnalyzer(boolean allTokens) {
        this.allTokens = allTokens;
    }

    public LexicalAnalyzer(InputStream input, boolean allTokens) throws IOException {
        this.allTokens = allTokens;
        this.input = new PushbackInputStream(input);
    }

    public long getCurrentLine() {
        return currentLine;
    }

    //<editor-fold>
    private boolean isSpace() {
        for (int i : spaces) {
            if (i == currentChar) {
                return true;
            }
        }
        return false;
    }

    private boolean isSeparator() {
        if (isSpace()) {
            return true;
        }
        for (int i : separators) {
            if (i == currentChar) {
                return true;
            }
        }
        return false;
    }

    private boolean isDecimalDigit() {
        return (currentChar >= '0') && (currentChar <= '9');
    }

    private boolean isHexadecimentDigit() {
        return ((currentChar >= '0') && (currentChar <= '9'))
                || ((currentChar >= 'a') && (currentChar <= 'f'))
                || ((currentChar >= 'A') && (currentChar <= 'F'));
    }
    //</editor-fold>

    protected int readChar() throws IOException {
        return input.read();
    }

    protected void prevChar() throws IOException {
        input.unread(currentChar);
    }

    private void nextChar() throws IOException {
        /* we reach the end of the line at last call,
         *  increment the current line number.
         *  Note this is not done at the end of the call
         *  to avoid a bad line number in case of an error
         *  on the last line of the source file */
        if (currentChar == '\n') {
            ++currentLine;
        }
        if ((currentChar = readChar()) == -1) {
            currentChar = 0;
        }
    }

    private boolean skipSpace() throws IOException {
        boolean spacesFound = false;
        nextChar();
        while (isSpace() && (currentChar != '\0')) {
            spacesFound = true;
            nextChar();
        }
        prevChar();
        return spacesFound;
    }

    private LexicalToken readIdent(String str) throws IOException {
        StringBuilder s = new StringBuilder(str);
        // An identifier is followed by a separator character
        while (!isSeparator()) {
            s.append((char) currentChar);
            nextChar();
            if (currentChar == 0) {
                break;
            }
        }
        prevChar();
        String ident = s.toString();
        long len = s.length();
        if (len > 0) {
            /* check if we have a reserved word
             or a simple identifer */
            Internal internal = Internal.toInternal(ident);
            if (internal != null) {
                return new LexicalToken.Internal(internal);
            }
            ReservedWord rword = ReservedWord.toReservedWord(ident);
            if (rword != null) {
                return new LexicalToken.ReservedWord(rword);
            }
            if (ident.startsWith("->")) {
                return new LexicalToken.Identifier(LexicalSymbol.VARSET,
                        ident.substring(2));
            }
            int max = (ident.length() > 100 ? 99 : ident.length());
            return new LexicalToken.Identifier(LexicalSymbol.IDENTIFIER,
                    ident.substring(0, max));
        }
        return null;
    }

    private LexicalToken readDecimal(StringBuilder s) throws IOException, SyntaxError {
        int sign = 1;
        if (currentChar == '-') {
            sign = -1;
            nextChar();
            if (isSpace()) {
                prevChar();
                return new LexicalToken.Internal(Internal.SUB);
            } else if (currentChar == '>') {
                nextChar();
                return readIdent("->");
            }
        }
        /* read the source file until we found a
         *      non decimal digit character */
        while (isDecimalDigit()) {
            s.append((char) currentChar);
            nextChar();
        }
        prevChar();
        /* check if we have a floating point number */
        boolean isDecimalFloat = false;
        if (currentChar == '.') {
            isDecimalFloat = true;
            nextChar();
            do {
                s.append((char) currentChar);
                nextChar();
            } while (isDecimalDigit());
            prevChar();
        }
        /* decimal numbers MUST be followed by a separator or a space */
        if (!isSeparator()) {
            throw new SyntaxError("incorrect decimal number");
        }
        if (s.length() > 0) {
            if (isDecimalFloat) {
                return new LexicalToken.Double(LexicalSymbol.DECIMAL,
                        Double.parseDouble(s.toString()) * sign);
            } else {
                return new LexicalToken.Long(LexicalSymbol.DECIMAL,
                        Long.parseLong(s.toString()) * sign);
            }
        } else {
            return new LexicalToken.Long(LexicalSymbol.DECIMAL, 0);
        }
    }

    private LexicalToken readNumber() throws IOException, SyntaxError {
        StringBuilder s = new StringBuilder();
        // check if we have an hexadecimal (start with 0x) or a decimal number
        if (currentChar == '0') {
            nextChar();
            // an hexadecimal number
            if (currentChar == 'x') {
                nextChar();
                /* read the source file until we found a
                 non hexadecimal digit character */
                while (isHexadecimentDigit()) {
                    s.append((char) currentChar);
                    nextChar();
                }
                /* hexadecimal numbers MUST be followed by a separator or a space */
                if (!isSeparator()) {
                    throw new SyntaxError("incorrect hexadecimal number");
                }
                if (s.length() > 0) {
                    prevChar();
                    return new LexicalToken.Long(LexicalSymbol.DECIMAL,
                            Long.parseLong(s.toString(), 16));
                } else {
                    throw new SyntaxError("internal error");
                }
            }// a decimal number
            else {
                return readDecimal(s);
            }
        }// a decimal number
        else {
            return readDecimal(s);
        }
    }

    private LexicalToken readString() throws IOException, SyntaxError {
        StringBuilder s = new StringBuilder();
        nextChar();
        /* read the source file until end of string,
         *      or end of file (note : we can have multi lines strings) */
        while ((currentChar != '"')
                && (currentChar != 0)) {
            // check if we have an escaped character
            if (currentChar == '\\') {
                nextChar();
                if (currentChar != 0) {
                    int char_to_add = currentChar;
                    // TODO: \num, \0num and \c
                    switch (currentChar) {
                        case 'b':
                            char_to_add = '\b';
                            break;
                        case 'f':
                            char_to_add = '\f';
                            break;
                        case 'n':
                            char_to_add = '\n';
                            break;
                        case 'r':
                            char_to_add = '\r';
                            break;
                        case 't':
                            char_to_add = '\t';
                            break;
                        default:
                            break;
                    }
                    s.append((char) char_to_add);
                    nextChar();
                } else {
                    throw new SyntaxError("incorrect espace sequence in string");
                }
            }// a simple character, just add it to the string
            else {
                s.append((char) currentChar);
                nextChar();
            }
        }
        // check if we have reached the end of file before the end of string
        if (currentChar == '"') {
            return new LexicalToken.String(s.toString());
        } else {
            throw new SyntaxError("unterminated string");
        }
    }

    public LexicalToken nextToken() throws IOException, SyntaxError {
        LexicalToken token;
        // find the first character of the next symbol
        if (skipSpace() && allTokens) {
            return new LexicalToken(LexicalSymbol.SPACE);
        }
        nextChar();
        switch (currentChar) {
            case 0:
                token = new LexicalToken(LexicalSymbol.EOF);
                break;
            case '<':
                nextChar();
                if (currentChar == '<') {
                    token = new LexicalToken(LexicalSymbol.STARTFUNC);
                    break;
                } else if (currentChar == '>') {
                    Internal id = Internal.toInternal("<>");
                    if (id != null) {
                        token = new LexicalToken.Internal(id);
                        break;
                    }
                } else if (currentChar == '=') {
                    Internal id = Internal.toInternal("<=");
                    if (id != null) {
                        token = new LexicalToken.Internal(id);
                        break;
                    }
                } else if (isSeparator()) {
                    Internal id = Internal.toInternal("<");
                    if (id != null) {
                        token = new LexicalToken.Internal(id);
                        break;
                    }
                }
                throw new SyntaxError("syntax error");
            case '>':
                nextChar();
                if (currentChar == '>') {
                    token = new LexicalToken(LexicalSymbol.ENDFUNC);
                    break;
                } else if (currentChar == '=') {
                    Internal id = Internal.toInternal(">=");
                    if (id != null) {
                        token = new LexicalToken.Internal(id);
                        break;
                    }
                } else if (isSeparator()) {
                    Internal id = Internal.toInternal(">");
                    if (id != null) {
                        token = new LexicalToken.Internal(id);
                        break;
                    }
                }
                throw new SyntaxError("syntax error");
            case '"':
                token = readString();
                break;
            case '[':
                token = new LexicalToken(LexicalSymbol.STARTARRAY);
                break;
            case ']':
                token = new LexicalToken(LexicalSymbol.ENDARRAY);
                break;
            case '@':
                token = new LexicalToken(LexicalSymbol.REF);
                break;
            case '#': {
                do {
                    if ((currentChar = readChar()) == -1) {
                        currentChar = 0;
                    }
                } while ((currentChar != '\n') && (currentChar != 0));
                if (currentChar == 0) {
                    token = new LexicalToken(LexicalSymbol.EOF);
                } else if (allTokens) {
                    token = new LexicalToken(LexicalSymbol.ONELINECOMMENT);
                } else {
                    return nextToken();
                }
                break;
            }
            default:
                /* decimal and hexadecimal (0x) numbers
                 starts with a decimal digit */
                if (isDecimalDigit()
                        || (currentChar == '-')) {
                    return readNumber();
                }
                /* all unknown symbols are identifiers */
                return readIdent("");
        }
        return token;
    }
}
