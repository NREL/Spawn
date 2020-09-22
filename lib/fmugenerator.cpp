#include "fmugenerator.hpp"
#include "input.hpp"
#include "ziputil.hpp"
#include "modelDescription.xml.hpp"
#include "../util/fmi_paths.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/UtilityRoutines.hh"
#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"
#include <boost/algorithm/string.hpp>
#include <pugixml.hpp>

using json = nlohmann::json;

namespace spawn {

nlohmann::json idfToJSON(const boost::filesystem::path & idfpath);
void jsonToIdf(const nlohmann::json & idfjson, const boost::filesystem::path & idfpath);
nlohmann::json & adjustSimulationControl(nlohmann::json & jsonidf);
nlohmann::json & addRunPeriod(nlohmann::json & jsonidf);
nlohmann::json & removeUnusedObjects(nlohmann::json & jsonidf);
nlohmann::json & addOutputVariables(nlohmann::json & jsonidf, const std::vector<spawn::OutputVariable> & requestedvars);

void createModelDescription(const spawn::Input & input, const boost::filesystem::path & savepath);

void energyplusToFMU(
  const std::string &jsoninput,
  bool nozip,
  bool nocompress,
  const std::string & outputpath,
  const std::string & outputdir,
  boost::filesystem::path iddpath,
  boost::filesystem::path epfmupath
) {
  spawn::Input input(jsoninput);

  // We are going to copy the required files into an FMU staging directory,
  // also copy the json input file into root of the fmu staging directory,
  // and rewrite the json input file paths so that they reflect the new paths
  // contained within the fmu layout.

  // The default fmu output path
  auto fmuPath = boost::filesystem::current_path() / (input.fmuBaseName() + ".fmu");

  // These are options to override the default output path
  if( ! outputpath.empty() ) {
    fmuPath = boost::filesystem::path(outputpath);
  } else if( ! outputdir.empty() ) {
    fmuPath = boost::filesystem::path(outputdir) / (input.fmuBaseName() + ".fmu");
  }

  const auto outputroot = fmuPath.parent_path();
  const auto fmuStagingPath = outputroot / fmuPath.stem();

  if( ! boost::filesystem::exists(outputroot) ) {
    boost::filesystem::create_directories(outputroot);
  }

  const auto modelDescriptionPath = fmuStagingPath / "modelDescription.xml";
  const auto fmuResourcesPath = fmuStagingPath / "resources";
  const auto fmuidfPath = fmuResourcesPath / input.idfInputPath().filename();
  const auto fmuepwPath = fmuResourcesPath / input.epwInputPath().filename();
  const auto fmuiddPath = fmuResourcesPath / iddpath.filename();
  const auto fmuspawnPath = fmuStagingPath / "model.spawn";
  const auto fmuEPFMIPath = fmuStagingPath / fmi_lib_path(epfmi_basename());


  boost::filesystem::remove_all(fmuPath);
  boost::filesystem::remove_all(fmuStagingPath);

  // Create fmu staging area and copy files into it
  boost::filesystem::create_directories(fmuStagingPath);
  boost::filesystem::create_directories(fmuResourcesPath);
  boost::filesystem::create_directories(fmuEPFMIPath.parent_path());

  boost::filesystem::copy_file(epfmupath, fmuEPFMIPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(iddpath, fmuiddPath, boost::filesystem::copy_option::overwrite_if_exists);
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
    boost::filesystem::remove_all(fmuStagingPath);
  }

}

json idfToJSON(const boost::filesystem::path & idfpath) {
  std::ifstream input_stream(idfpath.string(), std::ifstream::in);

  std::string input_file;
  std::string line;
  while (std::getline(input_stream, line)) {
    input_file.append(line + EnergyPlus::DataStringGlobals::NL);
  }

  ::IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);
  return parser.decode(input_file, schema);
}

void jsonToIdf(const json & jsonidf, const boost::filesystem::path & idfpath) {
  ::IdfParser parser;
  const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
  json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);

  std::ofstream newidfstream(idfpath.string(),  std::ofstream::out |  std::ofstream::trunc);
  newidfstream << parser.encode(jsonidf, schema);
  newidfstream.close();
}

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

} // namespace spawn

