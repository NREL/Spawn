#include "../compiler/compiler.hpp"
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <vector>
#include <iterator>
#include <boost/filesystem.hpp>
#include <pugixml.hpp>
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include <config.hxx>
#include <FMI/Variables.hpp>
#include <modelica.h>
#include <stdlib.h>  
#include "pugixml.hpp"

#if defined _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

using json = nlohmann::json;

boost::filesystem::path exeDir() {
  #ifdef __APPLE__
    Dl_info info;
    dladdr("main", &info);
    return boost::filesystem::canonical(boost::filesystem::path(info.dli_fname).parent_path())
  #elif _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    return boost::filesystem::canonical(boost::filesystem::path(szPath).parent_path());
  #else
    Dl_info info;
    dladdr("main", &info);
    return boost::filesystem::canonical(boost::filesystem::path(info.dli_fname).parent_path());
  #endif
}

bool isInstalled() {
  return exeDir().stem() == "bin";
}

int compileMO(const std::string &moinput) {
  try {
    std::vector<boost::filesystem::path> paths;
    if (isInstalled()) {
      auto jmodelica_home = exeDir() / "../JModelica/";
      setenv("JMODELICA_HOME", jmodelica_home.string().c_str(), 1);
    } else {
      boost::filesystem::path binary_dir(spawn::BINARY_DIR);
      auto jmodelica_home = binary_dir / "modelica/JModelica-prefix/src/JModelica/";
      setenv("JMODELICA_HOME", jmodelica_home.string().c_str(), 1);
      paths.push_back(binary_dir / "modelica/JModelica-prefix/src/JModelica/ThirdParty/MSL/");
      paths.push_back(binary_dir / "modelica-buildings/Buildings/");
    }
    std::vector<std::string> params{"modelica", moinput};
    std::transform(std::begin(paths), std::end(paths), std::inserter(params, std::next(std::begin(params))), [](const auto & path) {return path.string();});
    std::vector<const char *> cparams(params.size());
    std::transform(std::begin(params), std::end(params), std::begin(cparams), [](const auto & p) {return p.c_str();});

    run_main(cparams.size(), (char **)cparams.data());
    return 0;
  } catch(...) {
    return 1;
  }
}

