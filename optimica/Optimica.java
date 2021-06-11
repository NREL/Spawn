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
  static void main(String[] args) { 
  }

  @CEntryPoint(name = "optimica_compile")
  static void compile(IsolateThread thread, CCharPointer args) { 
    try {
      String argsString = CTypeConversion.toJavaString(args);
      System.out.println("java received data...");
      System.out.println(argsString);

      JsonObject deserialized = (JsonObject)Jsoner.deserialize(argsString);
      String model = (String)deserialized.get("model");
      String outputDir = (String)deserialized.get("outputDir");
      String mslDir = (String)deserialized.get("mslDir");
      String[] modelicaPaths = ((JsonArray)deserialized.get("modelicaPaths")).toArray(new String[0]);

      OptionRegistry options = OptimicaCompiler.createOptions();
      options.setStringOption("fmu_type", "FMUME20");
      OptimicaCompiler mc = new OptimicaCompiler(options);
      SpawnCompilerDelegator.register();
      mc.setDebugSrcIsHome(true);
      mc.setOutDir(new File(outputDir));
      mc.setLogger("d:" + outputDir + "/out.log");
      mc.setModelicapath(mslDir);
      OptimicaCompiler.TargetObject to = mc.createTargetObject("me", "2.0");

      System.out.println("Parse Model");
      SourceRoot sr = mc.parseModel(modelicaPaths);
      System.out.println("Instantiate Model");
      InstClassDecl mo = mc.instantiateModel(sr,model,to);
      System.out.println("Flatten Model");
      FClass flatMO = mc.flattenModel(mo,to,model);
      System.out.println("Generate C Code");
      mc.generateCode(flatMO,to);
    } catch (java.lang.Exception e) {
      System.out.println("Trouble during JModelica Compiling");
      System.out.println(e.getMessage());
      System.out.println("****");
    }
  }
}

