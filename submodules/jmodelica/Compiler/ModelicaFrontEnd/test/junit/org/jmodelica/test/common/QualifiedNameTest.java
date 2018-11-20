package org.jmodelica.test.common;

import static org.junit.Assert.assertEquals;

import org.jmodelica.util.QualifiedName;
import org.jmodelica.util.exceptions.NameFormatException;
import org.junit.Test;

public class QualifiedNameTest {

    @Test
    public void globalWithQuotedContainingExcapedDot() {
        assertEquals("['quotedWith.Dot\\'.', secondPart]", 
                new QualifiedName(".'quotedWith.Dot\\'.'.secondPart").toString());
    }

    @Test
    public void globalDotted()  {
        assertEquals("[first, second, third]", 
                new QualifiedName(".first.second.third").toString());
    }

    @Test
    public void global() {
        assertEquals("[global]", 
                new QualifiedName(".global").toString());
    }

    @Test
    public void qoutedDotted() {
        assertEquals("['first', second, 'third']", 
                new QualifiedName("'first'.second.'third'").toString());
    }

    @Test
    public void quotedDot() {
        assertEquals("[first, '.', 'third']",
                new QualifiedName(("first.'.'.'third'")).toString());
    }

    @Test(expected=NameFormatException.class)
    public void quotesWithoutNewPathfails() {
        assertEquals("['quoted''''', second]", 
                new QualifiedName("'quoted'''''.second"));
    }

    @Test
    public void shortNameFirst() {
        assertEquals("[A, 'B', C, D]", new QualifiedName(("A.'B'.C.D")).toString());
    }

    @Test
    public void nameFromUnqualifiedImport() {
        assertEquals("[A, B, C]", new QualifiedName("A.B.C.*").toString());
    }

    @Test(expected=NameFormatException.class)
    public void missplacedQuote() {
        new QualifiedName("first.secon'd.third'");
    }

    @Test(expected=NameFormatException.class)
    public void unmatchedQuotes() {
        new QualifiedName("first.'unclosedPart");
    }

    @Test(expected=NameFormatException.class)
    public void emptyNames() {
        new QualifiedName("first...last");
    }

    @Test(expected=NameFormatException.class)
    public void emptyNamesQuoted()  {
        new QualifiedName("first...'last'");
    }

    @Test(expected=NameFormatException.class)
    public void noName()  {
        new QualifiedName("");
    }
}
