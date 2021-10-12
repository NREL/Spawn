#include "zone.hpp"
#include <iostream>
#include <fmt/format.h>

using json = nlohmann::json;

namespace spawn {


Zone::Zone(std::string t_idfname, bool t_isconnected) :
  idfname(std::move(t_idfname)),
  isconnected(std::move(t_isconnected))
{
}

std::vector<Zone> Zone::createZones(const nlohmann::json & spawnjson, const nlohmann::json & jsonidf) {
  std::vector<Zone> result;
  constexpr auto & type = "Zone";
  const auto modelicazones = spawnjson.value("model",json()).value("zones", std::vector<json>(0));

  auto getIsConnected = [&](const std::string & idfname) {
    const auto zit = std::find_if(std::begin(modelicazones), std::end(modelicazones),
        [&](const nlohmann::json & modelicazone) {
          return (idfname == modelicazone.value("name",""));
        }
    );
    if(zit != std::end(modelicazones)) {
      return true;
    }

    return false;
  };

  auto buildzone = [&](const std::string & idfname) {
    Zone zone(idfname, getIsConnected(idfname));
    return zone;
  };

  if ( jsonidf.find(type) != jsonidf.end() ) {
    const auto idfzones = jsonidf[type];

    for( const auto & idfzone : idfzones.items() ) {
      result.emplace_back(buildzone(idfzone.key()));
    }
  }

  return result;
}

} // namespace spawn

