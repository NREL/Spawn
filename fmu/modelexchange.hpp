//
// Created by jason on 5/21/20.
//

#ifndef SPAWN_MODELEXCHANGE_HPP
#define SPAWN_MODELEXCHANGE_HPP

#include "fmu.hpp"
#include <string>

namespace spawn::fmu {

/// A FMU file used for modelexchange
/// loads the FMU file, extracts the FMI dynamic library from it
/// and initializes the FMI2 in ModelExchange mode
class ModelExchange
{
public:
  /// Todo: Validate that the provided fmu_file is valid for ModelExchange
  ModelExchange(const spawn_fs::path &fmu_file, const std::string &name, const bool visible, const bool loggingOn)
      : fmu{fmu_file, false}, m_unzippedResources{fmu_file, m_resourcesDirectory.dir(), {}}
  {
    auto resourcesUri = fmt::format("file://{}/resources", m_resourcesDirectory.dir().generic_string());

    const auto &guid = fmu.modelDescription().child("fmiModelDescription").attribute("guid").value();

    component = fmu.fmi.fmi2Instantiate(
        name.c_str(), fmi2Type::fmi2ModelExchange, guid, resourcesUri.c_str(), nullptr, visible, loggingOn);
  }

  template <typename Value> void setVariable(const std::string_view &name, const Value &value)
  {
    setVariable(fmu.getVariableByName(name), value);
  }

  /// sets a fmi2Real value, throws exception if the variable type does not match
  void setVariable(const FMU::Variable &variable, const double value);
  /// sets a fmi2Integer value, throws exception if the variable type does not match
  void setVariable(const FMU::Variable &variable, const int value);
  /// sets a fmi2String value, throws exception if the variable type does not match
  void setVariable(const FMU::Variable &variable, const char *value);
  /// sets a fmi2Boolean value, throws exception if the variable type does not match
  void setVariable(const FMU::Variable &variable, const bool value);

  /// gets a fmi2Boolean value, throws exception if the variable type does not match
  /// object is passed by reference to match calling semantics of the fmi2 interface
  void getVariable(const FMU::Variable &variable, double &value) const;

  /// gets a fmi2Integer value, throws exception if the variable type does not match
  /// object is passed by reference to match calling semantics of the fmi2 interface
  void getVariable(const FMU::Variable &variable, int &value) const;

  /// gets a fmi2String value, throws exception if the variable type does not match
  /// object is passed by reference to match calling semantics of the fmi2 interface
  void getVariable(const FMU::Variable &variable, const char *&value) const;

  /// gets a fmi2Boolean value, throws exception if the variable type does not match
  /// object is passed by reference to match calling semantics of the fmi2 interface
  void getVariable(const FMU::Variable &variable, bool &value) const;

  /// templated getVariable for convenience that calls through to the type specific
  /// getVariable implementations.
  template <typename VariableType> [[nodiscard]] VariableType getVariable(std::string_view name) const
  {
    const auto &variable = fmu.getVariableByName(name);
    VariableType result;
    getVariable(variable, result);
    return result;
  }

  /// call the visitor `Func` with the appropriate type of the given variable object
  /// semantics are similar to std::visit for Variant
  template <typename Func> auto visitVariable(Func &&func, const FMU::Variable &variable)
  {
    switch (variable.type) {
    case FMU::Variable::Type::Integer: {
      int result{};
      getVariable(variable, result);
      return func(result);
    }
    case FMU::Variable::Type::Real: {
      double result{};
      getVariable(variable, result);
      return func(result);
    }
    case FMU::Variable::Type::String: {
      const char *result{};
      getVariable(variable, result);
      return func(result);
    }
    case FMU::Variable::Type::Boolean: {
      bool result{};
      getVariable(variable, result);
      return func(result);
    }
    }
    throw std::logic_error("Unknown variable type to visit.");
  }

  FMU fmu;
  fmi2Component component{};

private:
  util::Temp_Directory m_resourcesDirectory;
  util::Unzipped_File m_unzippedResources;
};
} // namespace spawn::fmu

#endif // SPAWN_MODELEXCHANGE_HPP
