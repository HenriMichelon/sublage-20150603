package sublage.lexer;

import java.util.ArrayList;
import java.util.List;

public class LexicalToken {
    
    //<editor-fold>
    public static class Long extends LexicalToken {
        
        private long value;

        public Long(LexicalSymbol symbol, long value) {
            super(symbol);
            this.value = value;
        }

        public long getValue() {
            return value;
        }

        @Override
        public java.lang.String toString() {
            return super.toString() + " : " + value;
        }
        
    }
    public static class Double extends LexicalToken {
        
        private double value;

        public Double(LexicalSymbol symbol, double value) {
            super(symbol);
            this.value = value;
        }

        public double getValue() {
            return value;
        }

        @Override
        public java.lang.String toString() {
            return super.toString() + " : " + value;
        }
    }
    public static class Internal extends LexicalToken {
        
        private sublage.lang.Internal value;

        public Internal(sublage.lang.Internal value) {
            super(LexicalSymbol.INTERNAL);
            this.value = value;
        }

        public sublage.lang.Internal getValue() {
            return value;
        }

        @Override
        public java.lang.String toString() {
            return super.toString() + " : " + value;
        }
    }    
    public static class ReservedWord extends LexicalToken {
        
        private sublage.lang.ReservedWord value;
        private List<LexicalToken> code = new ArrayList<LexicalToken>();
        private List<LexicalToken> code2 = new ArrayList<LexicalToken>();

        public ReservedWord(sublage.lang.ReservedWord value) {
            super(LexicalSymbol.RESERVEDWORD);
            this.value = value;
        }

        public List<LexicalToken> getCode2() {
            return code2;
        }

        public List<LexicalToken> getCode() {
            return code;
        }

        public sublage.lang.ReservedWord getValue() {
            return value;
        }

        @Override
        public java.lang.String toString() {
            return super.toString() + " : " + value;
        }
    }
    public static class Identifier extends LexicalToken {
        
        private java.lang.String value;

        public Identifier(LexicalSymbol symbol, java.lang.String value) {
            super(symbol);
            this.value = value;
        }

        public java.lang.String getValue() {
            return value;
        }

        @Override
        public java.lang.String toString() {
            return super.toString() + " : " + value;
        }
    }
    public static class String extends LexicalToken {
        
        private java.lang.String value;

        public String(java.lang.String value) {
            super(LexicalSymbol.STRING);
            this.value = value;
        }

        public java.lang.String getValue() {
            return value;
        }

        @Override
        public java.lang.String toString() {
            return super.toString() + " : " + value;
        }
    } 
    public static class Array extends LexicalToken {
        
        private List<LexicalToken> value = new ArrayList<LexicalToken>();

        public Array(LexicalSymbol symbol) {
            super(symbol);
        }

        public List<LexicalToken> getValue() {
            return value;
        }

        @Override
        public java.lang.String toString() {
            return super.toString() + " : " + value;
        }
    }
    //</editor-fold>
 
    private LexicalSymbol symbol;

    @Override
    public java.lang.String toString() {
        return symbol.toString();
    }

    public LexicalToken(LexicalSymbol symbol) {
        this.symbol = symbol;
    }

    public LexicalSymbol getSymbol() {
        return symbol;
    }

    public void setSymbol(LexicalSymbol symbol) {
        this.symbol = symbol;
    }
    
}
