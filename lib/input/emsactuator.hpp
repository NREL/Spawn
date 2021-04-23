#ifndef emsactuator_hh_INCLUDED
#define emsactuator_hh_INCLUDED

#include "../../submodules/EnergyPlus/third_party/nlohmann/json.hpp"

namespace spawn {

class EMSActuator {
public:
  [[nodiscard]] static std::vector<EMSActuator> createEMSActuators(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf);

  std::string spawnname;
  std::string idfname;
  std::string idftype;
  std::string idfcontroltype;

private:
  EMSActuator(std::string t_spawnname, std::string t_idfname, std::string t_idftype, std::string t_idfcontroltype) noexcept;
};

} // namespace spawn

#endif // emsactuator_hh_INCLUDED
