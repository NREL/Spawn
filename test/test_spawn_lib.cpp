#include "../energyplus_coroutine/spawn.hpp"
#include "../util/config.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "../util/temp_directory.hpp"
#include "create_epfmu.hpp"
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <iostream>
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
      "weather": "{epwpath}"
    }},
    "model": {{
      "zones": [
         {{ "name": "Core_ZN" }}
      ],
      "outputVariables": [
        {{
          "name":    "Lights Electricity Rate",
          "key":     "Core_ZN_Lights",
          "fmiName": "Core_Zone_Lights_Output"
        }},
        {{
          "name":    "Zone Mean Air Temperature",
          "key":     "Core_ZN",
          "fmiName": "Core_ZN_Temp"
        }}
      ],
      "emsActuators": [
        {{
          "variableName"  : "Core_ZN People",
          "componentType" : "People",
          "controlType"   : "Number of People",
          "unit"          : "1",
          "fmiName"       : "Core_Zone_People"
        }}
      ]
    }}
  }}
)",
    fmt::arg("idfpath", idfpath.generic_string()),
    fmt::arg("epwpath", epwpath.generic_string()));

const std::string spawn_sfh_input = fmt::format( // NOLINT
    R"(
  {{
    "version": "0.1",
    "EnergyPlus": {{
      "idf": "{idfpath}",
      "weather": "{epwpath}"
    }},
    "fmu": {{
        "name": "MyBuilding.fmu",
        "version": "2.0",
        "kind"   : "ME"
    }},
    "model": {{
      "zones": [
         {{ "name": "LIVING ZONE" }},
         {{ "name": "ATTIC ZONE" }}
      ],
      "outputVariables": [
        {{
          "name":    "Zone Mean Air Temperature",
          "key":     "GARAGE ZONE",
          "fmiName": "GARAGE ZONE Temp"
        }}
      ]
    }}
  }}
)",
    fmt::arg("idfpath", single_family_house_idf_path().generic_string()),
    fmt::arg("epwpath", chicago_epw_path().generic_string()));

TEST_CASE("Test one Spawn")
{
  spawn::util::Temp_Directory working_path{};
  spawn::Spawn spawn1("spawn1", spawn_input, working_path.dir());
  spawn1.start();
  CHECK(spawn1.currentTime() == 0.0);

  for (int day = 0; day <= 365; ++day) {
    auto time = spawn::days_to_seconds(day);
    spawn1.setTime(time);
    CHECK(spawn1.currentTime() == time);
    const auto lighting_power = spawn1.GetValue("Core_Zone_Lights_Output");
    CHECK(lighting_power > 0.0);
  }
  spawn1.stop();
}

TEST_CASE("Test two Spawns")
{
  spawn::util::Temp_Directory working_path1{};
  spawn::util::Temp_Directory working_path2{};

  spawn::Spawn spawn1("spawn1", spawn_input, working_path1.dir());
  spawn::Spawn spawn2("spawn2", spawn_input, working_path2.dir());

  spawn1.start();
  spawn2.start();

  CHECK(spawn1.currentTime() == 0.0);
  CHECK(spawn2.currentTime() == 0.0);

  for (int day = 0; day <= 365; ++day) {
    auto time = spawn::days_to_seconds(day);
    spawn1.setTime(time);
    spawn2.setTime(time);
    CHECK(spawn1.currentTime() == time);
    CHECK(spawn2.currentTime() == time);

    const auto lighting_power1 = spawn1.GetValue("Core_Zone_Lights_Output");
    const auto lighting_power2 = spawn2.GetValue("Core_Zone_Lights_Output");
    const auto all_lighting_power = lighting_power1 + lighting_power2;
    CHECK(all_lighting_power > 0.0);
  }

  spawn1.stop();
  spawn2.stop();
}

TEST_CASE("Test negative start time")
{
  spawn::util::Temp_Directory working_path1{};
  spawn::util::Temp_Directory working_path2{};

  spawn::Spawn spawn1("spawn1", spawn_sfh_input, working_path1.dir());
  spawn::Spawn spawn2("spawn2", spawn_sfh_input, working_path2.dir());

  spawn1.setStartTime(spawn::days_to_seconds(364));
  spawn2.setStartTime(spawn::days_to_seconds(-1));

  spawn1.start();
  spawn2.start();

  auto seconds_in_day = spawn::days_to_seconds(1);

  for (int day = 0; day <= 365; ++day) {
    spawn1.setTime(spawn1.currentTime() + seconds_in_day);
    spawn2.setTime(spawn2.currentTime() + seconds_in_day);

    const auto zone_temp_1 = spawn1.GetValue("GARAGE ZONE Temp");
    const auto zone_temp_2 = spawn2.GetValue("GARAGE ZONE Temp");
    CHECK_THAT(zone_temp_1, Catch::Matchers::WithinAbs(zone_temp_2, 0.1));

    // const auto zone_heat_1 = spawn1.GetValue("LIVING ZONE_QConSen_flow");
    // const auto zone_heat_2 = spawn2.GetValue("LIVING ZONE_QConSen_flow");
    // CHECK(zone_heat_1 == Approx(zone_heat_2));
  }

  spawn1.stop();
  spawn2.stop();
}
