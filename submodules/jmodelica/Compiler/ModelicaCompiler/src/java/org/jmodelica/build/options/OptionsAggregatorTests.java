package org.jmodelica.build.options;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.fail;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.OutputStream;
import java.io.OutputStreamWriter;
import java.io.StringReader;

import org.jmodelica.build.options.OptionsAggregator.OptionsAggregationException;
import org.junit.Test;

public class OptionsAggregatorTests {

    private class StringOutputStream extends OutputStream {
        private StringBuilder sb = new StringBuilder();
        @Override
        public void write(int b) throws IOException {
            this.sb.append((char) b );
        }
        @Override
        public String toString() {
            return sb.toString();
        }
    }
    
    OptionsAggregator setup(String s) throws IOException, OptionsAggregationException {
        OptionsAggregator op = new OptionsAggregator();
        BufferedReader reader = new BufferedReader(new StringReader(s));
        try {
            op.parseFile("/file/path", reader);
        } finally {
            reader.close();
        }
        return op;
    }
    
    @Test
    public void testLongLine() throws IOException, OptionsAggregationException {
        try {
            setup(""
                    + "BOOLEAN opt1 compiler user true"
                    + "BOOLEAN opt2 compiler user true"
                );
            fail();
        } catch (OptionsAggregationException e) {
            String expected = "Too many parts on the line! /file/path\nBOOLEAN opt1 compiler user trueBOOLEAN opt2 compiler user true";
            assertEquals(expected, e.getMessage());
        }
    }
    
    @Test
    public void testSameName() throws IOException, OptionsAggregationException {
        try {
            setup(""
                    + "BOOLEAN opt1 compiler user true\n"
                    + "\n"
                    + "\"\"\n"
                    + "\n"
                    + "BOOLEAN opt1 compiler user true\n"
                );
            fail();
        } catch (OptionsAggregationException e) {
            assertEquals("Found duplicated option declaration for opt1. Old declaration from /file/path. New declaration from /file/path", e.getMessage());
        }
    }
    
    @Test
    public void testModifyNonExistent() throws IOException, OptionsAggregationException {
        OptionsAggregator op = setup("DEFAULT opt1 false\n");
        try {
            op.modify();
            fail();
        } catch (OptionsAggregationException e) {
            assertEquals("Missing option for modification DEFAULT opt1", e.getMessage());
        }
    }
    
    @Test
    public void testDoubleModification() throws IOException, OptionsAggregationException {
        OptionsAggregator op = setup(""
                + "BOOLEAN opt1 compiler user true\n"
                + "\n"
                + "\"\"\n"
                + "\n"
                + "DEFAULT opt1 false\n"
                + "\n"
                + "DEFAULT opt1 false\n"
                );
        try {
            op.modify();
            fail();
        } catch (OptionsAggregationException e) {
            assertEquals("Option already modified opt1", e.getMessage());
        }
    }

    @Test
    public void testFull() throws IOException, OptionsAggregationException {
        OptionsAggregator op = setup(""
                + "BOOLEAN opt1 compiler user true\n"
                + "\n"
                + "\"\"\n"
                + "\n"
                + "BOOLEAN opt2 runtime experimental false\n"
                + "\n"
                + "\"A description\"\n"
                );
        op.modify();
        StringOutputStream os = new StringOutputStream();
        OutputStreamWriter osw = new OutputStreamWriter(os);
        op.generate(osw, "org.pack");
        osw.close();
        String expected = 
                "package org.pack;\n" + 
                "import java.util.LinkedHashMap;\n" + 
                "import java.util.Map;\n" + 
                "\n" + 
                "import org.jmodelica.common.options.Option;\n" + 
                "import org.jmodelica.common.options.OptionRegistry;\n" + 
                "import org.jmodelica.common.options.OptionRegistry.Category;\n" + 
                "import org.jmodelica.common.options.OptionRegistry.OptionType;\n" + 
                "\n" + 
                "public class OptionsAggregated {\n" + 
                "    public static void addTo(OptionRegistry options) {\n" +
                "        options.addBooleanOption(\"opt1\", OptionType.compiler, Category.user, true, true, \"\");\n" + 
                "        options.addBooleanOption(\"opt2\", OptionType.runtime, Category.experimental, false, false, \"A description\");\n" +  
                "    }\n" + 
                "}\n" + 
                "";
        assertEquals(expected, os.toString());
    }
    
    @Test
    public void testModification() throws IOException, OptionsAggregationException {
        OptionsAggregator op = setup(""
                + "BOOLEAN opt1 compiler user true\n"
                + "\n"
                + "\"\"\n"
                + "\n"
                + "DEFAULT opt1 false\n"
                + "\n"
                );
        op.modify();
        StringOutputStream os = new StringOutputStream();
        OutputStreamWriter osw = new OutputStreamWriter(os);
        op.generateCalls(osw);
        osw.close();
        String expected = "        options.addBooleanOption(\"opt1\", OptionType.compiler, Category.user, false, true, \"\");\n";
        assertEquals(expected, os.toString());
    }
    
    @Test
    public void testInvert() throws IOException, OptionsAggregationException {
        OptionsAggregator op = setup(""
                + "BOOLEAN opt1 compiler user true\n"
                + "\n"
                + "\"\"\n"
                + "\n"
                + "BOOLEAN opt2 compiler user true\n"
                + "\n"
                + "\"\"\n"
                + "\n"
                + "INVERT opt1 opt2\n"
                + "\n"
            );
        op.modify();
        StringOutputStream os = new StringOutputStream();
        OutputStreamWriter osw = new OutputStreamWriter(os);
        op.generateCalls(osw);
        osw.close();
        String expected = 
                    "        options.addBooleanOption(\"opt1\", OptionType.compiler, Category.user, options.new DefaultInvertBoolean(\"opt2\"), options.new DefaultInvertBoolean(\"opt2\"), \"\");\n" + 
                    "        options.addBooleanOption(\"opt2\", OptionType.compiler, Category.user, true, true, \"\");\n";
        assertEquals(expected, os.toString());
    }
}
