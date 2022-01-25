#include "../lib/spawn.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "../util/temp_directory.hpp"
#include "../util/config.hpp"
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const auto idfpath = spawn::project_source_dir() / "submodules/EnergyPlus/testfiles/RefBldgSmallOfficeNew2004_Chicago.idf";
const auto epwpath = spawn::project_source_dir() / "submodules/EnergyPlus/weather/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

std::string spawn_input = fmt::format(
R"(
  {{
    "version": "0.1",
    "EnergyPlus": {{
      "idf": "{idfpath}",
      "weather": "{epwpath}"
    }},
    "model": {{
      "outputVariables": [
        {{
          "name":    "Lights Electricity Rate",
          "key":     "Core_ZN_Lights",
          "fmiName": "Core_Zone_Lights_Output"
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
)", fmt::arg("idfpath", idfpath.generic_string()), fmt::arg("epwpath", epwpath.generic_string()));

TEST_CASE("Test one Spawn")
{
  spawn::util::Temp_Directory working_path{};
  spawn::Spawn spawn1("spawn1", spawn_input, working_path.dir());
  spawn1.start();
  CHECK(spawn1.currentTime() == 0.0);

  for(int day = 0; day <= 365; ++day) {
    auto time = spawn::days_to_seconds(day);
    spawn1.setTime(time);
    CHECK(spawn1.currentTime() == time);
    const auto lighting_power = spawn1.getValue("Core_Zone_Lights_Output");
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

  for(int day = 0; day <= 365; ++day) {
    auto time = spawn::days_to_seconds(day);
    spawn1.setTime(time);
    spawn2.setTime(time);
    CHECK(spawn1.currentTime() == time);
    CHECK(spawn2.currentTime() == time);

    const auto lighting_power1 = spawn1.getValue("Core_Zone_Lights_Output");
    const auto lighting_power2 = spawn2.getValue("Core_Zone_Lights_Output");
    const auto all_lighting_power = lighting_power1 + lighting_power2;
    CHECK(all_lighting_power > 0.0);
  }

  spawn1.stop();
  spawn2.stop();
}
