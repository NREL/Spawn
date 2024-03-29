#ifndef Variables_hh_INCLUDED
#define Variables_hh_INCLUDED

#include "units.hpp"
#include <map>
#include <sstream>
#include <string>
#include <vector>

// See variable documentation here
// https://lbl-srg.github.io/soep/softwareArchitecture.html#coupling-of-the-envelope-model
enum class VariableType
{
  T,
  V,
  AFLO,
  MSENFAC,
  QCONSEN_FLOW,
  QGAIRAD_FLOW,
  QLAT_FLOW,
  QPEO_FLOW,
  TAVEINLET,
  TRAD,
  X,
  MINLETS_FLOW,
  SENSOR,
  EMS_ACTUATOR,
  SCHEDULE,
  ASURF,
  TSURF,
  QSURF,
  TSURF_FRONT,
  QSURF_FRONT,
  TSURF_BACK,
  QSURF_BACK,
};

using VariableAttribute = std::pair<std::string, std::string>;

namespace spawn {
namespace units {
  enum class UnitSystem;
}

class Input;
} // namespace spawn

class Variable
{
public:
  VariableType type;
  std::string name;
  spawn::units::UnitType epunittype{spawn::units::UnitType::one};
  spawn::units::UnitType mounittype{spawn::units::UnitType::one};

  // This is true "value" member has been set to a value
  // If valueset is false then value should not be used
  void setValue(const double &value, const spawn::units::UnitSystem &system);
  [[nodiscard]] double getValue(const spawn::units::UnitSystem &unitsystem) const;
  void resetValue();
  [[nodiscard]] bool isValueSet() const;

  std::vector<VariableAttribute> scalar_attributes;
  std::vector<VariableAttribute> real_attributes;

  // This is used for VariableType::OUTPUT
  std::string outputvarname;
  std::string outputvarkey;

  // This is used for VariableType::EMS_ACTUATOR
  std::string actuatorcomponentkey;
  std::string actuatorcomponenttype;
  std::string actuatorcontroltype;

private:
  // Value stored using the Modelica unit system
  double value{};
  bool valueset{false};
};

std::map<unsigned int, Variable> parseVariables(const spawn::Input &input);

#endif // Variables_hh_INCLUDED
