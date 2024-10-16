#include "variables.hpp"
#include "../energyplus/idd/embedded/EmbeddedEpJSONSchema.hh"
#include "../energyplus/src/EnergyPlus/DataStringGlobals.hh"
#include "../energyplus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../energyplus/src/EnergyPlus/UtilityRoutines.hh"
#include "energyplus_helpers.hpp"
#include "iddtypes.hpp"
#include "input/outputvariable.hpp"
#include "input/schedule.hpp"
#include "input/surface.hpp"
#include "input/user_config.hpp"
#include "input/zone.hpp"
#include "output_types.hpp"
#include "spawn.hpp"
#include "spdlog/spdlog.h"
#include <EnergyPlusData.hh>
#include <fstream>
#include <iostream>
#include <vector>

using json = nlohmann::json;
using namespace spawn::units;

namespace spawn::variable {

Variables::Variables([[maybe_unused]] const UserConfig &user_config)
{
  zone::V::CreateAll(user_config, *this);
  zone::AFlo::CreateAll(user_config, *this);
  zone::MSenFac::CreateAll(user_config, *this);
  zone::QConSenFlow::CreateAll(user_config, *this);
  zone::QLatFlow::CreateAll(user_config, *this);
  zone::QPeoFlow::CreateAll(user_config, *this);
  zone::TRad::CreateAll(user_config, *this);

  zone::QCooSenFlow::CreateAll(user_config, *this);
  zone::QCooLatFlow::CreateAll(user_config, *this);
  zone::TOutCoo::CreateAll(user_config, *this);
  zone::XOutCoo::CreateAll(user_config, *this);
  zone::MOutCooFlow::CreateAll(user_config, *this);
  zone::TCoo::CreateAll(user_config, *this);
  zone::QHeaFlow::CreateAll(user_config, *this);
  zone::TOutHea::CreateAll(user_config, *this);
  zone::XOutHea::CreateAll(user_config, *this);
  zone::MOutHeaFlow::CreateAll(user_config, *this);
  zone::THea::CreateAll(user_config, *this);

  zone::MInletsFlow::CreateAll(user_config, *this);
  zone::TAveInlet::CreateAll(user_config, *this);
  zone::T::CreateAll(user_config, *this);
  zone::X::CreateAll(user_config, *this);
  zone::QGaiRadFlow::CreateAll(user_config, *this);
  other::Sensor::CreateAll(user_config, *this);
  other::Actuator::CreateAll(user_config, *this);
  other::Schedule::CreateAll(user_config, *this);
  surface::A::CreateAll(user_config, *this);
  surface::QFlow::CreateAll(user_config, *this);
  surface::T::CreateAll(user_config, *this);
  construction::A::CreateAll(user_config, *this);
  construction::QFrontFlow::CreateAll(user_config, *this);
  construction::QBackFlow::CreateAll(user_config, *this);
  construction::TFront::CreateAll(user_config, *this);
  construction::TBack::CreateAll(user_config, *this);
}

const VariableVector &Variables::AllVariables() const
{
  return all_variables_;
}

const VariableRefs &Variables::Inputs() const
{
  return input_variables_;
}

const VariableRefs &Variables::Outputs() const
{
  return output_variables_;
}

const VariableRefs &Variables::Parameters() const
{
  return parameter_variables_;
}

void Variables::UpdateInputs(EnergyPlus::EnergyPlusData &energyplus_data)
{
  std::for_each(input_variables_.begin(), input_variables_.end(), [&energyplus_data](auto var) {
    var.get().Update(energyplus_data);
  });
}

void Variables::UpdateOutputs(EnergyPlus::EnergyPlusData &energyplus_data)
{
  std::for_each(output_variables_.begin(), output_variables_.end(), [&energyplus_data](auto var) {
    var.get().Update(energyplus_data);
  });
}

void Variables::UpdateParameters(EnergyPlus::EnergyPlusData &energyplus_data)
{
  std::for_each(parameter_variables_.begin(), parameter_variables_.end(), [&energyplus_data](auto var) {
    var.get().Update(energyplus_data);
  });
}

int Variables::VariableIndex(const std::string_view variable_name) const
{
  try {
    return variable_name_index_.at(std::string(variable_name));
  } catch (...) {
    throw std::runtime_error(fmt::format("Attempt to retrieve an invalid variable name: {}", variable_name));
  }
}

void Variables::AddVariable(std::unique_ptr<Variable> &&variable)
{
  variable_name_index_[std::string(variable->Name())] = variable->Index();
  all_variables_.push_back(std::move(variable));
}

void Variables::AddVariable(Output &variable)
{
  output_variables_.push_back(variable);
}

void Variables::AddVariable(Parameter &variable)
{
  parameter_variables_.push_back(variable);
}

void Variables::AddVariable(Input &variable)
{
  input_variables_.push_back(variable);
}

Variable::Variable(Variables &variables,
                   std::string_view name,
                   units::UnitType ep_unit, // NOLINT
                   units::UnitType mo_unit)
    : name_(name), ep_unit_(ep_unit), mo_unit_(mo_unit), index_((int)variables.AllVariables().size())
{
  variables.AddVariable(std::unique_ptr<Variable>(this));
}

std::string_view Variable::Name() const
{
  return name_;
}

int Variable::Index() const
{
  return index_;
}

const pugi::xml_document &Variable::Metadata() const
{
  return metadata_;
}

std::optional<double> Variable::Value(const units::UnitSystem &unit) const
{
  if (value_) {
    switch (unit) {
    case UnitSystem::MO:
      return value_;
    case UnitSystem::EP:
      return units::convert({*value_, mo_unit_}, ep_unit_).value;
    }
  }
  return {};
}

void Variable::SetValue(const double &value, const units::UnitSystem &unit)
{
  switch (unit) {
  case UnitSystem::MO:
    value_ = value;
    break;
  case UnitSystem::EP:
    value_ = units::convert({value, ep_unit_}, mo_unit_).value;
    break;
  }
}

void Variable::ResetValue()
{
  value_.reset();
}

Output::Output(Variables &variables, std::string_view name, units::UnitType ep_unit, units::UnitType mo_unit)
    : Variable(variables, name, ep_unit, mo_unit)
{
  variables.AddVariable(*this);
}

void Output::SetValue([[maybe_unused]] const double &value, [[maybe_unused]] const units::UnitSystem &unit)
{
  spdlog::info("Attempt to write a value to the Output named, {}, which is not allowed", name_);
}

void Output::ResetValue()
{
  spdlog::info("Attempt to reset the value of an Output named, {}, which is not allowed", name_);
}

Parameter::Parameter(Variables &variables, std::string_view name, units::UnitType ep_unit, units::UnitType mo_unit)
    : Variable(variables, name, ep_unit, mo_unit)
{
  variables.AddVariable(*this);
}

void Parameter::SetValue([[maybe_unused]] const double &value, [[maybe_unused]] const units::UnitSystem &unit)
{
  spdlog::info("Attempt to write a value to the Parameter named, {}, which is not allowed", name_);
}

void Parameter::ResetValue()
{
  spdlog::info("Attempt to reset the value of a Parameter named, {}, which is not allowed", name_);
}

Input::Input(Variables &variables, std::string_view name, units::UnitType ep_unit, units::UnitType mo_unit)
    : Variable(variables, name, ep_unit, mo_unit)
{
  variables.AddVariable(*this);
}

namespace zone {

