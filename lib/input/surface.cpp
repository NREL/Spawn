#include "surface.hpp"
#include <fmt/format.h>
#include <iostream>

using json = nlohmann::json;

namespace spawn {

Surface::Surface(std::string t_idfname, ControlType t_controltype) noexcept
    : idfname(std::move(t_idfname)), controltype(std::move(t_controltype))
{
}

std::vector<Surface> Surface::createSurfaces(const nlohmann::json &spawnjson, const nlohmann::json &jsonidf)
{
  std::vector<Surface> result;
  constexpr auto &idftype = "BuildingSurface:Detailed";
  const auto front_control_surfaces = spawnjson.value("model", json()).value("zoneSurfaces", std::vector<json>(0));
  const auto frontback_control_surfaces =
      spawnjson.value("model", json()).value("buildingSurfaceDetailed", std::vector<json>(0));

  auto surface_control_type = [&](const std::string &idfname) {
    const auto pred = [&](const nlohmann::json &modelicasurface) {
      return (idfname == modelicasurface.value("name", ""));
    };

    const auto frontit = std::find_if(std::begin(front_control_surfaces), std::end(front_control_surfaces), pred);
    if (frontit != std::end(front_control_surfaces)) {
      return Surface::ControlType::Front;
    }

    const auto frontbackit =
        std::find_if(std::begin(frontback_control_surfaces), std::end(frontback_control_surfaces), pred);
    if (frontbackit != std::end(frontback_control_surfaces)) {
      return Surface::ControlType::FrontBack;
    }

    return Surface::ControlType::None;
  };

  auto buildsurface = [&](const std::string &idfname) {
    Surface surface(idfname, surface_control_type(idfname));
    return surface;
  };

  if (jsonidf.find(idftype) != jsonidf.end()) {
    const auto idfsurfaces = jsonidf[idftype];

    for (const auto &idfsurface : idfsurfaces.items()) {
      result.emplace_back(buildsurface(idfsurface.key()));
    }
  }

  return result;
}

} // namespace spawn
