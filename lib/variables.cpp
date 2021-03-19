#include "variables.hpp"
#include "outputtypes.hpp"
#include "iddtypes.hpp"
#include "input/input.hpp"
#include "input/schedule.hpp"
#include "input/zone.hpp"
#include "input/outputvariable.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/UtilityRoutines.hh"
#include <iostream>
#include <fstream>

using json = nlohmann::json;
using namespace spawn::units;

std::map<unsigned int, Variable> parseVariables(const spawn::Input & input) {
  std::map<unsigned int, Variable> result;
  int i = 0;

  for (const auto & schedule : input.schedules) {
    const auto & buildvar = [&]() {
      Variable var;
      var.type = VariableType::SCHEDULE;
      var.name = schedule.spawnname;
      var.epunittype = spawn::units::UnitType::one;
      var.mounittype = spawn::units::UnitType::one;

      var.actuatorcomponentkey = schedule.idfname;
      var.actuatorcomponenttype = schedule.idftype;
      var.actuatorcontroltype = "Schedule Value";

      var.scalar_attributes.emplace_back("name",var.name);
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description","Schedule");
      var.scalar_attributes.emplace_back("causality","input");
      var.scalar_attributes.emplace_back("variability","continuous");

      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
      var.real_attributes.emplace_back("start","0.0");

      return var;
    };

    result.emplace(i,buildvar());
    ++i;
  }

  for (const auto & outputVariable : input.outputVariables) {
    const auto & build = [&]() {
      Variable var;
      var.type = VariableType::SENSOR;
      var.name = outputVariable.spawnname;
      var.outputvarname = outputVariable.idfname;
      var.outputvarkey = outputVariable.idfkey;

      const auto & output = std::find_if(std::begin(outputtypes), std::end(outputtypes),
        [&](const OutputProperties & v) {
          return EnergyPlus::UtilityRoutines::MakeUPPERCase(v.name) == var.outputvarname;
        });

      if(output != std::end(outputtypes)) {
        var.epunittype = output->epUnitType;
        var.mounittype = output->moUnitType;
      } else {
        var.epunittype = spawn::units::UnitType::one;
        var.mounittype = spawn::units::UnitType::one;
      }

      var.scalar_attributes.emplace_back("name", outputVariable.spawnname);
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description", "Custom Sensor");
      var.scalar_attributes.emplace_back("causality", "output");
      var.scalar_attributes.emplace_back("variability", "continuous");
      var.scalar_attributes.emplace_back("initial", "calculated");

      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
      return var;
    };

    result.emplace(i, build());
    ++i;
  }

  for (const auto & act : input.emsActuators) {
    const auto build = [&]() {
      Variable var;
      var.type = VariableType::EMS_ACTUATOR;
      var.name = act.spawnname;
      var.actuatorcomponentkey = act.idfname;
      var.actuatorcomponenttype = act.idftype;
      var.actuatorcontroltype = act.idfcontroltype;
      var.epunittype = spawn::units::UnitType::one;
      var.mounittype = spawn::units::UnitType::one;

      var.scalar_attributes.emplace_back("name", act.spawnname);
      var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
      var.scalar_attributes.emplace_back("description", "Custom Actuator");
      var.scalar_attributes.emplace_back("causality", "input");
      var.scalar_attributes.emplace_back("variability", "continuous");

      var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
      var.real_attributes.emplace_back("start","0.0");
      return var;
    };
    result.emplace(i, build());
    ++i;
  }

  // TODO: this variable initialization needs to be abstracted into a function or
  // constructor. The same code is repeated many times.
  for (const auto & zone : input.zones) {
    if(zone.isconnected) {
      {
        Variable var;
        var.type = VariableType::T;
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::C;
        var.mounittype = spawn::units::UnitType::K;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_T");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::W;
        var.mounittype = spawn::units::UnitType::W;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_QConSen_flow");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::m2;
        var.mounittype = spawn::units::UnitType::m2;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_AFlo");
        var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
        var.scalar_attributes.emplace_back("description","Floor area");
        var.scalar_attributes.emplace_back("causality","calculatedParameter");
        var.scalar_attributes.emplace_back("variability","fixed");
        var.scalar_attributes.emplace_back("initial","calculated");

        var.real_attributes.emplace_back("quantity","Area");
        var.real_attributes.emplace_back("relativeQuantity","false");
        var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

        result.emplace(i,std::move(var));
      }
      ++i;
      {
        Variable var;
        var.type = VariableType::V;
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::m3;
        var.mounittype = spawn::units::UnitType::m3;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_V");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::one;
        var.mounittype = spawn::units::UnitType::one;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_mSenFac");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::W;
        var.mounittype = spawn::units::UnitType::W;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_QGaiRad_flow");
        var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
        var.scalar_attributes.emplace_back("description","Radiative sensible heat gain added to the zone");
        var.scalar_attributes.emplace_back("causality","input");
        var.scalar_attributes.emplace_back("variability","continuous");

        var.real_attributes.emplace_back("quantity","Power");
        var.real_attributes.emplace_back("relativeQuantity","false");
        var.real_attributes.emplace_back("start","0.0");
        var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

        var.actuatorcomponentkey = zone.ep_qgairad_flow_object_name;
        var.actuatorcomponenttype = spawn::Zone::ep_qgairad_flow_object_type;
        var.actuatorcontroltype = spawn::Zone::ep_qgairad_flow_object_controltype;

        result.emplace(i,std::move(var));
      }
      ++i;
      {
        Variable var;
        var.type = VariableType::QLAT_FLOW;
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::W;
        var.mounittype = spawn::units::UnitType::W;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_QLat_flow");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::W;
        var.mounittype = spawn::units::UnitType::W;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_QPeo_flow");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::C;
        var.mounittype = spawn::units::UnitType::K;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_TAveInlet");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::C;
        var.mounittype = spawn::units::UnitType::K;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_TRad");
        var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
        var.scalar_attributes.emplace_back("description","Average radiative temperature in the room");
        var.scalar_attributes.emplace_back("causality","output");
        var.scalar_attributes.emplace_back("variability","discrete");
        var.scalar_attributes.emplace_back("initial","calculated");

        var.real_attributes.emplace_back("quantity","ThermodynamicTemperature");
        var.real_attributes.emplace_back("relativeQuantity","false");
        var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));

        result.emplace(i,std::move(var));
      }
      ++i;
      {
        Variable var;
        var.type = VariableType::X;
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::one;
        var.mounittype = spawn::units::UnitType::one;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_X");
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
        var.name = zone.idfname;
        var.epunittype = spawn::units::UnitType::kg_per_s;
        var.mounittype = spawn::units::UnitType::kg_per_s;

        var.scalar_attributes.emplace_back("name",zone.idfname + "_mInlets_flow");
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
  }

  for (const auto & surface : input.surfaces) {
    if(surface.isconnected) {
      {
        Variable var;
        var.type = VariableType::ASURF;
        var.name = surface.idfname;
        var.epunittype = spawn::units::UnitType::m2;
        var.mounittype = spawn::units::UnitType::m2;

        var.scalar_attributes.emplace_back("name",surface.idfname + "_A");
        var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
        var.scalar_attributes.emplace_back("description","Area of the surface that is exposed to the thermal zone");
        var.scalar_attributes.emplace_back("causality","local");
        var.scalar_attributes.emplace_back("variability","constant");
        var.scalar_attributes.emplace_back("initial","exact");

        var.real_attributes.emplace_back("quantity","Area");
        var.real_attributes.emplace_back("relativeQuantity","false");
        var.real_attributes.emplace_back("start","0.0");
        var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
        var.setValue(0.0, spawn::units::UnitSystem::MO);

        result.emplace(i,std::move(var));
      }
      ++i;
      {
        Variable var;
        var.type = VariableType::QSURF_FLOW;
        var.name = surface.idfname;
        var.epunittype = spawn::units::UnitType::W;
        var.mounittype = spawn::units::UnitType::W;

        var.scalar_attributes.emplace_back("name",surface.idfname + "_Q_flow");
        var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
        var.scalar_attributes.emplace_back("description","Net heat flow rate from the thermal zone to the surface, consisting of convective heat flow, absorbed solar radiation, absorbed infrared radiation minus emitted infrared radiation.");
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
        var.type = VariableType::TSURF;
        var.name = surface.idfname;
        var.epunittype = spawn::units::UnitType::C;
        var.mounittype = spawn::units::UnitType::K;

        var.scalar_attributes.emplace_back("name",surface.idfname + "_T");
        var.scalar_attributes.emplace_back("valueReference", std::to_string(i));
        var.scalar_attributes.emplace_back("description","Temperature of the surface");
        var.scalar_attributes.emplace_back("causality","input");
        var.scalar_attributes.emplace_back("variability","continuous");

        var.real_attributes.emplace_back("quantity","ThermodynamicTemperature");
        var.real_attributes.emplace_back("relativeQuantity","false");
        var.real_attributes.emplace_back("start","0.0");
        var.real_attributes.emplace_back("unit",spawn::units::toString(var.mounittype));
        var.setValue(21.0, spawn::units::UnitSystem::EP);

        result.emplace(i,std::move(var));
      }
      ++i;
    }
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

