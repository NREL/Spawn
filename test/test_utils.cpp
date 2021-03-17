#include "../util/paths.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <iostream>
#include <fmt/format.h>

TEST_CASE("Test exe")
{
  const auto exe = spawn::exe();

  CHECK(fs::is_regular_file(exe));
  CHECK(!fs::is_symlink(exe));
  CHECK(exe.is_absolute());

  std::cout << spawn::exe() << '\n';
}

TEST_CASE("Test exe with different working directory")
{
  const auto old_working_dir = fs::current_path();
  fs::current_path("/");

  CHECK(old_working_dir != fs::current_path());

  const auto exe = spawn::exe();

  CHECK(fs::is_regular_file(exe));
  CHECK(!fs::is_symlink(exe));
  CHECK(exe.is_absolute());

  std::cout << spawn::exe() << '\n';
}


TEST_CASE("Test exedir")
{
  const auto exedir = spawn::exedir();

  CHECK(fs::is_directory(exedir));
  CHECK(!fs::is_symlink(exedir));
  CHECK(exedir.is_absolute());

  std::cout << spawn::exedir() << '\n';
}

TEST_CASE("Test exedir with different working directory")
{
  const auto old_working_dir = fs::current_path();
  fs::current_path(fs::temp_directory_path());

  CHECK(old_working_dir != fs::current_path());

  const auto exedir = spawn::exedir();

  CHECK(fs::is_directory(exedir));
  CHECK(!fs::is_symlink(exedir));
  CHECK(exedir.is_absolute());

  std::cout << spawn::exedir() << '\n';
}

TEST_CASE("Test calling exedir on self with bad exe name")
{
  const auto exe_name = spawn::exe().filename();
  fs::current_path(spawn::exedir());
  const auto results = system(fmt::format(R"(./{}asdf "Test blerb")", exe_name.native()).c_str());
  CHECK(results != 0);
}


TEST_CASE("Test calling exedir on relative path for myself")
{
  const auto exe_name = spawn::exe().filename();
  fs::current_path(spawn::exedir());
  const auto results = system(fmt::format(R"(./{} "Test exedir")", exe_name.native()).c_str());
  CHECK(results == 0);

  const auto results2 = system(fmt::format(R"(./{} "Test exedir with different working directory")", exe_name.native()).c_str());
  CHECK(results2 == 0);
}


