#include "zone.hpp"
#include <iostream>

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

  if ( jsonidf.find(type) != jsonidf.end() ) {
    const auto idfzones = jsonidf[type];
    const auto & buildzone = [&](const std::string & idfname) {
      Zone zone(idfname, false);
      return zone;
    };
    for( const auto & idfzone : idfzones.items() ) {
      result.emplace_back(buildzone(idfzone.key()));
    }
  }

  const auto modelicazones = spawnjson.value("model",json()).value("zones", std::vector<json>(0));
  for(const auto & modelicazone : modelicazones) {
    const auto zit = std::find_if(std::begin(result), std::end(result),
        [&](const Zone & zone) {
          return (zone.idfname == modelicazone.value("name",""));
        }
    );
    if(zit != std::end(result)) {
      zit->isconnected = true;
    }
  }

  return result;
}

} // namespace spawn

