#include "emsactuator.hpp"

using json = nlohmann::json;

namespace spawn {

EMSActuator::EMSActuator(std::string t_spawnname, std::string t_idfname, std::string t_idftype, std::string t_idfcontroltype) :
  spawnname(t_spawnname),
  idfname(t_idfname),
  idftype(t_idftype),
  idfcontroltype(t_idfcontroltype)
{
  std::transform(idfname.begin(), idfname.end(), idfname.begin(), ::toupper);
}

std::vector<EMSActuator> EMSActuator::createEMSActuators(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf) {
  std::vector<EMSActuator> result;

  const auto spawnActuators = spawnjson.value("model",json()).value("emsActuators", std::vector<json>(0));
  for(const auto & spawnActuator : spawnActuators) {
    const auto spawnname = spawnActuator.value("fmiName", "");
    const auto idfname = spawnActuator.value("variableName", "");
    const auto idftype = spawnActuator.value("componentType", "");
    const auto idfcontroltype = spawnActuator.value("controlType", "");

    const auto & build = [&]() {
      return EMSActuator(spawnname, idfname, idftype, idfcontroltype);
    };

    result.emplace_back(build());
  }

  return result;
}


} // namespace spawn
