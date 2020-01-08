#include "Variables.hpp"
#include "EnergyPlus/InputProcessing/IdfParser.hh"
#include "EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "EnergyPlus/DataStringGlobals.hh"
#include "EnergyPlus/UtilityRoutines.hh"
#include "third_party/nlohmann/json.hpp"
#include <iostream>
#include <fstream>


using json = nlohmann::json;

//namespace EnergyPlus {
//namespace FMI {

struct FMUInfo {
  FMUInfo(const std::string & idfPath)
  {
    std::ifstream input_stream(idfPath, std::ifstream::in);

    std::string input_file;
    std::string line;
    while (std::getline(input_stream, line)) {
      input_file.append(line + EnergyPlus::DataStringGlobals::NL);
    }
    
    ::IdfParser parser;
    const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
    ::nlohmann::json schema = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);
    jsonidf = parser.decode(input_file, schema);

  }

  std::vector<std::string> zoneNames() const {
    std::vector<std::string> result;
    std::string type = "Zone";

    if ( jsonidf.find(type) != jsonidf.end() ) {
      const auto zones = jsonidf[type];
      for( const auto & zone : zones.items() ) {
        result.push_back(zone.key());
      }
    }

    std::sort(result.begin(), result.end());

    return result;
  }

  //std::vector<std::string> sensorNames() const {
  //  std::vector<std::string> result;
  //  std::string type = "EnergyManagementSystem:Sensor";

  //  if ( jsonidf.find(type) != jsonidf.end() ) {
  //    const auto sensors = jsonidf[type];
  //    for( const auto & sensor : sensors.items() ) {
  //      result.push_back(sensor.key());
  //    }
  //  }

  //  std::sort(result.begin(), result.end());

  //  return result;
  //}

  std::vector<std::string> actuatorNames() const {
    std::vector<std::string> result;
    std::string type = "EnergyManagementSystem:Actuator";

    if ( jsonidf.find(type) != jsonidf.end() ) {
      const auto actuators = jsonidf[type];
      for( const auto & actuator : actuators.items() ) {
        result.push_back(actuator.key());
      }
    }

    std::sort(result.begin(), result.end());

    return result;
  }

  nlohmann::json jsonidf;
};

