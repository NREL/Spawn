package org.jmodelica.build.options;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedHashMap;
import java.util.Map;

public class OptionsAggregator {

    private Map<String, OptionDeclaration> options = new LinkedHashMap<>();
    private ArrayList<OptionModification> optionsModification = new ArrayList<>();

    public static class OptionsAggregationException extends Exception {
        public OptionsAggregationException(String string) {
            super(string);
        }

        private static final long serialVersionUID = -8924212971616929776L;
    }

    private static class StringBuilderVarArgs {
        private StringBuilder sb = new StringBuilder();

        public void append(String... strings) {
            for (String s : strings) {
                sb.append(s);
            }
        }

        public String toString() {
            return sb.toString();
        }
    }

    public static abstract class Option {

        protected String filePath;
        protected String kind;
        protected String name;
        protected String defaultValue;

        public Option(String filePath, String kind, String name, String defaultValue) {
            this.filePath = filePath;
            this.kind = kind;
            this.name = name;
            this.defaultValue = defaultValue;
        }

        public String getFilePath() {
            return filePath;
        }
        
        public String getKind() {
            return kind;
        }
        
        public String getName() {
            return name;
        }
    }

    private abstract class OptionModification extends Option {
        public OptionModification(String filePath, String kind, String name, String defaultValue) {
            super(filePath, kind, name, defaultValue);
        }
        
        void modify() throws OptionsAggregationException {
            OptionDeclaration opt = options.get(getName());
            if (opt == null) {
                throw new OptionsAggregationException("Missing option for modification " + getKind() + " " + getName());
            }
            opt.setModified();
            modify(opt);
        }
        abstract void modify(OptionDeclaration opt) throws OptionsAggregationException;
    }
    
    private class OptionModificationSetDefault extends OptionModification {

        public OptionModificationSetDefault(String filePath, String kind, String name, String defaultValue) {
            super(filePath, kind, name, defaultValue);
        }

        @Override
        void modify(OptionDeclaration opt) throws OptionsAggregationException {
            opt.setDefaultValue(defaultValue);
        }

    }

    private class OptionModificationRemove extends OptionModification {

        public OptionModificationRemove(String filePath, String kind, String name) throws OptionsAggregationException {
            super(filePath, kind, name, null);
        }

        @Override
        void modify(OptionDeclaration opt) throws OptionsAggregationException {
            opt.setRemoved();
        }
    }

    private class OptionModificationInvert extends OptionModification {

        public OptionModificationInvert(String filePath, String kind, String name, String defaultValue) throws OptionsAggregationException {
            super(filePath, kind, name, defaultValue);
        }

        @Override
        void modify(OptionDeclaration opt) throws OptionsAggregationException {
            String newDefaultValue = "options.new DefaultInvertBoolean(\"" + defaultValue + "\")";
            opt.setDefaultValue(newDefaultValue);
            opt.setTestDefaultValue(newDefaultValue);
        }
    }

    public static class OptionDeclaration extends Option {
        private String type;
        private String cat;

        private String testDefaultValue;
        private String[] args;
        private String description;
        private String[] possibleValues;
        private boolean removed = false;
        private boolean modified = false;

        public OptionDeclaration(String filePath, String cls, String name, String defaultValue, String type,
                String cat) {
            super(filePath, cls, name, defaultValue);
            this.testDefaultValue = defaultValue;
            this.type = type;
            this.cat = cat;
        }

        public String getType() {
            return type;
        }

        public String getCategory() {
            return cat;
        }
        
        public void setModified() throws OptionsAggregationException {
            if (modified) {
                throw new OptionsAggregationException("Option already modified " + getName());
            }
            modified = true;
        }

        public void setDefaultValue(String defaultValue) {
            this.defaultValue = defaultValue;
        }

        public void setRemoved() {
            removed = true;
        }

        public boolean getRemoved() {
            return removed;
        }

        public void setTestDefaultValue(String testDefaultValue) {
            this.testDefaultValue = testDefaultValue;
        }

        public void setArgs(String[] args) {
            this.args = args;
        }

        public void setDescription(String description) {
            this.description = description;
        }

        public void setPossibleValues(String[] possibleValues) {
            this.possibleValues = possibleValues;
        }

        public final String toJavaString() {
            StringBuilderVarArgs sb = new StringBuilderVarArgs();
            toJavaString(sb);
            return sb.toString();
        }
        
