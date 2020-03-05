#ifndef Variables_hh_INCLUDED
#define Variables_hh_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <EnergyPlus.hh>

//namespace EnergyPlus {
//namespace FMI {

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
  EMS_ACTUATOR
};

using VariableAttribute = std::pair<std::string, std::string>;

struct Variable {
  VariableType type;
  std::string key;
  Real64 value;
  bool valueset; // This is true "value" member has been set to a value
  std::vector<VariableAttribute> scalar_attributes;
  std::vector<VariableAttribute> real_attributes;

  // This is used for VariableType::OUTPUT which corresponds to an E+ output var
  std::string epname;
  std::string epkey;
};

std::map<unsigned int, Variable> parseVariables(const std::string & idf,
  const std::string & jsonInput
);

//}
//}

#endif // Variables_hh_INCLUDED

