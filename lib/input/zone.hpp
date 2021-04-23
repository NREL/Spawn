#ifndef Zone_hh_INCLUDED
#define Zone_hh_INCLUDED

#include "../../submodules/EnergyPlus/third_party/nlohmann/json.hpp"

namespace spawn {

class Zone {
public:
  [[nodiscard]] static std::vector<Zone> createZones(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf);

  const std::string idfname;
  // Does the zone (defined in idf) have a connection to Modelica?
  const bool isconnected;
  // The name of the EnergyPlus object used to insert radiant gains
  // This is to support the QGAIRAD_FLOW spawn input
  // An OtherEquipment energyplus object is added by this name
  const std::string ep_qgairad_flow_object_name;

  static constexpr auto ep_qgairad_flow_object_type{"OtherEquipment"};
  static constexpr auto ep_qgairad_flow_object_controltype{"Power Level"};

private:
  Zone(std::string idfname, bool isconnected);
};

} // namespace spawn

#endif // Zone_hh_INCLUDED