        public void toJavaString(StringBuilderVarArgs sb) {
            if (removed) {
                return;
            }
            sb.append("        options.add", kind, "Option(\"");
            sb.append(name, "\", ");
            sb.append("OptionType.", type, ", ");
            sb.append("Category.", cat, ", ");
            sb.append(defaultValue, ", ");
            sb.append(testDefaultValue, ", ");
            sb.append(description);
            if (args != null) {
                for (String s : args) {
                    sb.append(", ", s);
                }
            }
            if (possibleValues != null) {
                sb.append(", new String[] {");
                for (String s : possibleValues) {
                    sb.append(s, ", ");
                }
                sb.append("}");
            }
            sb.append(");\n");
        }
    }

    public void generateHeader(OutputStreamWriter out, String pack) throws IOException {
        out.write("package " + pack + ";\n"
                + "import java.util.LinkedHashMap;\n"
                + "import java.util.Map;\n" + "\n"
                + "import org.jmodelica.common.options.Option;\n"
                + "import org.jmodelica.common.options.OptionRegistry;\n"
                + "import org.jmodelica.common.options.OptionRegistry.Category;\n"
                + "import org.jmodelica.common.options.OptionRegistry.OptionType;\n" + "\n"
                + "public class OptionsAggregated {\n");
    }
    
    public void generateCalls(OutputStreamWriter out) throws IOException {
        for (OptionDeclaration opt : getOptions()) {
            out.write(opt.toJavaString());
        }
    }
    
    public void generate(OutputStreamWriter out, String pack) throws IOException {
        generateHeader(out, pack);
        out.write("    public static void addTo(OptionRegistry options) {\n");
        generateCalls(out);
        out.write("    }\n");
        out.write("}\n");
    }

    public String nextLine(BufferedReader reader) throws IOException {
        String line;
        while ((line = reader.readLine()) != null && isEmpty(line)) {

        }
        return line;
    }

    private boolean isEmpty(String line) {
        return line == null || line.isEmpty() || line.startsWith("***");
    }

    private String parseKind(String kind) throws OptionsAggregationException {
        return kind.substring(0, 1).toUpperCase() + kind.substring(1).toLowerCase();
    }

    /**
     * Tries to parse an option modification. Whitespace and newline sensitive.
     * 
     * DEFAULT expects an option name and a value. It will change the default value
     * of that option to the value. Example line:
     * DEFAULT opt1 true
     * 
     * REMOVE expects an option name. It will remove the option from the set that is
     * included in the generated output. Example line: 
     * REMOVE opt1
     * 
     * INVERT expects two boolean option names. The default value of the first will
     * be the inverse of the value of the second. Example line:
     * INVERT opt1 opt2
     * 
     */
    private boolean parseModification(String optionsFile, String[] parts) throws OptionsAggregationException {
        if (parts[0].equals("DEFAULT")) {
            String name = parts[1];
            String defaultValue = parts[2];
            optionsModification.add(new OptionModificationSetDefault(optionsFile, parts[0], name, defaultValue));
            return true;
        } else if (parts[0].equals("REMOVE")) {
            String name = parts[1];
            optionsModification.add(new OptionModificationRemove(optionsFile, parts[0], name));
            return true;
        } else if (parts[0].equals("INVERT")) {
            String name = parts[1];
            String defaultValue = parts[2];
            optionsModification.add(new OptionModificationInvert(optionsFile, parts[0], name, defaultValue));
            return true;
        }
        return false;
    }
    
    /**
     * Parse the first line of a declaration. Whitespace and newline sensitive.
     * Expects 
     * 1. the kind of option (BOOLEAN, STRING, etc.)
     * 2. Name
     * 3. Category
     * 4. Type
     * 5. Default value
     * 6. Either a test default value or a min and max value
     * 
     * Example lines:
     * BOOLEAN opt1 compiler user true
     * or
     * BOOLEAN opt1 compiler user true false
     * or 
     * INTEGER opt1 compiler user 1 0 100
     */
    private OptionDeclaration parseDeclaration(String optionsFile, String[] parts) throws OptionsAggregationException {
        String kind = parseKind(parts[0]);
        String name = parts[1];
        String category = parts[2];
        String type = parts[3];
        String defaultValue = parts[4];
        OptionDeclaration res = new OptionDeclaration(optionsFile, kind, name, defaultValue, category, type);
        if (parts.length > 6) {
            res.setArgs(Arrays.copyOfRange(parts, 5, parts.length));
        } else if (parts.length > 5) {
            res.setTestDefaultValue(parts[5]);
        }
        return res;
    }
    
