#include "fmu/simulate.hpp"
#include "mbl/config.hpp"
#include "paths.hpp"
#include "util/config.hpp"
#include "util/math.hpp"
#include <catch2/catch.hpp>

#if defined ENABLE_COMPILER
TEST_CASE("Simulate class is able to run a simulation", "[.]")
{
  const auto create = spawnexe().string() + " modelica create-fmu " + "Buildings.ThermalZones." +
                      spawn::mbl_energyplus_version_string() + ".Examples.SingleFamilyHouse.AirHeating --fmu-type CS";
  auto result = system(create.c_str()); // NOLINT
  REQUIRE(result == 0);

  const std::string fmu_name =
      "Buildings_ThermalZones_" + spawn::mbl_energyplus_version_string() + "_Examples_SingleFamilyHouse_AirHeating.fmu";
  const auto fmu_path = spawn::project_binary_dir() / fmu_name;

  std::cout << "fmu_path: " << fmu_path << std::endl;

  spawn::fmu::Simulate sim;
  sim.fmu_path = fmu_path;
  sim.start = 0;
  sim.stop = spawn::days_to_seconds(30);
  sim.step = 60;
  sim();
}
#endif
