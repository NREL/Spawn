
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
  protected String getMake(String platform)
  {
    try {
      String myPath = new File(SpawnCompilerDelegator.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getPath();
      return myPath;
    } catch (java.lang.Exception ex) {
      return "make";
    }
  }

  public SpawnCompilerDelegator(File jmHome, String buildPlatform) {
    super(jmHome);
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
    } catch (NoSuchFieldException | IllegalAccessException e) {
      e.printStackTrace();
    }
  }
}