std::map<unsigned int, Variable> parseVariables(const std::string & idf,
    const std::string & jsonInput) 
{
  std::map<unsigned int, Variable> result;

  FMUInfo fmuInfo(idf);

  unsigned int i = 0;

  json j;
  std::ifstream jsonFileInput(jsonInput);
  if (!jsonFileInput.fail()) {
    // deserialize from file
    jsonFileInput >> j;
  } else {
    // Try to parse command line input as json string
    j = json::parse(jsonInput, nullptr, false);
  }

  auto outputVariables = j.value("model",json()).value("outputVariables", std::vector<json>(0));
  for (const auto & outputVariable : outputVariables) {
    auto epname = outputVariable.at("name").get<std::string>();
    auto epkey = outputVariable.at("key").get<std::string>();
    auto fmiName = outputVariable.at("fmiName").get<std::string>();

    Variable var;
    var.type = VariableType::SENSOR;
    var.key = fmiName;
    var.epname = EnergyPlus::UtilityRoutines::MakeUPPERCase(epname);
    var.epkey = EnergyPlus::UtilityRoutines::MakeUPPERCase(epkey);

    var.scalar_attributes.emplace_back(std::make_pair("name",fmiName));
    var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
    var.scalar_attributes.emplace_back(std::make_pair("description","Custom Sensor"));
    var.scalar_attributes.emplace_back(std::make_pair("causality","output"));
    var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));
    var.scalar_attributes.emplace_back(std::make_pair("initial","calculated"));

    result.emplace(i,std::move(var));
    ++i;
  }

  //const auto sensors = fmuInfo.sensorNames();
  //for (const auto & sensor : sensors) {
  //  Variable var;
  //  var.type = VariableType::EMS_SENSOR;
  //  var.key = sensor;

  //  var.scalar_attributes.emplace_back(std::make_pair("name",sensor));
  //  var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
  //  var.scalar_attributes.emplace_back(std::make_pair("description","Custom Sensor"));
  //  var.scalar_attributes.emplace_back(std::make_pair("causality","output"));
  //  var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));
  //  var.scalar_attributes.emplace_back(std::make_pair("initial","calculated"));

  //  result.emplace(i,std::move(var));
  //  ++i;
  //}

  const auto actuators = fmuInfo.actuatorNames();
  for (const auto & actuator : actuators) {
    Variable var;
    var.type = VariableType::EMS_ACTUATOR;
    var.key = actuator;

    var.scalar_attributes.emplace_back(std::make_pair("name",actuator));
    var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
    var.scalar_attributes.emplace_back(std::make_pair("description","Custom Acutor"));
    var.scalar_attributes.emplace_back(std::make_pair("causality","input"));
    var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));

    result.emplace(i,std::move(var));
    ++i;
  }

  const auto zones = fmuInfo.zoneNames();
  for (const auto & zone : zones) {
    {
      Variable var;
      var.type = VariableType::T;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_T"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Temperature of the zone air"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","input"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));

      var.real_attributes.emplace_back(std::make_pair("quantity","ThermodynamicTemperature"));
      var.real_attributes.emplace_back(std::make_pair("unit","degC"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("start","0.0"));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QCONSEN_FLOW;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_QConSen_flow"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Convective sensible heat added to the zone"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","output"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));
      var.scalar_attributes.emplace_back(std::make_pair("initial","calculated"));

      var.real_attributes.emplace_back(std::make_pair("quantity","Power"));
      var.real_attributes.emplace_back(std::make_pair("unit","W"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::AFLO;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_AFlo"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Floor area"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","local"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","constant"));
      var.scalar_attributes.emplace_back(std::make_pair("initial","exact"));

      var.real_attributes.emplace_back(std::make_pair("quantity","Area"));
      var.real_attributes.emplace_back(std::make_pair("unit","m2"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("start","12.0"));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::V;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_V"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Volume"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","local"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","constant"));
      var.scalar_attributes.emplace_back(std::make_pair("initial","exact"));

      var.real_attributes.emplace_back(std::make_pair("quantity","Volume"));
      var.real_attributes.emplace_back(std::make_pair("unit","m3"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("start","36.0"));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::MSENFAC;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_mSenFac"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Factor for scaling sensible thermal mass of volume"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","local"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","constant"));
      var.scalar_attributes.emplace_back(std::make_pair("initial","exact"));

      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("start","1.0"));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QGAIRAD_FLOW;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_QGaiRad_flow"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Radiative sensible heat gain added to the zone"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","input"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));

      var.real_attributes.emplace_back(std::make_pair("quantity","Power"));
      var.real_attributes.emplace_back(std::make_pair("unit","W"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("start","0.0"));

      var.value = 0.0;

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QLAT_FLOW;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_QLat_flow"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Latent heat gain added to the zone"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","output"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));
      var.scalar_attributes.emplace_back(std::make_pair("initial","calculated"));

      var.real_attributes.emplace_back(std::make_pair("quantity","Power"));
      var.real_attributes.emplace_back(std::make_pair("unit","W"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));

      // TODO exchange this variable with real EnergyPlus data
      var.value = 0.0;

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QPEO_FLOW;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_QPeo_flow"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Heat gain due to people"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","output"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));
      var.scalar_attributes.emplace_back(std::make_pair("initial","calculated"));

      var.real_attributes.emplace_back(std::make_pair("quantity","Power"));
      var.real_attributes.emplace_back(std::make_pair("unit","W"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));

      // TODO exchange this variable with real EnergyPlus data
      var.value = 0.0;

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::TAVEINLET;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_TAveInlet"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Average of inlets medium temperatures carried by the mass flow rates"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","input"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));

      var.real_attributes.emplace_back(std::make_pair("quantity","ThermodynamicTemperature"));
      var.real_attributes.emplace_back(std::make_pair("unit","degC"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("start","21.0"));

      var.value = 21.0;

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::TRAD;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_TRad"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Average radiative temperature in the room"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","output"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","discrete"));
      var.scalar_attributes.emplace_back(std::make_pair("initial","calculated"));

      var.real_attributes.emplace_back(std::make_pair("quantity","ThermodynamicTemperature"));
      var.real_attributes.emplace_back(std::make_pair("unit","degC"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));

      // TODO exchange this variable with real EnergyPlus data
      var.value = 21.0;

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::X;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_X"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Water vapor mass fraction in kg water/kg dry air"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","input"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));

      var.real_attributes.emplace_back(std::make_pair("unit","1"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("min","0"));
      var.real_attributes.emplace_back(std::make_pair("start","0.0"));

      // TODO exchange this variable with real EnergyPlus data
      var.value = 0.0;

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::MINLETS_FLOW;
      var.key = zone;

      var.scalar_attributes.emplace_back(std::make_pair("name",zone + "_mInlets_flow"));
      var.scalar_attributes.emplace_back(std::make_pair("valueReference", std::to_string(i)));
      var.scalar_attributes.emplace_back(std::make_pair("description","Sum of positive mass flow rates into the zone for all air inlets (including infiltration)"));
      var.scalar_attributes.emplace_back(std::make_pair("causality","input"));
      var.scalar_attributes.emplace_back(std::make_pair("variability","continuous"));

      var.real_attributes.emplace_back(std::make_pair("quantity","MassFlowRate"));
      var.real_attributes.emplace_back(std::make_pair("unit","kg/s"));
      var.real_attributes.emplace_back(std::make_pair("relativeQuantity","false"));
      var.real_attributes.emplace_back(std::make_pair("start","0.0"));

      // TODO exchange this variable with real EnergyPlus data
      var.value = 0.0;

      result.emplace(i,std::move(var));
    }
    ++i;
  }

  return result;
}

//}
//}