  void V::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<V>(variables, zone_name);
    }
  }

  V::V(Variables &variables, const std::string_view zone_name) // NOLINT
      : Parameter(variables, std::string(zone_name) + "_V", units::UnitType::m3, units::UnitType::m3),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Volume";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Volume";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void V::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneVolume(energyplus_data, zone_num_.get(energyplus_data)), units::UnitSystem::EP);
  }

  void AFlo::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<AFlo>(variables, zone_name);
    }
  }

  AFlo::AFlo(Variables &variables, const std::string_view zone_name)
      : Parameter(variables, std::string(zone_name) + "_AFlo", units::UnitType::m2, units::UnitType::m2),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Floor area";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Area";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void AFlo::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneFloorArea(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void MSenFac::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<MSenFac>(variables, zone_name);
    }
  }

  MSenFac::MSenFac(Variables &variables, const std::string_view zone_name)
      : Parameter(variables, std::string(zone_name) + "_mSenFac", units::UnitType::one, units::UnitType::one),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Factor for scaling sensible thermal mass of volume";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void MSenFac::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneVolCapMultpSens(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void QConSenFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<QConSenFlow>(variables, zone_name);
    }
  }

  QConSenFlow::QConSenFlow(Variables &variables, const std::string_view zone_name)
      : Output(variables, std::string(zone_name) + "_QConSen_flow", units::UnitType::W, units::UnitType::W),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Convective sensible heat added to the zone";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QConSenFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    const double &value = energyplus::ZoneSums(energyplus_data, zone_num_.get(energyplus_data)).QConSenFlow();
    Variable::SetValue(value, units::UnitSystem::EP);
  }

  void QLatFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<QLatFlow>(variables, zone_name);
    }
  }

  QLatFlow::QLatFlow(Variables &variables, const std::string_view zone_name)
      : Output(variables, std::string(zone_name) + "_QLat_flow", units::UnitType::W, units::UnitType::W),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Latent heat added to the zone";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QLatFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneLatentGain(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void QPeoFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<QPeoFlow>(variables, zone_name);
    }
  }

  QPeoFlow::QPeoFlow(Variables &variables, const std::string_view zone_name) // NOLINT
      : Output(variables, std::string(zone_name) + "_QPeo_flow", units::UnitType::W, units::UnitType::W),
        zone_name_(zone_name), sensor_handle_([this](EnergyPlus::EnergyPlusData &data) {
          return energyplus::VariableHandle(data, "Zone People Total Heating Rate", zone_name_);
        })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Heat gain due to people";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QPeoFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::VariableValue(energyplus_data, sensor_handle_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void TRad::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<TRad>(variables, zone_name);
    }
  }

  TRad::TRad(Variables &variables, const std::string_view zone_name)
      : Output(variables, std::string(zone_name) + "_TRad", units::UnitType::C, units::UnitType::K),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Average radiative temperature in the room";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TRad::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneMeanRadiantTemp(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void MInletsFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<MInletsFlow>(variables, zone_name);
    }
  }

  MInletsFlow::MInletsFlow(Variables &variables, const std::string_view zone_name) // NOLINT
      : Input(
            variables, std::string(zone_name) + "_mInlets_flow", units::UnitType::kg_per_s, units::UnitType::kg_per_s),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") =
        "Sum of positive mass flow rates into the zone for all air inlets (including infiltration)";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "MassFlowRate";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void MInletsFlow::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  {
    // TODO: Do something with this value.
    // It should probably impact the surface heat transfer coefficients.
    // Right?
  }

  void TAveInlet::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<TAveInlet>(variables, zone_name);
    }
  }

  TAveInlet::TAveInlet(Variables &variables, const std::string_view zone_name)
      : Input(variables, std::string(zone_name) + "_TAveInlet", units::UnitType::C, units::UnitType::K),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") =
        "Average of inlets medium temperatures carried by the mass flow rates";
    scalar_variable.append_attribute(" causality ") = " input ";
    scalar_variable.append_attribute(" variability ") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TAveInlet::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  {
    // TODO: Do something with this value.
  }

  void T::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<T>(variables, zone_name);
    }
  }

  T::T(Variables &variables, const std::string_view zone_name)
      : Input(variables, std::string(zone_name) + "_T", units::UnitType::C, units::UnitType::K), zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Temperature of the zone air";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void T::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetZoneTemperature(energyplus_data, zone_num_.get(energyplus_data), *v);
    }
  }

  void X::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<X>(variables, zone_name);
    }
  }

  X::X(Variables &variables, const std::string_view zone_name)
      : Input(variables, std::string(zone_name) + "_X", units::UnitType::one, units::UnitType::one),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Water vapor mass fraction in kg water/kg dry air";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void X::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetZoneHumidityRatio(energyplus_data, zone_num_.get(energyplus_data), *v);
    }
  }

  void QGaiRadFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<QGaiRadFlow>(variables, zone_name);
    }
  }

  QGaiRadFlow::QGaiRadFlow(Variables &variables, const std::string_view zone_name)
      : Input(variables, std::string(zone_name) + "_QGaiRad_flow", units::UnitType::W, units::UnitType::W),
        zone_name_{zone_name}, actuator_name_(fmt::format("Spawn-Zone-{}-RadiantGains", zone_name_)),
        handle_([this](EnergyPlus::EnergyPlusData &data) {
          return energyplus::ActuatorHandle(data, actuator_type_, actuator_controltype_, actuator_name_);
        })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Radiative sensible heat gain added to the zone";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QGaiRadFlow::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetActuatorValue(energyplus_data, handle_.get(energyplus_data), *v);
    }
  }

  void QCooSenFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<QCooSenFlow>(variables, zone_name);
    }
  }

  QCooSenFlow::QCooSenFlow(Variables &variables, const std::string_view zone_name) // NOLINT
      : Parameter(variables, std::string(zone_name) + "_QCooSen_flow", units::UnitType::W, units::UnitType::W),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Design sensible cooling load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QCooSenFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneDesignCoolingLoad(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void QCooLatFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<QCooLatFlow>(variables, zone_name);
    }
  }

  QCooLatFlow::QCooLatFlow(Variables &variables, const std::string_view zone_name) // NOLINT
      : Parameter(variables, std::string(zone_name) + "_QCooLat_flow", units::UnitType::W, units::UnitType::W),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Design latent cooling load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QCooLatFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneDesignCoolingLatentLoad(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void TOutCoo::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<TOutCoo>(variables, zone_name);
    }
  }

  TOutCoo::TOutCoo(Variables &variables, const std::string_view zone_name) // NOLINT
      : Parameter(variables, std::string(zone_name) + "_TOutCoo", units::UnitType::C, units::UnitType::K),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Outdoor drybulb temperature at the cooling design load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TOutCoo::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneOutdoorTempAtPeakCool(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void XOutCoo::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<XOutCoo>(variables, zone_name);
    }
  }

  XOutCoo::XOutCoo(Variables &variables, const std::string_view zone_name) // NOLINT
      : Parameter(variables, std::string(zone_name) + "_XOutCoo", units::UnitType::one, units::UnitType::one),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") =
        "Outdoor humidity ratio at the cooling design load per total air mass of the zone";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void XOutCoo::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneOutdoorHumidityRatioAtPeakCool(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void MOutCooFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<MOutCooFlow>(variables, zone_name);
    }
  }

  MOutCooFlow::MOutCooFlow(Variables &variables, const std::string_view zone_name)
      : Parameter(
            variables, std::string(zone_name) + "_mOutCoo_flow", units::UnitType::kg_per_s, units::UnitType::kg_per_s),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Minimum outdoor air flow rate during the cooling design load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "MassFlowRate";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void MOutCooFlow::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  {
    // TODO: get this value
    Variable::SetValue(0.0, units::UnitSystem::EP);
  }

  void TCoo::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<TCoo>(variables, zone_name);
    }
  }

  TCoo::TCoo(Variables &variables, const std::string_view zone_name)
      : Parameter(variables, std::string(zone_name) + "_TCoo", units::UnitType::s, units::UnitType::s),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Time at which these loads occurred";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Time";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TCoo::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneTimeAtPeakCool(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void QHeaFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<QHeaFlow>(variables, zone_name);
    }
  }

  QHeaFlow::QHeaFlow(Variables &variables, const std::string_view zone_name)
      : Parameter(variables, std::string(zone_name) + "_QHea_flow", units::UnitType::W, units::UnitType::W),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Design heating load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QHeaFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneDesignHeatingLoad(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void TOutHea::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<TOutHea>(variables, zone_name);
    }
  }

  TOutHea::TOutHea(Variables &variables, const std::string_view zone_name)
      : Parameter(variables, std::string(zone_name) + "_TOutHea", units::UnitType::C, units::UnitType::K),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Outdoor drybulb temperature at the heating design load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TOutHea::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneOutdoorTempAtPeakHeat(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void XOutHea::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<XOutHea>(variables, zone_name);
    }
  }

  XOutHea::XOutHea(Variables &variables, const std::string_view zone_name) // NOLINT
      : Parameter(variables, std::string(zone_name) + "_XOutHea", units::UnitType::one, units::UnitType::one),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") =
        "Outdoor humidity ratio at the heating design load per total air mass of the zone";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void XOutHea::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneOutdoorHumidityRatioAtPeakHeat(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void MOutHeaFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<MOutHeaFlow>(variables, zone_name);
    }
  }

  MOutHeaFlow::MOutHeaFlow(Variables &variables, const std::string_view zone_name)
      : Parameter(
            variables, std::string(zone_name) + "_mOutHea_flow", units::UnitType::kg_per_s, units::UnitType::kg_per_s),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Minimum outdoor air flow rate during the heating design load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "MassFlowRate";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void MOutHeaFlow::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  {
    // TODO: get this value
    Variable::SetValue(0.0, units::UnitSystem::EP);
  }

  void THea::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

    for (const auto &zone : zones) {
      std::string zone_name = zone.value("name", "");
      Variables::CreateOne<THea>(variables, zone_name);
    }
  }

  THea::THea(Variables &variables, const std::string_view zone_name)
      : Parameter(variables, std::string(zone_name) + "_THea", units::UnitType::s, units::UnitType::s),
        zone_name_(zone_name),
        zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Time at which these loads occurred";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Time";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void THea::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::ZoneTimeAtPeakHeat(energyplus_data, zone_num_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

} // namespace zone

