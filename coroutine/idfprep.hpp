#ifndef idfprep_hh_INCLUDED
#define idfprep_hh_INCLUDED

#include "../energyplus/third_party/nlohmann/json.hpp"
#include "start_time.hpp"

using json = nlohmann::json;

namespace spawn {
class UserConfig;

// Reduce the jsonidf down to the EnergyPlus features that Spawn depends on,
// and insert idf content that is required.
void prepare_idf(json &jsonidf, const UserConfig &user_config, const StartTime &start_time);

// Validate the jsonidf to ensure that the user input is not requesting something
// that Spawn does not support, such as zone multipliers.
// This function will throw an exception upon the first validation failure.
void validate_idf(json &jsonidf);
} // namespace spawn

#endif // idfprep_hh_INCLUDED
