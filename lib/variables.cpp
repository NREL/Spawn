#include "variables.hpp"
#include "outputtypes.hpp"
#include "../cli/iddtypes.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/UtilityRoutines.hh"
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>


using json = nlohmann::json;

using namespace spawn::units;

class FMUInfo {
public:
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

    initZoneNames();
    initSchedules();
  }

  // Given the name of a schedule, return the idd type
  // If multiple schedules of the same name, but different type
  // are located in the idf, then the return value is ambiguous, and non determinant
  std::string scheduleType(const std::string & name) const {
    const auto & scheduleit = schedules.find(name);
    if( scheduleit != std::end(schedules)) {
      return scheduleit->second;
    }

    return "";
  }

  std::vector<std::string> zonenames;

private:

  void initSchedules() {
    for(const auto & type : supportedScheduleTypes) {
      for(const auto & schedule : jsonidf[type].items()) {
        schedules[schedule.key()] = type;
      }
    }
  }

  void initZoneNames() {
    std::string type = "Zone";

    if ( jsonidf.find(type) != jsonidf.end() ) {
      const auto zones = jsonidf[type];
      for( const auto & zone : zones.items() ) {
        zonenames.push_back(zone.key());
      }
    }

    std::sort(zonenames.begin(), zonenames.end());
  }

  // Key is a schedule name, value is the corresponding schedule type
  std::map<std::string, std::string> schedules;
  nlohmann::json jsonidf;
};

