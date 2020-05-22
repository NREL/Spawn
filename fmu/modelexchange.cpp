//
// Created by jason on 5/21/20.
//

#include "modelexchange.hpp"

namespace spawn::fmu {

void ModelExchange::setVariable(const FMU::Variable &variable, const double value)
{
  variable.validate(FMU::Variable::Type::Real);
  fmu.fmi.fmi2SetReal(component, &variable.valueReference, 1, &value);
}

void ModelExchange::setVariable(const FMU::Variable &variable, const int value)
{
  variable.validate(FMU::Variable::Type::Integer);
  fmu.fmi.fmi2SetInteger(component, &variable.valueReference, 1, &value);
}

void ModelExchange::setVariable(const FMU::Variable &variable, const char *value)
{
  variable.validate(FMU::Variable::Type::String);
  fmu.fmi.fmi2SetString(component, &variable.valueReference, 1, &value);
}

void ModelExchange::setVariable(const FMU::Variable &variable, const bool value)
{
  variable.validate(FMU::Variable::Type::Boolean);
  fmi2Boolean fmi2val = static_cast<fmi2Boolean>(value);
  fmu.fmi.fmi2SetBoolean(component, &variable.valueReference, 1, &fmi2val);
}

void ModelExchange::getVariable(const FMU::Variable &variable, double &value)
{
  variable.validate(FMU::Variable::Type::Real);
  fmi2Real fmi2val{};
  fmu.fmi.fmi2GetReal(component, &variable.valueReference, 1, &fmi2val);
  value = fmi2val;
}

void ModelExchange::getVariable(const FMU::Variable &variable, int &value)
{
  variable.validate(FMU::Variable::Type::Integer);
  fmi2Integer fmi2val{};
  fmu.fmi.fmi2GetInteger(component, &variable.valueReference, 1, &fmi2val);
  value = fmi2val;
}

void ModelExchange::getVariable(const FMU::Variable &variable, const char *&value)
{
  variable.validate(FMU::Variable::Type::Boolean);
  fmi2String fmi2val{};
  fmu.fmi.fmi2GetString(component, &variable.valueReference, 1, &fmi2val);
  value = fmi2val;
}

void ModelExchange::getVariable(const FMU::Variable &variable, bool &value)
{
  variable.validate(FMU::Variable::Type::Boolean);
  fmi2Boolean fmi2val{};
  fmu.fmi.fmi2GetBoolean(component, &variable.valueReference, 1, &fmi2val);
  value = static_cast<bool>(fmi2val);
}
}