    /**
     * If the next line is not empty, parse a set of possible values for the option. Whitespace and newline sensitive.
     * Example line:
     * "str1" "str2"
     * or
     * 0 1 2 5 7
     */
    private void parsePossibleValues(OptionDeclaration res, BufferedReader reader) throws IOException {
        String line = reader.readLine();
        if (!isEmpty(line)) {
            res.setPossibleValues(line.split(" "));
        }
    }
    
    /**
     * Parse an option description. The description should start and end with
     * quotes. Will eat lines until there is an empty one. Newlines will be
     * discarded. Example lines: 
     * "This is a
     * multiline
     * description"
     * or
     * "This is a single line description"
     */
    private void parseDescription(OptionDeclaration res, BufferedReader reader) throws IOException {
        StringBuilder description = new StringBuilder();
        String line = nextLine(reader);
        do {
            description.append(line);
        } while ((line = reader.readLine()) != null && !isEmpty(line));

        res.setDescription(description.toString());
    }
    
    private void addDeclaration(OptionDeclaration res) throws OptionsAggregationException {
        Option old = options.get(res.getName());
        if (old != null) {
            throw new OptionsAggregationException(
                    "Found duplicated option declaration for " + res.getName() + ". Old declaration from "
                            + old.getFilePath() + ". New declaration from " + res.getFilePath());
        }
        options.put(res.getName(), res);
    }
    
    /**
     * Parse a full declaration including possible values and description. Example:
     * STRING opt1 compiler user "default"
     * "default" "otherValue"
     * 
     * "A very nice option"
     * 
     */
    private void parseFullDeclaration(String optionsFile, BufferedReader reader, String[] parts) throws OptionsAggregationException, IOException {
        OptionDeclaration res = parseDeclaration(optionsFile, parts);
        parsePossibleValues(res, reader);
        parseDescription(res, reader);
        addDeclaration(res);
    }
    
    private boolean parseNextOption(String optionsFile, BufferedReader reader) throws IOException, OptionsAggregationException {
        String line = nextLine(reader);
        if (line == null) {
            return false;
        }
        String[] parts = line.split(" ");
        if (parts.length > 7) {
            throw new OptionsAggregationException("Too many parts on the line! " + optionsFile + "\n" + line);
        }

        if (parseModification(optionsFile, parts)) {
            return true;
        } else {
            if (parts.length < 5) {
                throw new OptionsAggregationException("Too few parts on the line! " + optionsFile + "\n" + line);
            }
            parseFullDeclaration(optionsFile, reader, parts);
        }

        return true;
    }

    public void parseFile(String file, BufferedReader reader) throws IOException, OptionsAggregationException {
        while (parseNextOption(file, reader)) {
            
        }
    }
    
    public void parseFile(File optionsFile) throws IOException, OptionsAggregationException {
        BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(optionsFile), "UTF8"));
        parseFile(optionsFile.getAbsolutePath(), reader);
        reader.close();
    }

    public void parseFiles(String modules) throws IOException, OptionsAggregationException {
        for (String module : modules.split(",")) {
            module = module.trim();
            module = module.substring(1, module.length() - 1);
            File moduleFile = new File(module);
            if (moduleFile.exists()) {
                for (File in : moduleFile.listFiles()) {
                    if (in.getName().endsWith(".options")) {
                        parseFile(in);
                    }
                }
            }
        }
    }

    public void modify() throws OptionsAggregationException {
        for (OptionModification mod : optionsModification) {
            mod.modify();
        }
    }

    public OptionsAggregator() {
        
    }

    public OptionsAggregator(String modules) throws IOException, OptionsAggregationException {
        parseFiles(modules);
        modify();
    }

    public static void main(String[] args) throws IOException, OptionsAggregationException {
        File outDir = new File(args[0]);
        String pack = args[1];
        String modules = args[3];

        OptionsAggregator op = new OptionsAggregator(modules);
        File outFile = new File(outDir, "OptionsAggregated.java");
        OutputStreamWriter out = new OutputStreamWriter(new FileOutputStream(outFile));
        op.generate(out, pack);
        out.close();
        System.out.println("Generated " + outFile.getAbsolutePath() + "...");
    }

    public Iterable<OptionDeclaration> getOptions() {
        ArrayList<OptionDeclaration> res = new ArrayList<>();
        for (OptionDeclaration opt : options.values()) {
            if (!opt.getRemoved()) {
                res.add(opt);
            }
        }
        return res;
    }
}