std::map<unsigned int, Variable> parseVariables(const std::string & idf,
    const std::string & jsonInput)
{
  std::map<unsigned int, Variable> result;

  FMUInfo fmuinfo(idf);

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

  const auto schedules = j.value("model",json()).value("schedules", std::vector<json>(0));
  for (const auto & schedule : schedules) {
    Variable var;
    var.type = VariableType::SCHEDULE;
    var.name = schedule.at("fmiName").get<std::string>();
    var.epunittype = spawn::units::UnitType::one;
    var.mounittype = spawn::units::UnitType::one;

    const auto & idfname = schedule.at("name").get<std::string>();
    var.actuatorcomponentkey = idfname;
    var.actuatorcomponenttype = fmuinfo.scheduleType(idfname);
    var.actuatorcontroltype = "Schedule Value";

    var.scalar_attributes.emplace_back("name",var.name);
    var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
    var.scalar_attributes.emplace_back("description","Schedule");
    var.scalar_attributes.emplace_back("causality","input");
    var.scalar_attributes.emplace_back("variability","continuous");

    var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
    var.real_attributes.emplace_back("start","0.0");

    result.emplace(i,std::move(var));
    ++i;
  }

  const auto outputVariables = j.value("model",json()).value("outputVariables", std::vector<json>(0));
  for (const auto & outputVariable : outputVariables) {
    const auto epname = outputVariable.at("name").get<std::string>();
    const auto epkey = outputVariable.at("key").get<std::string>();
    const auto fmiName = outputVariable.at("fmiName").get<std::string>();

    const auto build_variable = [&]() {
      Variable var;
      var.type = VariableType::SENSOR;
      var.name = fmiName;
      var.outputvarname = EnergyPlus::UtilityRoutines::MakeUPPERCase(epname);
      var.outputvarkey = EnergyPlus::UtilityRoutines::MakeUPPERCase(epkey);

      const auto & output = std::find_if(std::begin(outputtypes), std::end(outputtypes),
        [&](const std::pair<const char *, OutputProperties> & v) {
          return EnergyPlus::UtilityRoutines::MakeUPPERCase(v.first) == var.outputvarname;
        });

      if(output != std::end(outputtypes)) {
        var.epunittype = output->second.epUnitType;
        var.mounittype = output->second.moUnitType;
      } else {
        var.epunittype = spawn::units::UnitType::one;
        var.mounittype = spawn::units::UnitType::one;
      }

      var.scalar_attributes.emplace_back("name", fmiName);
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description", "Custom Sensor");
      var.scalar_attributes.emplace_back("causality", "output");
      var.scalar_attributes.emplace_back("variability", "continuous");
      var.scalar_attributes.emplace_back("initial", "calculated");

      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
      return var;
    };

    result.emplace(i, build_variable());
    ++i;
  }

  const auto actuators = j.value("model",json()).value("emsActuators", std::vector<json>(0));
  for (const auto & act : actuators) {
    const auto build_variable = [&]() {
      Variable var;
      var.type = VariableType::EMS_ACTUATOR;
      var.name = act.at("fmiName").get<std::string>();
      var.actuatorcomponentkey = act.at("variableName").get<std::string>();
      var.actuatorcomponenttype = act.at("componentType").get<std::string>();
      var.actuatorcontroltype = act.at("controlType").get<std::string>();
      var.epunittype = spawn::units::UnitType::one;
      var.mounittype = spawn::units::UnitType::one;

      var.scalar_attributes.emplace_back("name", var.name);
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description", "Custom Actuator");
      var.scalar_attributes.emplace_back("causality", "input");
      var.scalar_attributes.emplace_back("variability", "continuous");

      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
      var.real_attributes.emplace_back("start","0.0");
      return var;
    };
    result.emplace(i, build_variable());
    ++i;
  }

  // TODO: this variable initialization needs to be abstracted into a function or
  // constructor. The same code is repeated many times.
  const auto zones = fmuinfo.zonenames;
  for (const auto & zone : zones) {
    {
      Variable var;
      var.type = VariableType::T;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::C;
      var.mounittype = spawn::units::UnitType::K;

      var.scalar_attributes.emplace_back("name",zone + "_T");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Temperature of the zone air");
      var.scalar_attributes.emplace_back("causality","input");
      var.scalar_attributes.emplace_back("variability","continuous");

      var.real_attributes.emplace_back("quantity","ThermodynamicTemperature");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("start","0.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QCONSEN_FLOW;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::W;
      var.mounittype = spawn::units::UnitType::W;

      var.scalar_attributes.emplace_back("name",zone + "_QConSen_flow");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Convective sensible heat added to the zone");
      var.scalar_attributes.emplace_back("causality","output");
      var.scalar_attributes.emplace_back("variability","continuous");
      var.scalar_attributes.emplace_back("initial","calculated");

      var.real_attributes.emplace_back("quantity","Power");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::AFLO;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::m2;
      var.mounittype = spawn::units::UnitType::m2;

      var.scalar_attributes.emplace_back("name",zone + "_AFlo");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Floor area");
      var.scalar_attributes.emplace_back("causality","local");
      var.scalar_attributes.emplace_back("variability","constant");
      var.scalar_attributes.emplace_back("initial","exact");

      var.real_attributes.emplace_back("quantity","Area");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("start","12.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::V;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::m3;
      var.mounittype = spawn::units::UnitType::m3;

      var.scalar_attributes.emplace_back("name",zone + "_V");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Volume");
      var.scalar_attributes.emplace_back("causality","local");
      var.scalar_attributes.emplace_back("variability","constant");
      var.scalar_attributes.emplace_back("initial","exact");

      var.real_attributes.emplace_back("quantity","Volume");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("start","36.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::MSENFAC;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::one;
      var.mounittype = spawn::units::UnitType::one;

      var.scalar_attributes.emplace_back("name",zone + "_mSenFac");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Factor for scaling sensible thermal mass of volume");
      var.scalar_attributes.emplace_back("causality","local");
      var.scalar_attributes.emplace_back("variability","constant");
      var.scalar_attributes.emplace_back("initial","exact");

      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("start","1.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QGAIRAD_FLOW;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::W;
      var.mounittype = spawn::units::UnitType::W;

      var.scalar_attributes.emplace_back("name",zone + "_QGaiRad_flow");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Radiative sensible heat gain added to the zone");
      var.scalar_attributes.emplace_back("causality","input");
      var.scalar_attributes.emplace_back("variability","continuous");

      var.real_attributes.emplace_back("quantity","Power");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("start","0.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      var.setValue(0.0, spawn::units::UnitSystem::MO);

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QLAT_FLOW;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::W;
      var.mounittype = spawn::units::UnitType::W;

      var.scalar_attributes.emplace_back("name",zone + "_QLat_flow");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Latent heat gain added to the zone");
      var.scalar_attributes.emplace_back("causality","output");
      var.scalar_attributes.emplace_back("variability","continuous");
      var.scalar_attributes.emplace_back("initial","calculated");

      var.real_attributes.emplace_back("quantity","Power");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      // TODO exchange this variable with real EnergyPlus data
      var.setValue(0.0, spawn::units::UnitSystem::MO);

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::QPEO_FLOW;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::W;
      var.mounittype = spawn::units::UnitType::W;

      var.scalar_attributes.emplace_back("name",zone + "_QPeo_flow");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Heat gain due to people");
      var.scalar_attributes.emplace_back("causality","output");
      var.scalar_attributes.emplace_back("variability","continuous");
      var.scalar_attributes.emplace_back("initial","calculated");

      var.real_attributes.emplace_back("quantity","Power");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      // TODO exchange this variable with real EnergyPlus data
      var.setValue(0.0, spawn::units::UnitSystem::MO);

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::TAVEINLET;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::C;
      var.mounittype = spawn::units::UnitType::K;

      var.scalar_attributes.emplace_back("name",zone + "_TAveInlet");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Average of inlets medium temperatures carried by the mass flow rates");
      var.scalar_attributes.emplace_back("causality","input");
      var.scalar_attributes.emplace_back("variability","continuous");

      var.real_attributes.emplace_back("quantity","ThermodynamicTemperature");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("start","21.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      var.setValue(21.0, spawn::units::UnitSystem::EP);

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::TRAD;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::C;
      var.mounittype = spawn::units::UnitType::K;

      var.scalar_attributes.emplace_back("name",zone + "_TRad");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Average radiative temperature in the room");
      var.scalar_attributes.emplace_back("causality","output");
      var.scalar_attributes.emplace_back("variability","discrete");
      var.scalar_attributes.emplace_back("initial","calculated");

      var.real_attributes.emplace_back("quantity","ThermodynamicTemperature");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      // TODO exchange this variable with real EnergyPlus data
      var.setValue(21.0, spawn::units::UnitSystem::EP);

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::X;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::one;
      var.mounittype = spawn::units::UnitType::one;

      var.scalar_attributes.emplace_back("name",zone + "_X");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Water vapor mass fraction in kg water/kg dry air");
      var.scalar_attributes.emplace_back("causality","input");
      var.scalar_attributes.emplace_back("variability","continuous");

      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("min","0");
      var.real_attributes.emplace_back("start","0.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      // TODO exchange this variable with real EnergyPlus data
      var.setValue(0.0, spawn::units::UnitSystem::MO);

      result.emplace(i,std::move(var));
    }
    ++i;
    {
      Variable var;
      var.type = VariableType::MINLETS_FLOW;
      var.name = zone;
      var.epunittype = spawn::units::UnitType::kg_per_s;
      var.mounittype = spawn::units::UnitType::kg_per_s;

      var.scalar_attributes.emplace_back("name",zone + "_mInlets_flow");
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Sum of positive mass flow rates into the zone for all air inlets (including infiltration)");
      var.scalar_attributes.emplace_back("causality","input");
      var.scalar_attributes.emplace_back("variability","continuous");

      var.real_attributes.emplace_back("quantity","MassFlowRate");
      var.real_attributes.emplace_back("relativeQuantity","false");
      var.real_attributes.emplace_back("start","0.0");
      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

      // TODO exchange this variable with real EnergyPlus data
      var.setValue(0.0, spawn::units::UnitSystem::MO);

      result.emplace(i,std::move(var));
    }
    ++i;
  }

  return result;
}

void Variable::setValue(const double & value, const spawn::units::UnitSystem & system) {
	valueset = true;
	switch(system) {
		case UnitSystem::MO:
			this->value = value;
      break;
		case UnitSystem::EP:
			this->value = convert({value,epunittype},mounittype).value;
      break;
	}
}

double Variable::getValue(const spawn::units::UnitSystem & system) const {
	switch(system) {
		case UnitSystem::MO:
      return value;
		case UnitSystem::EP:
			return convert({value,mounittype},epunittype).value;
	}

  return 0.0;
}

void Variable::resetValue() {
  valueset = false;
}

bool Variable::isValueSet() const {
  return valueset;
}

