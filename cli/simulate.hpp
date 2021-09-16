#include <nlohmann/json.hpp>
#include "../util/filesystem.hpp"

namespace spawn {

void simulate(const fs::path & fmupath, const nlohmann::json options = {});

} // namespace spawn
