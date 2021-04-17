#ifndef Surface_hh_INCLUDED
#define Surface_hh_INCLUDED

#include "../../submodules/EnergyPlus/third_party/nlohmann/json.hpp"

namespace spawn {

class Surface {
public:
  [[nodiscard]] static std::vector<Surface> createSurfaces(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf);

  const std::string idfname;
  // Does the surface (defined in idf) have a connection to Modelica?
  const bool isconnected;

private:
  Surface(std::string idfname, bool isconnected) noexcept;
};

} // namespace spawn

#endif // Surface_hh_INCLUDED

