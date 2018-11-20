package org.jmodelica.common.ast.prefixes;

public enum TypePrefixInputOutput {

    NONE(""),
    INPUT("input"),
    OUTPUT("output");
    
    private String toString;
    
    private TypePrefixInputOutput(String toString) {
        this.toString = toString;
    }
    
    public boolean isNone() {
        return this == NONE;
    }
    
    public boolean inputCausality() {
        return this == INPUT;
    }
    
    public boolean outputCausality() {
        return this == OUTPUT;
    }
    
    @Override
    public String toString() {
        return toString;
    }
}