int compileC() {
	boost::filesystem::path output_dir("mo-build");
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
  std::vector<boost::filesystem::path> runtime_libs; 
  if(isInstalled()) {
    // TODO this
  } else {
    boost::filesystem::path binary_dir(spawn::BINARY_DIR);
		const auto & jmodelica_dir = binary_dir / "modelica/JModelica-prefix/src/JModelica";
    runtime_libs = {
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

  // include paths
  std::vector<boost::filesystem::path> include_paths;
  if (isInstalled()) {
		// TODO this
	} else {
    boost::filesystem::path binary_dir(spawn::BINARY_DIR);
		const auto & jmodelica_dir = binary_dir / "modelica/JModelica-prefix/src/JModelica";
  	include_paths = std::vector<boost::filesystem::path>{
      jmodelica_dir / "include/RuntimeLibrary/",
      jmodelica_dir / "include/RuntimeLibrary/module_include",
      jmodelica_dir / "include/RuntimeLibrary/zlib",
      jmodelica_dir / "ThirdParty/FMI/2.0"
  	};
	}

  const auto model_description_path = output_dir / "modelDescription.xml";

  pugi::xml_document doc;
  pugi::xml_parse_result result = doc.load_file(model_description_path.native().c_str());
  if(! result) { return 1;}
  std::string model_identifier = doc.child("fmiModelDescription").child("ModelExchange").attribute("modelIdentifier").as_string();

  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  std::for_each(begin(sourcefiles), end(sourcefiles), [&](const auto &path) { compiler.compile_and_link(path); });

  const auto model_lib_path = output_dir / "binaries" / (model_identifier + ".so");
	compiler.write_shared_object_file(model_lib_path, runtime_libs);

  return 0;
}

int compileModel(const std::string &moinput) {
  int result = compileMO(moinput);
  if(result == 0)
  {
    result = compileC();
  }
  return result;
}

int createFMU(const std::string &jsoninput) {
  json j;

  std::ifstream fileinput(jsoninput);
  if (!fileinput.fail()) {
    // deserialize from file
    fileinput >> j;
  } else {
    // Try to parse command line input as json string
    j = json::parse(jsoninput, nullptr, false);
  }

  json idf;
  json idd;
  json weather;
  json zones;
  json fmuname;

  if (j.is_discarded()) {
    std::cout << "Cannot parse json: '" << jsoninput << "'" << std::endl;
  } else {
    try {
      idf = j.at("EnergyPlus").at("idf");
      idd = j.at("EnergyPlus").at("idd");
      weather = j.at("EnergyPlus").at("weather");
      zones = j.at("model").at("zones");
      fmuname = j.at("fmu").at("name");
    } catch (...) {
      std::cout << "Invalid json input: '" << jsoninput << "'" << std::endl;
      return 1;
    }
  }

  // Input paths
  auto basepath = boost::filesystem::canonical(boost::filesystem::path(jsoninput).parent_path());
  auto fmupath = boost::filesystem::path(fmuname.get<std::string>());
  if (! fmupath.is_absolute()) {
    fmupath = basepath / fmupath;
  }
  auto idfInputPath = boost::filesystem::path(idf.get<std::string>());
  if (! idfInputPath.is_absolute()) {
    idfInputPath = basepath / idfInputPath;
  }
  auto epwInputPath = boost::filesystem::path(weather.get<std::string>());
  if (! epwInputPath.is_absolute()) {
    epwInputPath = basepath / epwInputPath;
  }
  auto iddInputPath = boost::filesystem::path(idd.get<std::string>());
  if (! iddInputPath.is_absolute()) {
    iddInputPath = basepath / iddInputPath;
  }

  // Do the input paths exist?
  bool missingFile = false;
  if (! boost::filesystem::exists(idfInputPath)) {
    std::cout << "The specified idf input file does not exist, " << idfInputPath << "." << std::endl;
    missingFile = true;
  }
  if (! boost::filesystem::exists(epwInputPath)) {
    std::cout << "The specified epw input file does not exist, " << epwInputPath << "." << std::endl;
    missingFile = true;
  }
  if (! boost::filesystem::exists(iddInputPath)) {
    std::cout << "The specified idd input file does not exist, " << iddInputPath << "." << std::endl;
    missingFile = true;
  }

  if (missingFile) {
    return 1;
  }

  // Output paths
  auto fmuStaggingPath = fmupath.parent_path() / fmupath.stem();
  auto modelDescriptionPath = fmuStaggingPath / "modelDescription.xml";
  auto resourcesPath = fmuStaggingPath / "resources";
  auto idfPath = resourcesPath / idfInputPath.filename();
  auto epwPath = resourcesPath / epwInputPath.filename();
  auto iddPath = resourcesPath / iddInputPath.filename();

  boost::filesystem::path epFMIDestPath;
  boost::filesystem::path epFMISourcePath;

  boost::filesystem::remove(fmupath);

  #ifdef __APPLE__
    auto exedir = exeDir();
    epFMISourcePath = exedir / "../lib/libepfmi.dylib";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.dylib";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/darwin64/libepfmi.dylib";
  #elif _WIN32
    auto exedir = exeDir();
    epFMISourcePath = exedir / "epfmi.dll";
    epFMIDestPath = fmuStaggingPath / "binaries/win64/epfmi.dll";
  #else
    auto exedir = exeDir();
    epFMISourcePath = exedir / "../lib/libepfmi.so";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.so";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/linux64/libepfmi.so";
  #endif

  boost::filesystem::create_directories(fmuStaggingPath);
  boost::filesystem::create_directories(resourcesPath);
  boost::filesystem::create_directories(epFMIDestPath.parent_path());

  boost::filesystem::copy_file(epFMISourcePath, epFMIDestPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(idfInputPath, idfPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(iddInputPath, iddPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(epwInputPath, epwPath, boost::filesystem::copy_option::overwrite_if_exists);

  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto xmlvariables = doc.child("fmiModelDescription").child("ModelVariables");

  const auto epvariables = EnergyPlus::FMI::parseVariables(idfPath.string());

  for (const auto & varpair : epvariables) {
    const auto valueReference = varpair.first;
    const auto var = varpair.second;

    auto scalarVar = xmlvariables.append_child("ScalarVariable");
		for (const auto & attribute : var.scalar_attributes) {
    	scalarVar.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
		}

    auto real = scalarVar.append_child("Real");
		for (const auto & attribute : var.real_attributes) {
    	real.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
		}
  }

  doc.save_file(modelDescriptionPath.c_str());

  zip_directory(fmuStaggingPath.string(), fmupath.string());
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  std::string moinput = "";
  auto createOption =
      app.add_option("-c,--compile", moinput,
                     "Compile Modelica model to FMU format", true);

  std::string jsoninput = "spawn.json";
  auto exportOption =
      app.add_option("-e,--export", jsoninput,
                     "Export a standalone EnergyPlus based FMU", true);

  auto versionOption =
    app.add_flag("-v,--version", "Print version info and exit");

  CLI11_PARSE(app, argc, argv);

  if (*createOption) {
    return compileModel(moinput);
  }

  if (*exportOption) {
    return createFMU(jsoninput);
  }

  if (*versionOption) {
    std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
  }

  return 0;
}

