#ifndef idf_to_json_hh_INCLUDED
#define idf_to_json_hh_INCLUDED

#include "../util/filesystem.hpp"
#include "energyplus/src/EnergyPlus/DataStringGlobals.hh"
#include "energyplus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "energyplus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "energyplus/src/EnergyPlus/UtilityRoutines.hh"
#include "energyplus/third_party/nlohmann/json.hpp"

namespace spawn {

[[nodiscard]] nlohmann::json idf_to_json(const spawn_fs::path &idfpath);

void json_to_idf(const nlohmann::json &jsonidf, const spawn_fs::path &idfpath);

} // namespace spawn

#endif // idf_to_json_hh_INCLUDED
