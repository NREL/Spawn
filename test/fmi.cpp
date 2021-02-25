#include "../fmu/fmu.hpp"
#include "../util/filesystem.hpp"
#include "testpaths.hpp"
#include "spawn.hpp"
#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Test loading of FMI")
{
  const auto fmi_file = fmi_load_test();
  REQUIRE(fs::exists(fmi_file));
  REQUIRE(fs::is_regular_file(fmi_file));

  spawn::fmu::FMI fmi{fmi_load_test(), false};

  REQUIRE(fmi.fmi2GetVersion.has_value());


  CHECK(fmi.fmi2GetVersion() == std::string("TEST_VERSION"));

  CHECK(fmi.loadResults().successes.size() == 1);
  CHECK(fmi.loadResults().failures.size() == 43);
}

TEST_CASE("Test loading of FMI missing symbols")
{
  const auto fmi_file = fmi_load_test();
  REQUIRE(fs::exists(fmi_file));
  REQUIRE(fs::is_regular_file(fmi_file));

  REQUIRE_THROWS(spawn::fmu::FMI{fmi_load_test(), true});
}

TEST_CASE("Test loading of Spawn Generated FMU")
{
  const auto fmu_file = create_fmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));
}

// Do nothing logger
void fmuLogger(fmi2ComponentEnvironment, fmi2String, fmi2Status, fmi2String, fmi2String, ...)
{
}

double days_to_seconds(const int days)
{
  const double seconds_per_minute = 60.0;
  const double minutes_per_hour = 60.0;
  const double hours_per_day = 24.0;
  return seconds_per_minute * minutes_per_hour * hours_per_day * days;
}

TEST_CASE("Test one year simulation")
{
  const auto fmu_file = create_fmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  fmu.fmi.fmi2SetTime(comp, days_to_seconds(365));
  fmu.fmi.fmi2Terminate(comp);
}

TEST_CASE("Test two year simulation")
{
  const auto fmu_file = create_fmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  fmu.fmi.fmi2SetTime(comp, days_to_seconds(365 * 2));
  fmu.fmi.fmi2Terminate(comp);
}

