import org.graalvm.nativeimage.c.function.CEntryPoint;
import org.graalvm.nativeimage.IsolateThread;
import org.graalvm.nativeimage.c.type.CCharPointer;
import org.graalvm.nativeimage.c.type.CTypeConversion;

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

// https://github.com/cliftonlabs/json-simple
import com.github.cliftonlabs.json_simple.JsonObject;
import com.github.cliftonlabs.json_simple.JsonArray;
import com.github.cliftonlabs.json_simple.Jsoner;


interface JModelica {
  static void main(String[] args) { 
  }

  @CEntryPoint(name = "jmodelica_compile")
  static void compile(IsolateThread thread, CCharPointer args) { 
    try {
      String argsString = CTypeConversion.toJavaString(args);

      JsonObject deserialized = (JsonObject)Jsoner.deserialize(argsString);
      String model = (String)deserialized.get("model");
      String outputDir = (String)deserialized.get("outputDir");
      String mslDir = (String)deserialized.get("mslDir");
      String[] modelicaPaths = ((JsonArray)deserialized.get("modelicaPaths")).toArray(new String[0]);

      OptionRegistry options = ModelicaCompiler.createOptions();
      options.setStringOption("fmu_type", "FMUCS20");
      ModelicaCompiler mc = new ModelicaCompiler(options);
      SpawnCompilerDelegator.register();
      mc.setDebugSrcIsHome(true);
      mc.setOutDir(new File(outputDir));
      mc.setLogger("d:" + outputDir + "/out.log");
      mc.setModelicapath(mslDir);
      ModelicaCompiler.TargetObject to = mc.createTargetObject("cs", "2.0");

      System.out.println("Compiling Model with JModelica");
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
      System.out.println("Trouble during JModelica Compiling");
      System.out.println(e.getMessage());
      System.out.println("****");
    }
  }
}

