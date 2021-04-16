#include "surface.hpp"
#include <iostream>
#include <fmt/format.h>

using json = nlohmann::json;

namespace spawn {

Surface::Surface(std::string t_idfname, bool t_isconnected) :
  idfname(std::move(t_idfname)),
  isconnected(std::move(t_isconnected))
{
}

std::vector<Surface> Surface::createSurfaces(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf) {
  std::vector<Surface> result;
  constexpr auto & type = "BuildingSurface:Detailed";
  const auto modelicasurfaces = spawnjson.value("model",json()).value("zoneSurfaces", std::vector<json>(0));

  auto getIsConnected = [&](const std::string & idfname) {
    const auto zit = std::find_if(std::begin(modelicasurfaces), std::end(modelicasurfaces),
        [&](const nlohmann::json & modelicasurface) {
          return (idfname == modelicasurface.value("name",""));
        }
    );
    if(zit != std::end(modelicasurfaces)) {
      return true;
    }

    return false;
  };

  auto buildsurface = [&](const std::string & idfname) {
    Surface surface(idfname, getIsConnected(idfname));
    return surface;
  };

  if ( jsonidf.find(type) != jsonidf.end() ) {
    const auto idfsurfaces = jsonidf[type];

    for( const auto & idfsurface : idfsurfaces.items() ) {
      result.emplace_back(buildsurface(idfsurface.key()));
    }
  }

  return result;
}

} // namespace spawn