namespace hvaczones {
  void QCooSenFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto groups = user_config.spawnjson.value("model", json::object()).value("hvacZones", std::vector<json>(0));

    for (const auto &group : groups) {
      std::string group_name = group.value("name", "");
      const auto zones = group.value("zones", std::vector<json>(0));
      std::vector<std::string> zone_names(zones.size());
      std::transform(
          zones.begin(), zones.end(), zone_names.begin(), [](const auto &zone) { return zone.value("name", ""); });
      Variables::CreateOne<QCooSenFlow>(variables, group_name, zone_names);
    }
  }

  QCooSenFlow::QCooSenFlow(Variables &variables,
                           const std::string_view group_name,
                           const std::vector<std::string> &zone_names) // NOLINT
      : Parameter(variables,
                  std::string("hvac_sizing_group_") + std::string(group_name) + "_QCooSen_flow",
                  units::UnitType::W,
                  units::UnitType::W),
        zone_names_(zone_names),
        zone_nums_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNums(data, zone_names_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Design sensible cooling load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QCooSenFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::HVACSizingGroupDesignCoolingLoad(energyplus_data, zone_nums_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void QCooLatFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto groups = user_config.spawnjson.value("model", json::object()).value("hvacZones", std::vector<json>(0));

    for (const auto &group : groups) {
      std::string group_name = group.value("name", "");
      const auto zones = group.value("zones", std::vector<json>(0));
      std::vector<std::string> zone_names(zones.size());
      std::transform(
          zones.begin(), zones.end(), zone_names.begin(), [](const auto &zone) { return zone.value("name", ""); });
      Variables::CreateOne<QCooLatFlow>(variables, group_name, zone_names);
    }
  }

  QCooLatFlow::QCooLatFlow(Variables &variables,
                           const std::string_view group_name,
                           const std::vector<std::string> &zone_names) // NOLINT
      : Parameter(variables,
                  std::string("hvac_sizing_group_") + std::string(group_name) + "_QCooLat_flow",
                  units::UnitType::W,
                  units::UnitType::W),
        zone_names_(zone_names),
        zone_nums_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNums(data, zone_names_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Design latent cooling load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QCooLatFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(
        energyplus::HVACSizingGroupDesignCoolingLatentLoad(energyplus_data, zone_nums_.get(energyplus_data)),
        units::UnitSystem::EP);
  }

  void TOutCoo::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto groups = user_config.spawnjson.value("model", json::object()).value("hvacZones", std::vector<json>(0));

    for (const auto &group : groups) {
      std::string group_name = group.value("name", "");
      const auto zones = group.value("zones", std::vector<json>(0));
      std::vector<std::string> zone_names(zones.size());
      std::transform(
          zones.begin(), zones.end(), zone_names.begin(), [](const auto &zone) { return zone.value("name", ""); });
      Variables::CreateOne<TOutCoo>(variables, group_name, zone_names);
    }
  }

  TOutCoo::TOutCoo(Variables &variables,
                   const std::string_view group_name,
                   const std::vector<std::string> &zone_names) // NOLINT
      : Parameter(variables,
                  std::string("hvac_sizing_group_") + std::string(group_name) + "_TOutCoo",
                  units::UnitType::C,
                  units::UnitType::K),
        zone_names_(zone_names),
        zone_nums_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNums(data, zone_names_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Outdoor drybulb temperature at the cooling design load";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TOutCoo::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(
        energyplus::HVACSizingGroupOutdoorTempAtPeakCool(energyplus_data, zone_nums_.get(energyplus_data)),
        units::UnitSystem::EP);
  }

  // void XOutCoo::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<XOutCoo>(variables, zone_name);
  //  }
  //}

  // XOutCoo::XOutCoo(Variables &variables, const std::string_view zone_name) // NOLINT
  //     : Parameter(variables, std::string(zone_name) + "_XOutCoo", units::UnitType::one, units::UnitType::one),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") =
  //       "Outdoor humidity ratio at the cooling design load per total air mass of the zone";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void XOutCoo::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   Variable::SetValue(energyplus::ZoneOutdoorHumidityRatioAtPeakCool(energyplus_data,
  //   zone_num_.get(energyplus_data)),
  //                      units::UnitSystem::EP);
  // }

  // void MOutCooFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<MOutCooFlow>(variables, zone_name);
  //  }
  //}

  // MOutCooFlow::MOutCooFlow(Variables &variables, const std::string_view zone_name)
  //     : Parameter(
  //           variables, std::string(zone_name) + "_mOutCoo_flow", units::UnitType::kg_per_s,
  //           units::UnitType::kg_per_s),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") = "Minimum outdoor air flow rate during the cooling design load";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("quantity") = "MassFlowRate";
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void MOutCooFlow::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   // TODO: get this value
  //   Variable::SetValue(0.0, units::UnitSystem::EP);
  // }

  // void TCoo::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<TCoo>(variables, zone_name);
  //  }
  //}

  // TCoo::TCoo(Variables &variables, const std::string_view zone_name)
  //     : Parameter(variables, std::string(zone_name) + "_TCoo", units::UnitType::s, units::UnitType::s),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") = "Time at which these loads occurred";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("quantity") = "Time";
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void TCoo::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   Variable::SetValue(energyplus::ZoneTimeAtPeakCool(energyplus_data, zone_num_.get(energyplus_data)),
  //                      units::UnitSystem::EP);
  // }

  // void QHeaFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<QHeaFlow>(variables, zone_name);
  //  }
  //}

  // QHeaFlow::QHeaFlow(Variables &variables, const std::string_view zone_name)
  //     : Parameter(variables, std::string(zone_name) + "_QHea_flow", units::UnitType::W, units::UnitType::W),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") = "Design heating load";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("quantity") = "Power";
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void QHeaFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   Variable::SetValue(energyplus::ZoneDesignHeatingLoad(energyplus_data, zone_num_.get(energyplus_data)),
  //                      units::UnitSystem::EP);
  // }

  // void TOutHea::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<TOutHea>(variables, zone_name);
  //  }
  //}

  // TOutHea::TOutHea(Variables &variables, const std::string_view zone_name)
  //     : Parameter(variables, std::string(zone_name) + "_TOutHea", units::UnitType::C, units::UnitType::K),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") = "Outdoor drybulb temperature at the heating design load";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("quantity") = "ThermodynamicTemperature";
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void TOutHea::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   Variable::SetValue(energyplus::ZoneOutdoorTempAtPeakHeat(energyplus_data, zone_num_.get(energyplus_data)),
  //                      units::UnitSystem::EP);
  // }

  // void XOutHea::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<XOutHea>(variables, zone_name);
  //  }
  //}

  // XOutHea::XOutHea(Variables &variables, const std::string_view zone_name) // NOLINT
  //     : Parameter(variables, std::string(zone_name) + "_XOutHea", units::UnitType::one, units::UnitType::one),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") =
  //       "Outdoor humidity ratio at the heating design load per total air mass of the zone";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void XOutHea::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   Variable::SetValue(energyplus::ZoneOutdoorHumidityRatioAtPeakHeat(energyplus_data,
  //   zone_num_.get(energyplus_data)),
  //                      units::UnitSystem::EP);
  // }

  // void MOutHeaFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<MOutHeaFlow>(variables, zone_name);
  //  }
  //}

  // MOutHeaFlow::MOutHeaFlow(Variables &variables, const std::string_view zone_name)
  //     : Parameter(
  //           variables, std::string(zone_name) + "_mOutHea_flow", units::UnitType::kg_per_s,
  //           units::UnitType::kg_per_s),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") = "Minimum outdoor air flow rate during the heating design load";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("quantity") = "MassFlowRate";
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void MOutHeaFlow::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   // TODO: get this value
  //   Variable::SetValue(0.0, units::UnitSystem::EP);
  // }

  // void THea::CreateAll(const UserConfig &user_config, Variables &variables)
  //{
  //   const auto zones = user_config.spawnjson.value("model", json::object()).value("zones", std::vector<json>(0));

  //  for (const auto &zone : zones) {
  //    std::string zone_name = zone.value("name", "");
  //    Variables::CreateOne<THea>(variables, zone_name);
  //  }
  //}

  // THea::THea(Variables &variables, const std::string_view zone_name)
  //     : Parameter(variables, std::string(zone_name) + "_THea", units::UnitType::s, units::UnitType::s),
  //       zone_name_(zone_name),
  //       zone_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::ZoneNum(data, zone_name_); })
  //{
  //   auto scalar_variable = metadata_.append_child("ScalarVariable");
  //   scalar_variable.append_attribute("name") = name_.c_str();
  //   scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
  //   scalar_variable.append_attribute("description") = "Time at which these loads occurred";
  //   scalar_variable.append_attribute("causality") = "calculatedParameter";
  //   scalar_variable.append_attribute("variability") = "fixed";
  //   scalar_variable.append_attribute("initial") = "calculated";

  //  auto real = scalar_variable.append_child("Real");
  //  real.append_attribute("quantity") = "Time";
  //  real.append_attribute("relativeQuantity") = "false";
  //  real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  //}

  // void THea::Update([[maybe_unused]] EnergyPlus::EnergyPlusData &energyplus_data)
  //{
  //   Variable::SetValue(energyplus::ZoneTimeAtPeakHeat(energyplus_data, zone_num_.get(energyplus_data)),
  //                      units::UnitSystem::EP);
  // }
} // namespace hvaczones

namespace other {
  void Sensor::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto output_variables =
        user_config.spawnjson.value("model", json::object()).value("outputVariables", std::vector<json>(0));

    for (const auto &variable : output_variables) {
      const auto sensor_name = variable.value("fmiName", "");
      const auto energyplus_name = variable.value("name", "");
      const auto energyplus_key = variable.value("key", "");

      Variables::CreateOne<Sensor>(variables, sensor_name, energyplus_name, energyplus_key);
    }
  }

  Sensor::Sensor(Variables &variables,
                 const std::string_view sensor_name, // NOLINT
                 const std::string_view energyplus_name,
                 const std::string_view energyplus_key)
      : Output(variables, sensor_name, units::UnitType::one, units::UnitType::one), energyplus_name_(energyplus_name),
        energyplus_key_(energyplus_key), sensor_handle_([this](EnergyPlus::EnergyPlusData &data) {
          return energyplus::VariableHandle(data, energyplus_name_, energyplus_key_);
        })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Custom sensor";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();

    const auto &output_type = energyplus::FindOutputTypeByName(energyplus_name_);
    ep_unit_ = output_type.epUnitType;
    mo_unit_ = output_type.moUnitType;
  }

  void Sensor::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::VariableValue(energyplus_data, sensor_handle_.get(energyplus_data)),
                       units::UnitSystem::EP);
  }

  void Actuator::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto actuators =
        user_config.spawnjson.value("model", json::object()).value("emsActuators", std::vector<json>(0));

    for (const auto &actuator : actuators) {
      const auto &name = actuator.value("fmiName", "");
      const auto &component_name = actuator.value("variableName", "");
      const auto &component_type = actuator.value("componentType", "");
      const auto &component_control_type = actuator.value("controlType", "");

      Variables::CreateOne<Actuator>(variables, name, component_name, component_type, component_control_type);
    }
  }

  Actuator::Actuator(Variables &variables,
                     const std::string_view name, // NOLINT
                     const std::string_view component_name,
                     const std::string_view component_type,
                     const std::string_view component_controltype)
      : Input(variables, name, units::UnitType::one, units::UnitType::one), component_name_(component_name),
        component_type_(component_type), component_controltype_(component_controltype),
        actuator_handle_([this](EnergyPlus::EnergyPlusData &data) {
          return energyplus::ActuatorHandle(data, component_type_, component_controltype_, component_name_);
        })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Custom actuator";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void Actuator::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetActuatorValue(energyplus_data, actuator_handle_.get(energyplus_data), *v);
    } else {
      energyplus::ResetActuator(energyplus_data, actuator_handle_.get(energyplus_data));
    }
  }

  void Schedule::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto schedules =
        user_config.spawnjson.value("model", json::object()).value("schedules", std::vector<json>(0));

    // Schedules values are set using the EnergyPlus actuator
    //
    // Since actuators require an EnergyPlus object type and name,
    // the first step is to build a map of schedules names and types.
    std::map<std::string, std::string> schedule_type_map;

    for (const auto &type : supportedScheduleTypes) {
      if (user_config.jsonidf.contains(type)) {
        for (const auto &schedule : user_config.jsonidf[type].items()) {
          schedule_type_map[schedule.key()] = type;
        }
      }
    }

    // Use the type map to find the schedule type,
    // then find the actuator handle,
    // and finally create a new Schedule variable.
    for (const auto &schedule : schedules) {
      const auto idf_name = schedule.value("name", "");
      const auto name = schedule.value("fmiName", "");

      const auto type = schedule_type_map.find(idf_name);
      if (type != std::end(schedule_type_map)) {
        Variables::CreateOne<Schedule>(variables, name, idf_name, type->second, "Schedule Value");
      }
    }
  }

  Schedule::Schedule(Variables &variables,
                     const std::string_view name, // NOLINT
                     const std::string_view component_name,
                     const std::string_view component_type,
                     const std::string_view component_controltype)
      : Input(variables, name, units::UnitType::one, units::UnitType::one), component_name_(component_name),
        component_type_(component_type), component_controltype_(component_controltype),
        handle_([this](EnergyPlus::EnergyPlusData &data) {
          return energyplus::ActuatorHandle(data, component_type_, component_controltype_, component_name_);
        })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Schedule";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void Schedule::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetActuatorValue(energyplus_data, handle_.get(energyplus_data), *v);
    }
  }
} // namespace other

