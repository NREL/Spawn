import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

import org.jmodelica.optimica.compiler.OptimicaCompiler;
import org.jmodelica.optimica.compiler.generated.OptionRegistry;
import org.jmodelica.optimica.compiler.SourceRoot;
import org.jmodelica.optimica.compiler.InstClassDecl;
import org.jmodelica.optimica.compiler.FClass;
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

// https://github.com/cliftonlabs/json-simple
import com.github.cliftonlabs.json_simple.JsonObject;
import com.github.cliftonlabs.json_simple.JsonArray;
import com.github.cliftonlabs.json_simple.Jsoner;

interface Optimica {

  // This function is only used to generate reflection information at build time
  // The build system will compile a simple model using this interface
  static void main(String[] args) { 
    String jsonOptions = args[0];
    compile(jsonOptions);
  }

  // This is the normal entry point for using the embedded Optimica compiler
  @CEntryPoint(name = "optimica_compile")
  static void compile(IsolateThread thread, CCharPointer args) { 
    String options = CTypeConversion.toJavaString(args);
    compile(options);
  }

  static void compile(String jsonOptions) { 
    try {
      JsonObject deserialized = (JsonObject)Jsoner.deserialize(jsonOptions);
      String model = (String)deserialized.get("model");
      String outputDir = (String)deserialized.get("outputDir");
      String mslDir = (String)deserialized.get("mslDir");
      String fmuType = (String)deserialized.get("fmuType"); // options are "ME" or "CS"
      String[] modelicaPaths = ((JsonArray)deserialized.get("modelicaPaths")).toArray(new String[0]);

      OptionRegistry options = OptimicaCompiler.createOptions();
      if (fmuType.matches("ME")) {
        options.setStringOption("fmu_type", "FMUME20");
        options.setBooleanOption("generate_ode", true);
        options.setBooleanOption("generate_dae", false);
        options.setBooleanOption("equation_sorting", true);
        options.setBooleanOption("generate_fmi_me_xml", true);
        options.setBooleanOption("generate_fmi_cs_xml", false);
        options.setBooleanOption("generate_xml_equations", false);
      } else {
        options.setStringOption("fmu_type", "FMUCS20");
        options.setBooleanOption("generate_ode", true);
        options.setBooleanOption("generate_dae", false);
        options.setBooleanOption("equation_sorting", true);
        options.setBooleanOption("generate_fmi_me_xml", false);
        options.setBooleanOption("generate_fmi_cs_xml", true);
        options.setBooleanOption("generate_xml_equations", false);
      }

      OptimicaCompiler mc = new OptimicaCompiler(options);
      SpawnCompilerDelegator.register();
      mc.setDebugSrcIsHome(true);
      mc.setOutDir(new File(outputDir));
      mc.setLogger("d:" + outputDir + "/out.log");
      mc.setModelicapath(mslDir);
      OptimicaCompiler.TargetObject to;
      if (fmuType.matches("ME")) {
        to = mc.createTargetObject("me", "2.0");
      } else {
        to = mc.createTargetObject("cs", "2.0");
      }

      System.out.println("Compiling Model with Optimica");
      System.out.println("Parse Model");
      SourceRoot sr = mc.parseModel(modelicaPaths);
      System.out.println("Instantiate Model");
      InstClassDecl mo = mc.instantiateModel(sr,model,to);
      System.out.println("Flatten Model");
      FClass flatMO = mc.flattenModel(mo,to,model);
      System.out.println("Load Resources");
      flatMO.loadResources(new File(outputDir, "resources"));
      System.out.println("Generate C Code");
      mc.generateCode(flatMO,to);
    } catch (java.lang.Exception e) {
      System.out.println("Trouble during Optimica Compiling");
      System.out.println(e.getMessage());
      System.out.println("****");
    }
  }
}

