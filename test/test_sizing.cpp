#include "../energyplus_coroutine/spawn.hpp"
#include "../util/config.hpp"
#include "../util/temp_directory.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const auto idfpath = // NOLINT
    spawn::project_source_dir() / "energyplus/testfiles/RefBldgSmallOfficeNew2004_Chicago.idf";
const auto epwpath = // NOLINT
    spawn::project_source_dir() / "energyplus/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

const std::string spawn_input = fmt::format( // NOLINT
    R"(
  {{
    "version": "0.1",
    "EnergyPlus": {{
      "idf": "{idfpath}",
      "weather": "{epwpath}",
      "autosize": true
    }},
    "model": {{
      "zones": [
         {{ "name": "Core_ZN" }}
      ]
    }}
  }}
)",
    fmt::arg("idfpath", idfpath.generic_string()),
    fmt::arg("epwpath", epwpath.generic_string()));

TEST_CASE("Test Sizing Variables")
{
  spawn::util::Temp_Directory working_path{};
  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, working_path.dir());
  spawn1.start();
  CHECK(spawn1.currentTime() == 0.0);

  double value = spawn1.GetValue("Core_ZN_QCooSen_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_QCooLat_flow");
  // The Sizing:Zone input for "Zone Load Sizing Method" defaults to "Sensible Load Only No Latent Load",
  // and most common IDFs use this option, therefore we will not have latent loads reported.
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("Core_ZN_TOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_XOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_mOutCoo_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_TCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_QHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_TOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_XOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_mOutHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("Core_ZN_THea");
  CHECK(value > 0.0);

  spawn1.stop();
}
