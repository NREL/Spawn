#include "../fmu/fmi2.hpp"
#include "../util/filesystem.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Test loading of FMI")
{
  const auto fmi_file = example_fmu_path();
  REQUIRE(fs::exists(fmi_file));
  REQUIRE(fs::is_regular_file(fmi_file));

  spawn::fmu::FMI2 fmi{example_fmu_path(), false};

  REQUIRE(fmi.fmi2GetVersion.has_value());


  CHECK(fmi.fmi2GetVersion() == std::string("TEST_VERSION"));

  CHECK(fmi.loadResults().successes.size() == 1);
  CHECK(fmi.loadResults().failures.size() == 43);
}

TEST_CASE("Test loading of FMI missing symbols")
{
  const auto fmi_file = example_fmu_path();
  REQUIRE(fs::exists(fmi_file));
  REQUIRE(fs::is_regular_file(fmi_file));

  REQUIRE_THROWS(spawn::fmu::FMI2{example_fmu_path(), true});
}

