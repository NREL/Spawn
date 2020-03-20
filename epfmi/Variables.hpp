#ifndef Variables_hh_INCLUDED
#define Variables_hh_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <EnergyPlus.hh>

enum class VariableType {
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
  SCHEDULE
};

using VariableAttribute = std::pair<std::string, std::string>;

struct Variable {
  VariableType type{};
  std::string name;

  Real64 value{};
  bool valueset{false}; // This is true "value" member has been set to a value

  std::vector<VariableAttribute> scalar_attributes;
  std::vector<VariableAttribute> real_attributes;

  // This is used for VariableType::OUTPUT
  std::string outputvarname;
  std::string outputvarkey;

  // This is used for VariableType::EMS_ACTUATOR
  std::string actuatorcomponentkey;
  std::string actuatorcomponenttype;
  std::string actuatorcontroltype;
};

std::map<unsigned int, Variable> parseVariables(const std::string & idf,
  const std::string & jsonInput
);

#endif // Variables_hh_INCLUDED

