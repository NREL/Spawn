//
// Created by jason on 5/21/20.
//

#ifndef SPAWN_MODELEXCHANGE_HPP
#define SPAWN_MODELEXCHANGE_HPP

#include "fmu.hpp"
#include <string>


namespace spawn::fmu {
class ModelExchange
{
public:
  ModelExchange(const boost::filesystem::path &fmu_file,
                const std::string &name,
                const bool visible,
                const bool loggingOn)
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

  void setVariable(const FMU::Variable &variable, const double value);
  void setVariable(const FMU::Variable &variable, const int value);
  void setVariable(const FMU::Variable &variable, const char *value);
  void setVariable(const FMU::Variable &variable, const bool value);

  void getVariable(const FMU::Variable &variable, double &value);
  void getVariable(const FMU::Variable &variable, int &value);
  void getVariable(const FMU::Variable &variable, const char *&value);
  void getVariable(const FMU::Variable &variable, bool &value);

  template <typename VariableType>[[nodiscard]] VariableType getVariable(std::string_view name)
  {
    const auto &variable = fmu.getVariableByName(name);
    VariableType result;
    getVariable(variable, result);
    return result;
  }

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
  }

  FMU fmu;
  fmi2Component component{};

private:
  util::Temp_Directory m_resourcesDirectory;
  util::Unzipped_File m_unzippedResources;
};
}

#endif // SPAWN_MODELEXCHANGE_HPP
