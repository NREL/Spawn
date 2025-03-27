#include "../coroutine/spawn.hpp"
#include "../util/config.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <filesystem>

using json = nlohmann::json;

const auto idfpath = // NOLINT
    spawn::project_source_dir() / "energyplus/testfiles/RefBldgSmallOfficeNew2004_Chicago.idf";
const auto epwpath = // NOLINT
    spawn::project_source_dir() / "energyplus/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

TEST_CASE("Test Zone Sizing Variables")
{
  const std::string spawn_input = fmt::format(
      R"({{
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
    }})",
      fmt::arg("idfpath", idfpath.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, spawn::test::get_current_test_dir());

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

TEST_CASE("Test Zone Group Sizing Variables")
{
  const std::string spawn_input = fmt::format(
      R"({{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}",
        "autosize": true
      }},
      "model": {{
        "zones": [
           {{ "name": "Core_ZN" }}
        ],
        "hvacZones": [{{
          "name": "conditioned_zones",
          "zones": [
           {{ "name": "Core_ZN" }},
           {{ "name": "Perimeter_ZN_1" }},
           {{ "name": "Perimeter_ZN_2" }},
           {{ "name": "Perimeter_ZN_3" }},
           {{ "name": "Perimeter_ZN_4" }}
          ]
        }}]
      }}
    }})",
      fmt::arg("idfpath", idfpath.generic_string()),
      fmt::arg("epwpath", epwpath.generic_string()));

  spawn::Spawn spawn1("spawn1", spawn::test::idd_path(), spawn_input, spawn::test::get_current_test_dir());

  spawn1.start();
  CHECK(spawn1.currentTime() == 0.0);

  double value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_QCooSen_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_QCooLat_flow");
  // The Sizing:Zone input for "Zone Load Sizing Method" defaults to "Sensible Load Only No Latent Load",
  // and most common IDFs use this option, therefore we will not have latent loads reported.
  CHECK(value == Approx(0.0));
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_TOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_XOutCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_mOutCoo_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_TCoo");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_QHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_TOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_XOutHea");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_mOutHea_flow");
  CHECK(value > 0.0);
  value = spawn1.GetValue("hvac_sizing_group_conditioned_zones_THea");
  CHECK(value > 0.0);

  spawn1.stop();
}
