import org.graalvm.nativeimage.c.function.CEntryPoint;
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
  //@CEntryPoint
  //public static void foo() {
  //}

  public static void main(String[] args) { 
    OptionRegistry options = ModelicaCompiler.createOptions();
    // Why do we need this in addition to the arguments passed to createTargetObject ?
    // All I know is that without it the C code generator does not have option "fmu_type"
    // that it is dependent on
    options.setStringOption("fmu_type", "FMUME20");

    ModelicaCompiler mc = new ModelicaCompiler(options);
    mc.setDebugSrcIsHome(true);
    mc.setOutDir(new File("mo-build/"));
    mc.setLogger("d:mo-build/out.log");
    // arg 0 is the MODELICAPATH
    mc.setModelicapath(args[0]);
    ModelicaCompiler.TargetObject to = mc.createTargetObject("me", "2.0");
    // arg 1 is the path to the files to compile
		String[] files = new String[1];
    files[0] = args[1];

		try {
    	System.out.println("Parse Model");
			SourceRoot sr = mc.parseModel(files);
    	System.out.println("Instantiate Model");
		  InstClassDecl mo = mc.instantiateModel(sr,"Buildings.Examples.Tutorial.Boiler.System3",to);
    	System.out.println("Flatten Model");
			FClass flatMO = mc.flattenModel(mo,to,"Buildings.Examples.Tutorial.Boiler.System3");
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
    	System.out.println(e.getMessage());
		} catch (java.lang.Exception e) {
    	System.out.println("Trouble during Modelica Compiling");
    	System.out.println(e.getMessage());
    	System.out.println("****");
		}
  } 
}

