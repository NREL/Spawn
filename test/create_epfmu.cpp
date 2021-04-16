#include "create_epfmu.hpp"
#include "paths.hpp"

fs::path create_epfmu()
{
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
