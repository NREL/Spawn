#include "../compiler/compiler.hpp"
#include "cli/embedded_files.hxx"
#include <pugixml.hpp>
#include <modelica.h>
#include <iostream>

namespace spawn {

int compileMO(
  const std::string & moInput,
  const boost::filesystem::path & outputDir,
  const boost::filesystem::path & mblPath,
  const boost::filesystem::path & jmodelicaHome
) {
  try {
    std::vector<boost::filesystem::path> paths;

    setenv("JMODELICA_HOME", jmodelicaHome.string().c_str(), 1);
    const auto mslPath = jmodelicaHome / "ThirdParty/MSL/";

    paths.push_back(mslPath);
    paths.push_back(mblPath);

    std::vector<std::string> params{"modelica", moInput, outputDir.native()};
    std::transform(std::begin(paths), std::end(paths), std::inserter(params, std::next(std::begin(params))), [](const auto & path) {return path.string();});
    std::vector<const char *> cparams(params.size());
    std::transform(std::begin(params), std::end(params), std::begin(cparams), [](const auto & p) {return p.c_str();});

    run_main(cparams.size(), (char **)cparams.data());
    return 0;
  } catch(...) {
    return 1;
  }
}

std::vector<boost::filesystem::path> modelicaIncludePaths(const boost::filesystem::path &jmodelica_dir)
{
  return {
    jmodelica_dir / "include/RuntimeLibrary/",
    jmodelica_dir / "include/RuntimeLibrary/module_include",
    jmodelica_dir / "include/RuntimeLibrary/zlib",
    jmodelica_dir / "ThirdParty/FMI/2.0"
  };
}

std::vector<boost::filesystem::path> modelicaLibs(const boost::filesystem::path &jmodelica_dir)
{
  return {
    jmodelica_dir / "/lib/RuntimeLibrary/liblapack.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libModelicaIO.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libModelicaExternalC.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libfmi1_cs.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libjmi_get_set_default.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libfmi2.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libblas.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libjmi_block_solver.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libjmi_evaluator_util.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libjmi.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libModelicaStandardTables.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libModelicaMatIO.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libzlib.a",
    jmodelica_dir / "/lib/RuntimeLibrary/libfmi1_me.a",
    jmodelica_dir / "/ThirdParty/Minpack/lib/libcminpack.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_nvecserial.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_idas.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_cvodes.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_ida.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_nvecopenmp.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_arkode.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_cvode.a",
    jmodelica_dir / "/ThirdParty/Sundials/lib/libsundials_kinsol.a"
  };

}

int compileC(const boost::filesystem::path & output_dir) {
  std::cout << "Compile C Code" << std::endl;

  const auto & sourcesdir = output_dir / "sources";
  std::vector<boost::filesystem::path> sourcefiles;

  if( ! boost::filesystem::is_directory(sourcesdir) ) {
    return 1;
  }

  for( const auto & p : boost::filesystem::directory_iterator(sourcesdir) ) {
  	sourcefiles.push_back(p.path());
  }

	// Exclude files that are not C source files
	sourcefiles.erase(
		std::remove_if(begin(sourcefiles), end(sourcefiles), [](const auto & p) {return (p.extension() != ".c");}),
		end(sourcefiles)
	);

	// Exclude ModelicaStrings_skipWhiteSpace.c
  // These symbols seem to be already defined within the runtime libraries
  // Excluding to avoid duplicate symbol error from compiler
	sourcefiles.erase(
		std::remove_if(begin(sourcefiles), end(sourcefiles), [](const auto & p) {return (p.filename().string() == "ModelicaStrings_skipWhiteSpace.c");}),
		end(sourcefiles)
	);

	sourcefiles.erase(
		std::remove_if(begin(sourcefiles), end(sourcefiles), [](const auto & p) {return (p.filename().string() == "ModelicaStrings_compare.c");}),
		end(sourcefiles)
	);

  // src/JModelica/Makefiles/MakeFile
  // Setup link libs and include paths
  // This could be improved, by referring to the MakeFile that JModelica normally uses
  // Refer to JModelica/Makefiles/MakeFile
  //
  // Also need a way to handle external C code that may be referred to and contained within
  // some Modelica models

  // Libs to link

  const auto temp_dir = output_dir / "tmp";

  const auto jmodelica_dir = temp_dir / "JModelica";
  const auto runtime_libs = modelicaLibs(jmodelica_dir);
  // include paths
  const auto include_paths = modelicaIncludePaths(jmodelica_dir) ;

  const auto model_description_path = output_dir / "modelDescription.xml";

  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(model_description_path.native().c_str());
  if(! result) { return 1;}
  std::string model_identifier = doc.child("fmiModelDescription").child("ModelExchange").attribute("modelIdentifier").as_string();

  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  std::for_each(begin(sourcefiles), end(sourcefiles), [&](const auto &path) { compiler.compile_and_link(path); });

  const auto model_lib_dir = output_dir / "binaries";
  const auto model_lib_path = model_lib_dir / (model_identifier + ".so");
  boost::filesystem::create_directories(model_lib_dir);
	compiler.write_shared_object_file(model_lib_path, runtime_libs);

  return 0;
}


void makeModelicaExternalFunction(const std::vector<std::string> &parameters)
{
  for (const auto &param : parameters) {
    std::cout << "makeModelicalExternalFunction '" << param << "'\n";
  }

  if (parameters.size() < 5) {
    std::cout << " unable to determine build target \n";
    return;
  }

  if (parameters[5] == "ceval") {
    std::cout << " ceval: compiling executable from .c files\n";
  } else {
    std::cout << "Unknown build target: '" << parameters[5] << "' aborting\n";
    return;
  }

  std::map<std::string, std::string> arguments;

  for (const auto &param : parameters) {
    const auto equals_pos = param.find('=');

    if (equals_pos != std::string::npos) {
      arguments[param.substr(0, equals_pos)] = param.substr(equals_pos + 1);
    }
  }

  for (const auto &[lhs, rhs] : arguments) {
    std::cout << "arg: '" << lhs << "' = '" << rhs << "'\n";
  }

  boost::filesystem::path fileToCompile = boost::filesystem::path{"sources"} / arguments["FILE_NAME"];
  fileToCompile += ".c";

  const auto jmodelica_dir = boost::filesystem::path{arguments["JMODELICA_HOME"]};
  const auto include_paths = modelicaIncludePaths(jmodelica_dir);
  const auto runtime_libs = modelicaLibs(jmodelica_dir);

  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  compiler.compile_and_link(fileToCompile);
  boost::filesystem::create_directories(boost::filesystem::path{"binaries"});
  compiler.write_executable_file(boost::filesystem::path{"binaries"} / arguments["FILE_NAME"], runtime_libs);
}


int modelicaToFMU(
  const std::string &moinput,
  const boost::filesystem::path & mblPath,
  const boost::filesystem::path & jmodelicaHome
) {
  // output_dir_name is moinput with "." replaced by "_"
  std::string output_dir_name;
  std::transform(begin(moinput), end(moinput), std::back_inserter(output_dir_name),
    [](const auto & c) {
      if(c == '.') {
        return '_';
      } else {
        return c;
      }
    }
  );
	const auto output_dir = boost::filesystem::current_path() / output_dir_name;
  if(! output_dir_name.empty()) {
    boost::filesystem::remove_all(output_dir);
  }

  // tmp is where we extract embedded files
  const auto & temp_dir = output_dir / "tmp";
  boost::filesystem::create_directories(temp_dir);

  for (const auto &file : spawnmodelica::embedded_files::fileNames()) {
    spawnmodelica::embedded_files::extractFile(file, temp_dir.native());
  }

  int result = compileMO(moinput, output_dir.native(), mblPath, jmodelicaHome);
  if(result == 0) {
    result = compileC(output_dir);
  }

  if(result == 0) {
    boost::filesystem::remove_all(temp_dir);
    std::cout << "Model Compiled" << std::endl;
  }

  return result;
}

} // namespace spawn

