#ifndef idfprep_hh_INCLUDED
#define idfprep_hh_INCLUDED

#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"

using json = nlohmann::json;

namespace spawn {
class Input;

void prepare_idf(json &jsonidf, const Input &input);
} // namespace spawn

#endif // idfprep_hh_INCLUDED
