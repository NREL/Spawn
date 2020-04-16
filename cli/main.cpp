#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include "../lib/iddtypes.hpp"
#include "../lib/input.hpp"
#include "../lib/outputtypes.hpp"
#include "../lib/variables.hpp"
#include "../util/idf_to_json.hpp"
#include <CLI/CLI.hpp>
#include <third_party/nlohmann/json.hpp>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <pugixml.hpp>
#include <config.hxx>
#include <boost/algorithm/string.hpp>

#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"

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

  return jsonidf;
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

// Remove objects related to HVAC and controls
json & removeUnusedObjects(json & jsonidf) {
  for(auto typep = jsonidf.cbegin(); typep != jsonidf.cend();){
    if(std::find(std::begin(supportedIDDTypes), std::end(supportedIDDTypes), typep.key()) == std::end(supportedIDDTypes)) {
      typep = jsonidf.erase(typep);
    } else {
      ++typep;
    }
  }

  // Remove unsupported output vars
  auto & outputvars = jsonidf.at("Output:Variable");
  for(auto var = outputvars.cbegin(); var != outputvars.cend();){
    const auto & name = var.value().at("variable_name").get<std::string>();
    const auto & findit = std::find_if(std::begin(outputtypes), std::end(outputtypes),
      [&](const std::pair<const char *, OutputProperties> & v) {
        return boost::iequals(v.first,name);
      });

    if(findit == std::end(outputtypes)) {
      var = outputvars.erase(var);
    } else {
      ++var;
    }
  }

  return jsonidf;
}

// Add output variables requested in the spawn input file, but not in the idf
json & addOutputVariables(json & jsonidf, const std::vector<spawn::OutputVariable> & requestedvars) {
  // A pair that holds an output variable name and key,
  typedef std::pair<std::string, std::string> Varpair;

  // Make a list of the requested outputs
  std::vector<Varpair> requestedpairs;
  for(const auto & var : requestedvars) {
    requestedpairs.emplace_back(var.idfname, var.idfkey);
  }

  // And a list of the current output variables
  auto & currentvars = jsonidf["Output:Variable"];
  std::vector<Varpair> currentpairs;
  for(const auto & var : currentvars) {
    currentpairs.emplace_back(var.at("variable_name").get<std::string>(), var.at("key_value").get<std::string>());
  }

  // Identify any missing pairs. ie. those that are requested but not in the idf
  std::vector<Varpair> missingpairs;
  std::sort(requestedpairs.begin(), requestedpairs.end());
  std::sort(currentpairs.begin(), currentpairs.end());

  std::set_difference(requestedpairs.begin(), requestedpairs.end(),
      currentpairs.begin(), currentpairs.end(),
      std::back_inserter(missingpairs));

  for( const auto & pair : missingpairs) {
    json newvar;
    newvar["variable_name"] = pair.first;
    newvar["key_value"] = pair.second;
    newvar["reporting_frequency"] = "Timestep";
    currentvars[pair.first + pair.second] = newvar;
  }

  return jsonidf;
}

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

int createFMU(const std::string &jsoninput, bool nozip, bool nocompress) {
  spawn::Input input(jsoninput);

  // We are going to copy the required files into an FMU staging directory,
  // also copy the json input file into root of the fmu staging directory,
  // and rewrite the json input file paths so that they reflect the new paths
  // contained within the fmu layout.

  // FMU staging paths
  const auto fmuPath = input.basepath() / (input.fmuBaseName() + ".fmu");
  const auto fmuStagingPath = input.basepath() / input.fmuBaseName();
  const auto modelDescriptionPath = fmuStagingPath / "modelDescription.xml";
  const auto fmuResourcesPath = fmuStagingPath / "resources";
  const auto fmuidfPath = fmuResourcesPath / input.idfInputPath().filename();
  const auto fmuepwPath = fmuResourcesPath / input.epwInputPath().filename();
  const auto fmuiddPath = fmuResourcesPath / iddInstallPath().filename();
  const auto fmuspawnPath = fmuStagingPath / "model.spawn";
  const auto fmuEPFMIPath = fmuStagingPath / ("binaries/" + fmiplatform() + "/" + epfmiName());

  boost::filesystem::remove_all(fmuPath);
  boost::filesystem::remove_all(fmuStagingPath);

  // Create fmu staging area and copy files into it
  boost::filesystem::create_directories(fmuStagingPath);
  boost::filesystem::create_directories(fmuResourcesPath);
  boost::filesystem::create_directories(fmuEPFMIPath.parent_path());

  boost::filesystem::copy_file(epfmiInstallPath(), fmuEPFMIPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(iddInstallPath(), fmuiddPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(input.epwInputPath(), fmuepwPath, boost::filesystem::copy_option::overwrite_if_exists);

  auto jsonidf = spawn::idfToJSON(input.idfInputPath());
  adjustSimulationControl(jsonidf);
  addRunPeriod(jsonidf);
  removeUnusedObjects(jsonidf);
  addOutputVariables(jsonidf, input.outputVariables);
  spawn::jsonToIdf(jsonidf, fmuidfPath);

  createModelDescription(input, modelDescriptionPath);

  const auto relativeEPWPath = boost::filesystem::relative(fmuepwPath, fmuStagingPath);
  input.setEPWInputPath(relativeEPWPath);
  const auto relativeIdfPath = boost::filesystem::relative(fmuidfPath, fmuStagingPath);
  input.setIdfInputPath(relativeIdfPath);
  input.save(fmuspawnPath);

  if (! nozip) {
    zip_directory(fmuStagingPath.string(), fmuPath.string(), nocompress);
  }

  return 0;
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  bool nozip = false;
  bool nocompress = false;

  std::string jsoninput = "spawn.json";
  auto createOption =
      app.add_option("-c,--create", jsoninput,
                     "Create a standalone FMU based on json input", true);

  auto zipOption = app.add_flag("--no-zip", nozip, "Stage FMU files on disk without creating a zip archive");
  zipOption->needs(createOption);

  auto compressOption = app.add_flag("--no-compress", nocompress, "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead.");
  compressOption->needs(createOption);

  auto versionOption =
    app.add_flag("-v,--version", "Print version info and exit");

  CLI11_PARSE(app, argc, argv);

  if (*createOption) {
    auto result = createFMU(jsoninput, nozip, nocompress);
    if (result) {
      return result;
    }
  }

  if (*versionOption) {
    std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
  }

  return 0;
}