namespace surface {
  void A::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("zoneSurfaces", std::vector<json>(0));

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");
      Variables::CreateOne<A>(variables, surface_name);
    }
  }

  A::A(Variables &variables, const std::string_view surface_name)
      : Parameter(variables, std::string(surface_name) + "_A", units::UnitType::m2, units::UnitType::m2),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Area of the surface that is exposed to the thermal zone";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Area";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void A::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::SurfaceArea(energyplus_data, surface_num_.get(energyplus_data)),
                       spawn::units::UnitSystem::EP);
  }

  void QFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("zoneSurfaces", std::vector<json>(0));

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");
      Variables::CreateOne<QFlow>(variables, surface_name);
    }
  }

  QFlow::QFlow(Variables &variables, const std::string_view surface_name)
      : Output(variables, std::string(surface_name) + "_Q_flow", units::UnitType::W, units::UnitType::W),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") =
        "Net heat flow rate from the thermal zone to the surface, consisting of convective heat flow, absorbed "
        "solar radiation, absorbed infrared radiation minus emitted infrared radiation.";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::SurfaceInsideHeatFlow(energyplus_data, surface_num_.get(energyplus_data)),
                       spawn::units::UnitSystem::EP);
  }

  void T::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("zoneSurfaces", std::vector<json>(0));

    const std::string actuator_type("Surface");
    const std::string actuator_controltype("Surface Inside Temperature");

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");

      Variables::CreateOne<T>(variables, surface_name);
    }
  }

  T::T(Variables &variables, const std::string_view surface_name)
      : Input(variables, std::string(surface_name) + "_T", units::UnitType::C, units::UnitType::K),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Temperature of the surface.";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void T::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetInsideSurfaceTemperature(energyplus_data, surface_num_.get(energyplus_data), *v);
    }
  }
} // namespace surface

