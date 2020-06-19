#include "testpaths.hpp"
#include "spawn.hpp"
#include <catch2/catch.hpp>
#include <cstdlib>
#include <boost/filesystem.hpp>


boost::filesystem::path create_fmu()
{
  // testcase1 is the RefBldgSmallOfficeNew2004_Chicago
  // This call generates an FMU for the corresponding idf file
  // testcase1() returns a path to RefBldgSmallOfficeNew2004_Chicago.spawn
  // which is a json file that configures the spawn input
  const auto cmd = spawnexe() + " --create " + testcase1() + " --output-dir " + testdir().string();
  const auto result = system(cmd.c_str());
  if (result != 0) {
    throw std::runtime_error("Error creating FMU, non-0 result");
  }
  return testdir() / "MyBuilding.fmu";
}

TEST_CASE( "Spawn shows help" ) {
  const auto cmd = spawnexe() + " --help";
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}

// This is the main requirement of spawn executable,
// generate an FMU for a given EnergyPlus model
TEST_CASE( "Spawn creates an FMU" ) {
  boost::filesystem::path created_fmu;
  REQUIRE_NOTHROW(created_fmu = create_fmu());
  CHECK(boost::filesystem::is_regular_file(created_fmu));
  CHECK(boost::filesystem::file_size(created_fmu) > 0);
}

/**
TEST_CASE( "Spawn simulates an FMU" ) {
  // Well it can't actually do this yet. People are using
  // JModelica, Dymola, or pyfmi to simulate the spawn FMU,
  // but there is work underway to bake an installer in directly
  const auto cmd = spawnexe() + " --simulate " + testcase1();
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}

 This is not a supported feature, but it is desirable
 and not a lot of additional effort to have spawn generate an
 FMU for co simulation. This is equivalent to the existing energyplus
 external interface feature. Not a requirement of the spawn archicture
 because spawn is based on FMU for model exchange.
TEST_CASE( "Spawn creates an FMU for co simulation" ) {
  const auto cmd = spawnexe() + "--create-cosim " + testcase1();
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}

 Only works on linux right now, because it involves packaging
 a complicated compiler toolchain
TEST_CASE( "Spawn compiles modelica files" ) {
  const auto cmd = spawnexe() + " --compile " + model1();
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}
**/

