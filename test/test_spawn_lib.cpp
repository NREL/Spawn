#include "../lib/spawn.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "../util/temp_directory.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Test one Spawn")
{
  // Path to a spawn json input file
  const auto spawn_input = testcase1();

  spawn::util::Temp_Directory working_path{};
  spawn::Spawn spawn1("spawn1", spawn_input, working_path.dir());
  spawn1.start();
  CHECK(spawn1.currentTime() == 0.0);

  for(int day = 0; day <= 365; ++day) {
    auto time = spawn::days_to_seconds(day);
    spawn1.setTime(time);
    CHECK(spawn1.currentTime() == time);
  }
  spawn1.stop();
}

//TEST_CASE("Test two Spawns")
//{
//  // Path to a spawn json input file
//  const auto spawn_input = testcase1();
//
//  spawn::util::Temp_Directory working_path1{};
//  spawn::util::Temp_Directory working_path2{};
//
//  spawn::Spawn spawn1("spawn1", spawn_input, working_path1.dir());
//  spawn::Spawn spawn2("spawn2", spawn_input, working_path2.dir());
//
//  spawn1.start();
//  spawn2.start();
//
//  CHECK(spawn1.currentTime() == 0.0);
//  CHECK(spawn2.currentTime() == 0.0);
//
//  for(int day = 0; day <= 365; ++day) {
//    auto time = spawn::days_to_seconds(day);
//    spawn1.setTime(time);
//    spawn2.setTime(time);
//    CHECK(spawn1.currentTime() == time);
//    CHECK(spawn2.currentTime() == time);
//  }
//
//  spawn1.stop();
//  spawn2.stop();
//}
