#include "../fmu/fmu.hpp"
#include "testpaths.hpp"
#include "spawn.hpp"
#include <catch2/catch.hpp>

TEST_CASE("Test loading of FMI")
{
  const auto fmi_file = fmi_load_test();
  REQUIRE(boost::filesystem::exists(fmi_file));
  REQUIRE(boost::filesystem::is_regular(fmi_file));

  spawn::fmu::FMI2 fmi{fmi_load_test(), false};

  REQUIRE(fmi.fmi2GetVersion.has_value());


  CHECK(fmi.fmi2GetVersion() == std::string("TEST_VERSION"));

  CHECK(fmi.loadResults().successes.size() == 1);
  CHECK(fmi.loadResults().failures.size() == 43);
}

TEST_CASE("Test loading of FMI missing symbols")
{
  const auto fmi_file = fmi_load_test();
  REQUIRE(boost::filesystem::exists(fmi_file));
  REQUIRE(boost::filesystem::is_regular(fmi_file));

  REQUIRE_THROWS(spawn::fmu::FMI2{fmi_load_test(), true});
}

TEST_CASE("Test loading of Spawn Generated FMU")
{
  const auto fmu_file = create_fmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));
}