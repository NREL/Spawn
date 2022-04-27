#include "../fmu/simulate.hpp"
#include "../util/config.hpp"
#include "../util/math.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>

#if defined ENABLE_MODELICA_COMPILER
TEST_CASE("Simulate class is able to run a simulation")
{
  const auto create = spawnexe() + " modelica --create-fmu " + "Buildings.ThermalZones." +
                      spawn::mbl_energyplus_version_string() + ".Examples.SingleFamilyHouse.AirHeating --fmu-type CS";
  auto result = system(create.c_str()); // NOLINT
  REQUIRE(result == 0);

  const std::string fmu_name =
      "Buildings_ThermalZones_" + spawn::mbl_energyplus_version_string() + "_Examples_SingleFamilyHouse_AirHeating.fmu";
  const auto fmu_path = spawn::project_binary_dir() / fmu_name;

  spawn::fmu::Sim sim(fmu_path);
  nlohmann::json config;
  config["start"] = 0;
  config["stop"] = spawn::days_to_seconds(30);
  config["step"] = 60;
  sim.run(config);
}
#endif
