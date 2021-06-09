
import java.lang.reflect.Field;
import java.util.Map;
import java.io.File;

import org.jmodelica.util.ccompiler.*;
import org.jmodelica.util.EnvironmentUtils;
import org.jmodelica.util.exceptions.CcodeCompilationException;
import org.jmodelica.util.logging.ModelicaLogger;
import org.jmodelica.util.streams.NullStream;


public class SpawnCompilerDelegator extends GccCompilerDelegator {

  @Override
  protected void compileCCode(ModelicaLogger log, CCompilerArguments args, File workDir, String[] platforms) {
    System.out.println("Calling SpawnCompilerDelegator::compile()");

    try {
      System.out.println("Executing from: " + new File(SpawnCompilerDelegator.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getPath());
    } catch (java.lang.Exception ex) {
    }

    System.out.println("Makefile: " + getMakefile().getPath());

    super.compileCCode(log, args, workDir, platforms);

    System.out.println("Compiled: " + args.getFileName());
  }

  @Override
  protected String getMake(String platform)
  {
    System.out.println("getMake called");
    try {
      String myPath = new File(SpawnCompilerDelegator.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getPath();
      System.out.println("Calling '" + myPath + "'");
      //System.out.println("Delegating to real make for now");
      //return super.getMake(platform);
      return myPath;
    } catch (java.lang.Exception ex) {
      System.out.println("Unable to determine my executable location? What to do?");
      return "make";
    }
  }

  public SpawnCompilerDelegator(File jmHome, String buildPlatform) {
    super(jmHome, buildPlatform);
  }


  static public void register()
  {
    // The CCompilerDelegator does not provide any way for us to replace a delegator. If we try to
    // then it will throw an exception because one already exists for that name
    // and the registry is private, so we cannot do what we want

    try {
      // Find private field
      Field creators = CCompilerDelegator.class.getDeclaredField("creators");

      // Make it public
      creators.setAccessible(true);

      // get it (null ptr to object because it is a static field)
      Map<String, CCompilerDelegator.Creator> creatorMap = (Map<String, CCompilerDelegator.Creator>)creators.get(null);

      // Add my own compiler delegator. Could replace many with the same code
      creatorMap.replace("gcc", new SpawnCompilerDelegator.Creator() {
        @Override
        public SpawnCompilerDelegator create() {
          return new SpawnCompilerDelegator(EnvironmentUtils.getJModelicaHome(), EnvironmentUtils.getJavaPlatform());
        }
      });
      System.out.println("Registered SpawnCompilerDelegator in place of GccCompilerDelegator");
    } catch (NoSuchFieldException | IllegalAccessException e) {
      e.printStackTrace();
    }
  }
}


