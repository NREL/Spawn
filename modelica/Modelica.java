import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.jmodelica.modelica.compiler.ModelicaCompiler;
import org.jmodelica.modelica.compiler.SourceRoot;
import org.jmodelica.modelica.compiler.InstClassDecl;
import org.jmodelica.modelica.compiler.FClass;
import org.jmodelica.modelica.compiler.generated.OptionRegistry;
import org.jmodelica.common.options.AbstractOptionRegistry;
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
    // ie /path/to/JModelica/ThirdParty/MSL/
    String mslPath = args[0];
    // ie /path/to/modelica-buildings/Buildings/
    String[] modelpaths = new String[args.length - 3];
    System.arraycopy(args, 1, modelpaths, 0, args.length - 3);
    // ie "Buildings.Examples.Tutorial.Boiler.System3"
    String modelid = args[args.length - 2];
    String output_dir = args[args.length - 1];


    OptionRegistry options = ModelicaCompiler.createOptions();
    // Why do we need this in addition to the arguments passed to createTargetObject ?
    // All I know is that without it the C code generator does not have option "fmu_type"
    // that it is dependent on
    options.setStringOption("fmu_type", "FMUME20");

    ModelicaCompiler mc = new ModelicaCompiler(options);

    SpawnCompilerDelegator.register();

    mc.setDebugSrcIsHome(true);
    mc.setOutDir(new File(output_dir));
    mc.setLogger("d:" + output_dir + "/out.log");
    mc.setModelicapath(mslPath);
    ModelicaCompiler.TargetObject to = mc.createTargetObject("me", "2.0");

    try {
      System.out.println("Parse Model");
      SourceRoot sr = mc.parseModel(modelpaths);
      System.out.println("Instantiate Model");
      InstClassDecl mo = mc.instantiateModel(sr,modelid,to);
      System.out.println("Flatten Model");
      FClass flatMO = mc.flattenModel(mo,to,modelid);
      System.out.println("Generate C Code");
      mc.generateCode(flatMO,to);
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

