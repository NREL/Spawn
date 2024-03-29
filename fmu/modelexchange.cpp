//
// Created by jason on 5/21/20.
//

#include "modelexchange.hpp"

namespace spawn::fmu {

void ModelExchange::setVariable(const FMU::Variable &variable, const double value)
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::Real);
  fmu.fmi.fmi2SetReal(component, &variable.valueReference, 1U, &value);
}

void ModelExchange::setVariable(const FMU::Variable &variable, const int value)
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::Integer);
  fmu.fmi.fmi2SetInteger(component, &variable.valueReference, 1U, &value);
}

void ModelExchange::setVariable(const FMU::Variable &variable, const char *value)
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::String);
  fmu.fmi.fmi2SetString(component, &variable.valueReference, 1U, &value);
}

void ModelExchange::setVariable(const FMU::Variable &variable, const bool value)
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::Boolean);
  const auto fmi2val = static_cast<fmi2Boolean>(value);
  fmu.fmi.fmi2SetBoolean(component, &variable.valueReference, 1U, &fmi2val);
}

void ModelExchange::getVariable(const FMU::Variable &variable, double &value) const
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::Real);
  fmi2Real fmi2val{};
  fmu.fmi.fmi2GetReal(component, &variable.valueReference, 1U, &fmi2val);
  value = fmi2val;
}

void ModelExchange::getVariable(const FMU::Variable &variable, int &value) const
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::Integer);
  fmi2Integer fmi2val{};
  fmu.fmi.fmi2GetInteger(component, &variable.valueReference, 1U, &fmi2val);
  value = fmi2val;
}

void ModelExchange::getVariable(const FMU::Variable &variable, const char *&value) const
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::Boolean);
  fmi2String fmi2val{};
  fmu.fmi.fmi2GetString(component, &variable.valueReference, 1U, &fmi2val);
  value = fmi2val;
}

void ModelExchange::getVariable(const FMU::Variable &variable, bool &value) const
{
  // validate throws with default parameters
  [[maybe_unused]] const auto result = variable.validate(FMU::Variable::Type::Boolean);
  fmi2Boolean fmi2val{};
  fmu.fmi.fmi2GetBoolean(component, &variable.valueReference, 1U, &fmi2val);
  value = static_cast<bool>(fmi2val);
}
} // namespace spawn::fmu
