#include "../lib/spawn.hpp"
#include "../util/filesystem.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Test a single instance of the Spawn class")
{
  // Path to a spawn json input file
  const auto spawn_input = testcase1();

  spawn::Spawn spawn1("spawn1", spawn_input);
  spawn1.start();
  CHECK(spawn1.currentTime() == 0.0);
  spawn1.setTime(1000.0);
  CHECK(spawn1.currentTime() == 1000.0);
  spawn1.stop();
}
