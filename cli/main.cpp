#include "iddtypes.hpp"
#include "../lib/outputtypes.hpp"
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include "../lib/variables.hpp"
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
json & addOutputVariables(json & jsonidf, const json & requestedvars) {
  // A pair that holds an output variable name and key,
  typedef std::pair<std::string, std::string> Varpair;

  // Make a list of the requested outputs
  std::vector<Varpair> requestedpairs;
  for(const auto & var : requestedvars) {
    requestedpairs.emplace_back(var.at("name").get<std::string>(), var.at("key").get<std::string>());
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

int createFMU(const std::string &jsoninput, bool nozip, bool nocompress) {
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
  json outputVariables;

  if (j.is_discarded()) {
    std::cout << "Cannot parse json: '" << jsoninput << "'" << std::endl;
  } else {
    try {
      idf = j.at("EnergyPlus").at("idf");
      idd = j.value("idd", "");
      weather = j.at("EnergyPlus").at("weather");
      zones = j.at("model").at("zones");
      fmuname = j.at("fmu").at("name");
      outputVariables = j.value("model",json::object()).value("outputVariables",json::array());
    } catch (...) {
      std::cout << "Invalid json input: '" << jsoninput << "'" << std::endl;
      return 1;
    }
  }

  // Input paths
  auto spawnInputPath = boost::filesystem::canonical(boost::filesystem::path(jsoninput));
  auto basepath = spawnInputPath.parent_path();
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
  boost::filesystem::path iddInputPath;
  if(! idd.get<std::string>().empty()) {
    iddInputPath= boost::filesystem::path(idd.get<std::string>());
    if (! iddInputPath.is_absolute()) {
      iddInputPath = basepath / iddInputPath;
    }
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

  if (missingFile) {
    return 1;
  }

  // Output paths
  constexpr auto iddfilename = "Energy+.idd";
  auto fmuStaggingPath = fmupath.parent_path() / fmupath.stem();
  auto modelDescriptionPath = fmuStaggingPath / "modelDescription.xml";
  auto resourcesPath = fmuStaggingPath / "resources";
  auto idfPath = resourcesPath / idfInputPath.filename();
  auto epwPath = resourcesPath / epwInputPath.filename();
  auto iddPath = resourcesPath / iddfilename;
  auto spawnPath = resourcesPath / spawnInputPath.filename();

  boost::filesystem::path epFMIDestPath;
  boost::filesystem::path epFMISourcePath;
  #ifdef __APPLE__
    Dl_info info;
    dladdr("main", &info);
    auto exedir = boost::filesystem::path(info.dli_fname).parent_path();
    epFMISourcePath = exedir / "../lib/libepfmi.dylib";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.dylib";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/darwin64/libepfmi.dylib";
  #elif _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    auto exedir = boost::filesystem::path(szPath).parent_path();
    epFMISourcePath = exedir / "epfmi.dll";
    epFMIDestPath = fmuStaggingPath / "binaries/win64/epfmi.dll";
  #else
    Dl_info info;
    dladdr("main", &info);
    auto exedir = boost::filesystem::path(info.dli_fname).parent_path();
    epFMISourcePath = exedir / "../lib/libepfmi.so";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.so";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/linux64/libepfmi.so";
  #endif

  // This would be the configuration in an install tree
  if (! boost::filesystem::exists(iddInputPath)) {
    iddInputPath = exedir / "../etc" / iddfilename;
  }

  // This would be the configuration in a developer tree
  if (! boost::filesystem::exists(iddInputPath)) {
    iddInputPath = exedir / iddfilename;
  }

  boost::filesystem::remove(fmupath);

  // Create fmu staging area and copy files into it
  boost::filesystem::create_directories(fmuStaggingPath);
  boost::filesystem::create_directories(resourcesPath);
  boost::filesystem::create_directories(epFMIDestPath.parent_path());

  boost::filesystem::copy_file(epFMISourcePath, epFMIDestPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(iddInputPath, iddPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(epwInputPath, epwPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(spawnInputPath, spawnPath, boost::filesystem::copy_option::overwrite_if_exists);

  // Make a copy of the idf input file, but remove unused objects
  std::ifstream input_stream(idfInputPath.string(), std::ifstream::in);
  std::string input_file;
  std::string line;
  while (std::getline(input_stream, line)) {
    input_file.append(line + EnergyPlus::DataStringGlobals::NL);
  }
  IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);
  auto jsonidf = parser.decode(input_file, schema);
  adjustSimulationControl(jsonidf);
  addRunPeriod(jsonidf);
  removeUnusedObjects(jsonidf);
  addOutputVariables(jsonidf, outputVariables);

  std::ofstream newidfstream(idfPath.string(),  std::ofstream::out |  std::ofstream::trunc);
  newidfstream << parser.encode(jsonidf, schema);
  newidfstream.close();

  // Create the modelDescription.xml file
  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto xmlvariables = doc.child("fmiModelDescription").child("ModelVariables");

  const auto epvariables = parseVariables(idfPath.string(),spawnInputPath.string());

  for (const auto & varpair : epvariables) {
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

  if (! nozip) {
    zip_directory(fmuStaggingPath.string(), fmupath.string(), nocompress);
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