namespace construction {
  void A::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("buildingSurfaceDetailed", std::vector<json>(0));

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");
      Variables::CreateOne<A>(variables, surface_name);
    }
  }

  A::A(Variables &variables, const std::string_view surface_name)
      : Parameter(variables, std::string(surface_name) + "_A", units::UnitType::m2, units::UnitType::m2),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Area of the surface that is exposed to the thermal zone";
    scalar_variable.append_attribute("causality") = "calculatedParameter";
    scalar_variable.append_attribute("variability") = "fixed";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Area";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void A::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::SurfaceArea(energyplus_data, surface_num_.get(energyplus_data)),
                       spawn::units::UnitSystem::EP);
  }

  void QFrontFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("buildingSurfaceDetailed", std::vector<json>(0));

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");
      Variables::CreateOne<QFrontFlow>(variables, surface_name);
    }
  }

  QFrontFlow::QFrontFlow(Variables &variables, const std::string_view surface_name)
      : Output(variables, std::string(surface_name) + "_QFront_flow", units::UnitType::W, units::UnitType::W),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") =
        "Net heat flow rate from the thermal zone to the front-facing surface, consisting of convective heat flow,"
        " absorbed solar radiation,"
        " absorbed infrared radiation minus emitted infrared radiation.";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QFrontFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::SurfaceInsideHeatFlow(energyplus_data, surface_num_.get(energyplus_data)),
                       spawn::units::UnitSystem::EP);
  }

  void QBackFlow::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("buildingSurfaceDetailed", std::vector<json>(0));

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");
      Variables::CreateOne<QBackFlow>(variables, surface_name);
    }
  }

  QBackFlow::QBackFlow(Variables &variables, std::string_view surface_name)
      : Output(variables, std::string(surface_name) + "_QBack_flow", units::UnitType::W, units::UnitType::W),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") =
        "Net heat flow rate to the back-facing surface. If coupled to another thermal zone or the outside, this "
        "consist of convective heat flow, absorbed solar radiation, absorbed infrared radiation minus emitted "
        "infrared radiation. If coupled to the ground, this consists of the heat flow rate from the ground.";
    scalar_variable.append_attribute("causality") = "output";
    scalar_variable.append_attribute("variability") = "continuous";
    scalar_variable.append_attribute("initial") = "calculated";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void QBackFlow::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    Variable::SetValue(energyplus::SurfaceOutsideHeatFlow(energyplus_data, surface_num_.get(energyplus_data)),
                       spawn::units::UnitSystem::EP);
  }

  void TFront::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("buildingSurfaceDetailed", std::vector<json>(0));

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");
      Variables::CreateOne<TFront>(variables, surface_name);
    }
  }

  TFront::TFront(Variables &variables, const std::string_view surface_name)
      : Input(variables, std::string(surface_name) + "_TFront", units::UnitType::C, units::UnitType::K),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Temperature of the front-facing surface.";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TFront::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetInsideSurfaceTemperature(energyplus_data, surface_num_.get(energyplus_data), *v);
    }
  }

  void TBack::CreateAll(const UserConfig &user_config, Variables &variables)
  {
    const auto surfaces =
        user_config.spawnjson.value("model", json::object()).value("buildingSurfaceDetailed", std::vector<json>(0));

    for (const auto &surface : surfaces) {
      const auto surface_name = surface.value("name", "");
      Variables::CreateOne<TBack>(variables, surface_name);
    }
  }

  TBack::TBack(Variables &variables, const std::string_view surface_name)
      : Input(variables, std::string(surface_name) + "_TBack", units::UnitType::C, units::UnitType::K),
        surface_name_(surface_name),
        surface_num_([this](EnergyPlus::EnergyPlusData &data) { return energyplus::SurfaceNum(data, surface_name_); })
  {
    auto scalar_variable = metadata_.append_child("ScalarVariable");
    scalar_variable.append_attribute("name") = name_.c_str();
    scalar_variable.append_attribute("valueReference") = std::to_string(index_).c_str();
    scalar_variable.append_attribute("description") = "Temperature of the back-facing surface.";
    scalar_variable.append_attribute("causality") = "input";
    scalar_variable.append_attribute("variability") = "continuous";

    auto real = scalar_variable.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("unit") = units::toString(mo_unit_).c_str();
  }

  void TBack::Update(EnergyPlus::EnergyPlusData &energyplus_data)
  {
    if (const auto v = Value(units::UnitSystem::EP)) {
      energyplus::SetOutsideSurfaceTemperature(energyplus_data, surface_num_.get(energyplus_data), *v);
    }
  }
} // namespace construction

} // namespace spawn::variable
