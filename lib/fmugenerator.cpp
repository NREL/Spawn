#include "fmugenerator.hpp"
#include "ziputil.hpp"
#include "modelDescription.xml.hpp"
#include "input/input.hpp"
#include "../util/compare.hpp"
#include "../util/fmi_paths.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/UtilityRoutines.hh"
#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"
#include <pugixml.hpp>
#include <fmt/format.h>

using json = nlohmann::json;

namespace spawn {

nlohmann::json idfToJSON(const std::filesystem::path & idfpath);
void jsonToIdf(const nlohmann::json & idfjson, const std::filesystem::path & idfpath);
nlohmann::json & adjustSimulationControl(nlohmann::json & jsonidf);
nlohmann::json & addRunPeriod(nlohmann::json & jsonidf);
nlohmann::json & removeUnusedObjects(nlohmann::json & jsonidf);
nlohmann::json & addOtherEquipment(nlohmann::json & jsonidf, const Input& input);
nlohmann::json & addOutputVariables(nlohmann::json & jsonidf, const Input& input);
void createModelDescription(const spawn::Input & input, const std::filesystem::path & savepath);

void energyplusToFMU(
  const std::string &jsoninput,
  bool nozip,
  bool nocompress,
  const std::string & outputpath,
  const std::string & outputdir,
  std::filesystem::path iddpath,
  std::filesystem::path epfmupath
) {
  spawn::Input input(jsoninput);

  // We are going to copy the required files into an FMU staging directory,
  // also copy the json input file into root of the fmu staging directory,
  // and rewrite the json input file paths so that they reflect the new paths
  // contained within the fmu layout.

  // The default fmu output path
  auto fmuPath = std::filesystem::current_path() / (input.fmuBaseName() + ".fmu");

  // These are options to override the default output path
  if( ! outputpath.empty() ) {
    fmuPath = std::filesystem::path(outputpath);
  } else if( ! outputdir.empty() ) {
    fmuPath = std::filesystem::path(outputdir) / (input.fmuBaseName() + ".fmu");
  }

  const auto outputroot = fmuPath.parent_path();
  const auto fmuStagingPath = outputroot / fmuPath.stem();

  if( ! std::filesystem::exists(outputroot) ) {
    std::filesystem::create_directories(outputroot);
  }

  const auto modelDescriptionPath = fmuStagingPath / "modelDescription.xml";
  const auto fmuResourcesPath = fmuStagingPath / "resources";
  const auto fmuspawnPath = fmuResourcesPath / "model.spawn";
  const auto fmuidfPath = fmuResourcesPath / input.idfInputPath().filename();
  const auto fmuepwPath = fmuResourcesPath / input.epwInputPath().filename();
  const auto fmuiddPath = fmuResourcesPath / iddpath.filename();
  const auto fmuEPFMIPath = fmuStagingPath / fmi_lib_path(epfmi_basename());

  std::filesystem::remove_all(fmuPath);
  std::filesystem::remove_all(fmuStagingPath);

  // Create fmu staging area and copy files into it
  std::filesystem::create_directories(fmuStagingPath);
  std::filesystem::create_directories(fmuResourcesPath);
  std::filesystem::create_directories(fmuEPFMIPath.parent_path());

  std::filesystem::copy_file(epfmupath, fmuEPFMIPath, std::filesystem::copy_options::overwrite_existing);
  std::filesystem::copy_file(iddpath, fmuiddPath, std::filesystem::copy_options::overwrite_existing);
  std::filesystem::copy_file(input.epwInputPath(), fmuepwPath, std::filesystem::copy_options::overwrite_existing);

  auto jsonidf = spawn::idfToJSON(input.idfInputPath());
  adjustSimulationControl(jsonidf);
  removeUnusedObjects(jsonidf);
  addRunPeriod(jsonidf);
  addOtherEquipment(jsonidf, input);
  addOutputVariables(jsonidf, input);
  spawn::jsonToIdf(jsonidf, fmuidfPath);

  createModelDescription(input, modelDescriptionPath);

  const auto relativeEPWPath = std::filesystem::relative(fmuepwPath, fmuResourcesPath);
  input.setEPWInputPath(relativeEPWPath);
  const auto relativeIdfPath = std::filesystem::relative(fmuidfPath, fmuResourcesPath);
  input.setIdfInputPath(relativeIdfPath);
  input.save(fmuspawnPath);

  if (! nozip) {
    zip_directory(fmuStagingPath.string(), fmuPath.string(), nocompress);
    std::filesystem::remove_all(fmuStagingPath);
  }

}

json idfToJSON(const std::filesystem::path & idfpath) {
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

void jsonToIdf(const json & jsonidf, const std::filesystem::path & idfpath) {
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

// An OtherEquipment object is added for each zone
// in order to support the Spawn input "QGaiRad_flow"
// A different approach would be to create a new EnergyPlus actuator,
// "otherRadiantGains" and include in the summation performed by
// the EnergyPlus function "SumAllInternalRadiationGains"
// Since there is already substantial manipulation of the idf by Spawn,
// this approach seems reasonable
// With this approach QGaiRad_flow will interface with the OtherEquipment actuator,
// Spawn user does not need to interface directly with the actuator
json & addOtherEquipment(json& jsonidf, const Input& input) {
  constexpr auto scheduletype = "Schedule:Constant";
  constexpr auto schedulename = "Spawn-RadiantGains-Schedule";

  jsonidf[scheduletype] = {
    {
      schedulename, {
        {"schedule_type_limits_name", ""},
        {"hourly_value", "1.0"}
      }
    }
  };

  for(const auto & zone : input.zones) {
    if( ! zone.isconnected ) continue;

    jsonidf[Zone::ep_qgairad_flow_object_type][zone.ep_qgairad_flow_object_name] = {
      {"fuel_type", "None"},
      {"zone_or_zonelist_name", zone.idfname},
      {"schedule_name", schedulename},
      {"design_level_calculation_method", "EquipmentLevel"},
      {"design_level", 0.0},
      {"power_per_zone_floor_area", 0.0},
      {"power_per_person", 0.0},
      {"fraction_latent", 0.0},
      {"fraction_radiant", 1.0},
      {"fraction_lost", 0.0}
    };
  }

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
      [&](const OutputProperties & v) {
        return case_insensitive_compare(v.name, name);
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
json & addOutputVariables(json& jsonidf, const Input& input) {
  // A pair that holds an output variable name and key,
  typedef std::pair<std::string, std::string> Varpair;

  // Make a list of the requested outputs
  std::vector<Varpair> requestedpairs;
  for(const auto & var : input.outputVariables) {
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

void createModelDescription(const spawn::Input & input, const std::filesystem::path & savepath) {
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

