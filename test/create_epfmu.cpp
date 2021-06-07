#include "create_epfmu.hpp"
#include "paths.hpp"
#include "../util/paths.hpp"
#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"
#include <fmt/format.h>
#include <fstream>

using json = nlohmann::json;

fs::path single_family_house_idf_path() {
  return spawn::project_source_dir() / "submodules/modelica-buildings/Buildings/Resources/Data/ThermalZones/EnergyPlus/Examples/SingleFamilyHouse_TwoSpeed_ZoneAirBalance/SingleFamilyHouse_TwoSpeed_ZoneAirBalance.idf";
}

fs::path two_zones_idf_path() {
  return spawn::project_source_dir() / "submodules/modelica-buildings/Buildings/Resources/Data/ThermalZones/EnergyPlus/Validation/TwoIdenticalZones/TwoIdenticalZones.idf";
}

fs::path chicago_epw_path() {
  return  spawn::project_source_dir() / "submodules/modelica-buildings/Buildings/Resources/weatherdata/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";
}

fs::path create_epfmu() {
  // testcase1 is the RefBldgSmallOfficeNew2004_Chicago
  // This call generates an FMU for the corresponding idf file
  // testcase1() returns a path to RefBldgSmallOfficeNew2004_Chicago.spawn
  // which is a json file that configures the spawn input
  const auto cmd = spawnexe() + " --create " + testcase1() + " --no-compress --output-dir " + testdir().string();
  const auto result = system(cmd.c_str());
  if (result != 0) {
    throw std::runtime_error("Error creating FMU, non-0 result");
  }
  return testdir() / "MyBuilding.fmu";
}

fs::path create_epfmu(const std::string & input_string) {
  auto input = json::parse(input_string);
  const auto fmuname = input.value("fmu", json()).value("name", "MyBuilding.fmu");
  fs::path fmupath(fmuname);

  const auto spawn_input_path = testdir() / (fmupath.stem().string() + ".json");
  std::ofstream spawn_input_file(spawn_input_path);
  spawn_input_file << input_string << std::endl;
  spawn_input_file.close();

  const auto fmu_file_path = testdir() / fmupath;
  const auto cmd = spawnexe() + " --create " + spawn_input_path.generic_string() + " --no-compress --output-path " + fmu_file_path.generic_string();
  const auto result = system(cmd.c_str());
  if (result) {
    throw std::runtime_error("Error creating FMU, non-0 result");
  }

  return fmu_file_path;
}

fs::path create_single_family_house_fmu() {
  const auto idfpath = single_family_house_idf_path();
  const auto epwpath = chicago_epw_path();

  std::string spawn_input_string = fmt::format(
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
        "buildingSurfaceDetailed": [
           {{ "name"    : "Living:Ceiling" }},
           {{ "name"    : "Living:South" }},
           {{ "name"    : "Attic:LivingFloor" }}
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
  )", fmt::arg("idfpath", idfpath.generic_string()), fmt::arg("epwpath", epwpath.generic_string()));

  return create_epfmu(spawn_input_string);
}
