import org.jmodelica.modelica.compiler.ModelicaCompiler;
import org.jmodelica.modelica.compiler.SourceRoot;
import org.jmodelica.modelica.compiler.InstClassDecl;
import org.jmodelica.modelica.compiler.FClass;
import org.jmodelica.common.options.OptionRegistry;
import org.jmodelica.util.exceptions.CompilerException;
import org.jmodelica.util.exceptions.ModelicaClassNotFoundException;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.lang.Exception;
import java.nio.channels.FileChannel;
import java.nio.charset.StandardCharsets;
import java.nio.charset.Charset;
import java.io.File;

public class Modelica { 
  public static void main(String[] args) { 
    OptionRegistry options = ModelicaCompiler.createOptions();
    ////options.setOption("debug_duplicate_generated","true");
    ModelicaCompiler mc = new ModelicaCompiler(options);
    mc.setDebugSrcIsHome(true);
    //mc.setJModelicaHome("/home/kbenne/Development/JModelica/ThirdParty/MSL/Modelica/");
    mc.setOutDir(new File("mo-build/"));
    //mc.setLogger("d:out.log");
    ModelicaCompiler.TargetObject to = mc.createTargetObject("me", "1.0");
		String[] files = new String[1];
    files[0] = args[0];

		try {
    	System.out.println("Parse Model");
			SourceRoot sr = mc.parseModel(files);
    	System.out.println("Instantiate Model");
		  InstClassDecl mo = mc.instantiateModel(sr,"Vdp",to);
    	System.out.println("Flatten Model");
			FClass flatMO = mc.flattenModel(mo,to,"Vdp");
    	System.out.println("Generate Code");
			mc.generateCode(flatMO,to);
    	System.out.println("Done generating code");
		} catch (beaver.Parser.Exception e) {
    	System.out.println("Beaver parser Exception");
		} catch (CompilerException e) {
    	System.out.println("Compiler Exception");
    	System.out.println(e.getMessage());
		} catch (FileNotFoundException e) {
    	System.out.println("File Not found Exception");
		} catch (IOException e) {
    	System.out.println("IO Exception");
		} catch (ModelicaClassNotFoundException e) {
    	System.out.println("Class not found");
		} catch (java.lang.Exception e) {
    	System.out.println("Trouble during Modelica Compiling");
    	System.out.println(e.getMessage());
    	System.out.println("****");
		}
  } 
}

