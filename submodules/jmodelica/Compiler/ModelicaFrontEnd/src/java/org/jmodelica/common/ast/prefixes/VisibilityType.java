package org.jmodelica.common.ast.prefixes;

public enum VisibilityType {

    PUBLIC,
    PROTECTED,
    TEMPORARY,
    EXPANDABLE,
    RUNTIME_OPTION,
    HIDDEN;
    
    public boolean isPublic() {
        return this == PUBLIC;
    }

    public boolean isProtected() {
        return this == PROTECTED;
    }
    
    public boolean isTemporary() {
        return this == TEMPORARY;
    }

    public boolean isFromExpandableConnector() {
        return this == EXPANDABLE;
    }

    public boolean isRuntimeOptionVisibility() {
        return this == RUNTIME_OPTION;
    }
    
    public boolean isHidden() {
        return this == HIDDEN;
    }

}
