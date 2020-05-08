#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
//#include "../lib/iddtypes.hpp"
//#include "../lib/input.hpp"
//#include "../lib/outputtypes.hpp"
//#include "../lib/variables.hpp"
//#include "../util/idf_to_json.hpp"
//#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
//#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
//#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../compiler/compiler.hpp"
#include "cli/embedded_files.hxx"
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <boost/filesystem.hpp>
#include <pugixml.hpp>
#include <config.hxx>
#include <boost/algorithm/string.hpp>
#include <modelica.h>
#include <stdlib.h>

#if defined _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

using json = nlohmann::json;

json & adjustSimulationControl(json & jsonidf) {
  constexpr auto simulationcontroltype = "SimulationControl";

  // Remove the existing control first
  jsonidf.erase(simulationcontroltype);

  // This is what we need for spawn
  jsonidf[simulationcontroltype] = {
    {
      "Spawn-SimulationControl", {
        {"do_plant_sizing_calculation", "Yes"},
        {"do_system_sizing_calculation","Yes"},
        {"do_zone_sizing_calculation", "Yes"},
        {"run_simulation_for_sizing_periods", "No"},
        {"run_simulation_for_weather_file_run_periods", "Yes"}
      }
    }
  };
}

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

int compileMO(const std::string &moinput, const boost::filesystem::path & output_dir) {
  try {
    std::vector<boost::filesystem::path> paths;
    boost::filesystem::path jmodelica_home;
    boost::filesystem::path mbl_path;
    if (isInstalled()) {
      jmodelica_home = exeDir() / "../JModelica/";
      mbl_path = exeDir() / "../modelica-buildings/Buildings/";
    } else {
      boost::filesystem::path binary_dir(spawn::BINARY_DIR);
      jmodelica_home = binary_dir / "modelica/JModelica-prefix/src/JModelica/";
      mbl_path = binary_dir / "modelica-buildings/Buildings/";
    }

    setenv("JMODELICA_HOME", jmodelica_home.string().c_str(), 1);
    const auto msl_path = jmodelica_home / "ThirdParty/MSL/";

    paths.push_back(msl_path);
    paths.push_back(mbl_path);

    std::vector<std::string> params{"modelica", moinput, output_dir.native()};
    std::transform(std::begin(paths), std::end(paths), std::inserter(params, std::next(std::begin(params))), [](const auto & path) {return path.string();});
    std::vector<const char *> cparams(params.size());
    std::transform(std::begin(params), std::end(params), std::begin(cparams), [](const auto & p) {return p.c_str();});

    run_main(cparams.size(), (char **)cparams.data());
    return 0;
  } catch(...) {
    return 1;
  }
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

  const auto & temp_dir = output_dir / "tmp";

  std::vector<boost::filesystem::path> runtime_libs;
  const auto & jmodelica_dir = temp_dir / "JModelica";
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

  // include paths
  std::vector<boost::filesystem::path> include_paths;
  include_paths = std::vector<boost::filesystem::path>{
    jmodelica_dir / "include/RuntimeLibrary/",
    jmodelica_dir / "include/RuntimeLibrary/module_include",
    jmodelica_dir / "include/RuntimeLibrary/zlib",
    jmodelica_dir / "ThirdParty/FMI/2.0"
  };

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

int compileModel(const std::string &moinput) {
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

  int result = compileMO(moinput, output_dir.native());
  if(result == 0) {
    result = compileC(output_dir);
  }

  if(result == 0) {
    boost::filesystem::remove_all(temp_dir);
    std::cout << "Model Compiled" << std::endl;
  }

  return result;
}

json & addRunPeriod(json & jsonidf) {
  constexpr auto runperiodtype = "RunPeriod";
  // Remove the existing run periods first
  jsonidf.erase(runperiodtype);

  // Add a new run period just for spawn
  // 200 years should be plenty
  jsonidf[runperiodtype] = {
    {
      "Spawn-RunPeriod", {
        {"apply_weekend_holiday_rule", "No"},
        {"begin_day_of_month", 1},
        {"begin_month", 1},
        {"begin_year", 2017},
        {"day_of_week_for_start_day", "Sunday"},
        {"end_day_of_month", 31},
        {"end_month", 12},
        {"end_year", 2217},
        {"use_weather_file_daylight_saving_period", "No"},
        {"use_weather_file_holidays_and_special_days", "No"},
        {"use_weather_file_rain_indicators", "Yes"},
        {"use_weather_file_snow_indicators", "Yes"}
      }
    }
  };

  return jsonidf;
}

//// Remove objects related to HVAC and controls
//json & removeUnusedObjects(json & jsonidf) {
//  for(auto typep = jsonidf.cbegin(); typep != jsonidf.cend();){
//    if(std::find(std::begin(supportedIDDTypes), std::end(supportedIDDTypes), typep.key()) == std::end(supportedIDDTypes)) {
//      typep = jsonidf.erase(typep);
//    } else {
//      ++typep;
//    }
//  }
//
//  // Remove unsupported output vars
//  auto & outputvars = jsonidf.at("Output:Variable");
//  for(auto var = outputvars.cbegin(); var != outputvars.cend();){
//    const auto & name = var.value().at("variable_name").get<std::string>();
//    const auto & findit = std::find_if(std::begin(outputtypes), std::end(outputtypes),
//      [&](const std::pair<const char *, OutputProperties> & v) {
//        return boost::iequals(v.first,name);
//      });
//
//    if(findit == std::end(outputtypes)) {
//      var = outputvars.erase(var);
//    } else {
//      ++var;
//    }
//  }
//
//  return jsonidf;
//}
//
//// Add output variables requested in the spawn input file, but not in the idf
//json & addOutputVariables(json & jsonidf, const std::vector<spawn::OutputVariable> & requestedvars) {
//  // A pair that holds an output variable name and key,
//  typedef std::pair<std::string, std::string> Varpair;
//
//  // Make a list of the requested outputs
//  std::vector<Varpair> requestedpairs;
//  for(const auto & var : requestedvars) {
//    requestedpairs.emplace_back(var.idfname, var.idfkey);
//  }
//
//  // And a list of the current output variables
//  auto & currentvars = jsonidf["Output:Variable"];
//  std::vector<Varpair> currentpairs;
//  for(const auto & var : currentvars) {
//    currentpairs.emplace_back(var.at("variable_name").get<std::string>(), var.at("key_value").get<std::string>());
//  }
//
//  // Identify any missing pairs. ie. those that are requested but not in the idf
//  std::vector<Varpair> missingpairs;
//  std::sort(requestedpairs.begin(), requestedpairs.end());
//  std::sort(currentpairs.begin(), currentpairs.end());
//
//  std::set_difference(requestedpairs.begin(), requestedpairs.end(),
//      currentpairs.begin(), currentpairs.end(),
//      std::back_inserter(missingpairs));
//
//  for( const auto & pair : missingpairs) {
//    json newvar;
//    newvar["variable_name"] = pair.first;
//    newvar["key_value"] = pair.second;
//    newvar["reporting_frequency"] = "Timestep";
//    currentvars[pair.first + pair.second] = newvar;
//  }
//
//  return jsonidf;
//}

boost::filesystem::path exedir() {
  #if _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    return boost::filesystem::path(szPath).parent_path();
  #else
    Dl_info info;
    dladdr("main", &info);
    return boost::filesystem::path(info.dli_fname).parent_path();
  #endif
}

boost::filesystem::path iddInstallPath() {
  constexpr auto & iddfilename = "Energy+.idd";
  // Configuration in install tree
  auto iddInputPath = exedir() / "../etc" / iddfilename;

  // Configuration in a developer tree
  if (! boost::filesystem::exists(iddInputPath)) {
    iddInputPath = exedir() / iddfilename;
  }

  return iddInputPath;
}

std::string epfmiName() {
  // Configure this using cmake
  #ifdef __APPLE__
    return "libepfmi.dylib";
  #elif _WIN32
    return "epfmi.dll";
  #else
    return "libepfmi.so";
  #endif
}

std::string fmiplatform() {
  #ifdef __APPLE__
    return "darwin64";
  #elif _WIN32
    return "win64";
  #else
    return "linux64";
  #endif
}

boost::filesystem::path epfmiInstallPath() {
  const auto candidate = exedir() / ("../lib/" + epfmiName());
  if (boost::filesystem::exists(candidate)) {
    return candidate;
  } else {
    return exedir() / epfmiName();
  }
}

void createModelDescription(const spawn::Input & input, const boost::filesystem::path & savepath) {
  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto xmlvariables = doc.child("fmiModelDescription").child("ModelVariables");

  const auto variables = parseVariables(input);

  for (const auto & varpair : variables) {
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

  doc.save_file(savepath.c_str());
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  bool nozip = false;
  bool nocompress = false;

  std::string jsoninput = "spawn.json";
  auto createOption =
      app.add_option("-c,--create", jsoninput,
                     "Create a standalone FMU based on json input", true);

  bool nozip = false;
  auto zipOption = app.add_flag("--no-zip", nozip, "Stage FMU files on disk without creating a zip archive");
  zipOption->needs(createOption);

  auto compressOption = app.add_flag("--no-compress", nocompress, "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead.");
  compressOption->needs(createOption);

  auto versionOption =
    app.add_flag("-v,--version", "Print version info and exit");

  std::string moinput = "";
  auto compileOption =
      app.add_option("--compile", moinput,
                     "Compile Modelica model to FMU format", true);

  CLI11_PARSE(app, argc, argv);
    //return compileModel(moinput);

  if (*createOption) {
    auto result = createFMU(jsoninput, nozip, nocompress);
    if (result) {
      return result;
    }
  } else if (*compileOption) {
    auto result = compileOption(moinput);
    if (result) {
      return result;
    }
  } else if (*versionOption) {
    std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
  }

  return 0;
}

