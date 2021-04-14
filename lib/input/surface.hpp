#ifndef Surface_hh_INCLUDED
#define Surface_hh_INCLUDED

#include "../../submodules/EnergyPlus/third_party/nlohmann/json.hpp"

namespace spawn {

class Surface {
public:
  static std::vector<Surface> createSurfaces(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf);

  const std::string idfname;

  // ControlType indicates if there is 1. no FMU control,
  // 2. control of the inside face,
  // or 3. control of both inside and outside face
  enum class ControlType {None, Front, FrontBack};
  const ControlType controltype{ControlType::None};
private:
  Surface(std::string idfname, ControlType controltype);
};

} // namespace spawn

#endif // Surface_hh_INCLUDED

