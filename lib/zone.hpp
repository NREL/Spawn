#ifndef Zone_hh_INCLUDED
#define Zone_hh_INCLUDED

#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"

namespace spawn {

class Zone {
public:
  static std::vector<Zone> createZones(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf);

  std::string idfname;
  // Does the zone (defined in idf) have a connection to Modelica?
  bool isconnected;

  double volume() const;
  double floorArea() const;

private:
  Zone(std::string idfname, bool isconnected);
};

} // namespace spawn

#endif // Zone_hh_INCLUDED

