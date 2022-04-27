#include "../fmu/fmi2.hpp"
#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include "../util/config.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <iostream>

TEST_CASE("Test loading of FMI")
{
  const auto fmi_file = example_fmu_path();
  REQUIRE(spawn_fs::exists(fmi_file));
  REQUIRE(spawn_fs::is_regular_file(fmi_file));

  spawn::fmu::FMI2 fmi{example_fmu_path(), false};

  REQUIRE(fmi.fmi2GetVersion.has_value());

  CHECK(fmi.fmi2GetVersion() == std::string("TEST_VERSION"));

  CHECK(fmi.loadResults().successes.size() == 1);
  CHECK(fmi.loadResults().failures.size() == 43);
}

TEST_CASE("Test loading of FMI missing symbols")
{
  const auto fmi_file = example_fmu_path();
  REQUIRE(spawn_fs::exists(fmi_file));
  REQUIRE(spawn_fs::is_regular_file(fmi_file));

  REQUIRE_THROWS(spawn::fmu::FMI2{example_fmu_path(), true});
}

#if defined ENABLE_MODELICA_COMPILER
TEST_CASE("Test Resetting a Spawn based FMU")
{
  const std::string model_name = "Buildings.ThermalZones." + spawn::mbl_energyplus_version_string() + ".Validation.ThermalZone.OneZoneOneYear";
  const std::string fmu_name = "Buildings_ThermalZones_" + spawn::mbl_energyplus_version_string() + "_Validation_ThermalZone_OneZoneOneYear.fmu";

  const auto cmd =
      spawnexe() +
      " modelica --create-fmu " +
      model_name +
      " --fmu-type ME";
  const auto result = system(cmd.c_str()); // NOLINT
  REQUIRE(result == 0);

  const auto fmu_path =
      spawn::project_binary_dir() / fmu_name;

  spawn::fmu::FMU fmu{fmu_path, false};
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));
  const auto resource_path = std::string("file://") + (fmu.extractedFilesPath() / "resources").string();

  spawn::fmu::ModelDescription modelDescription(fmu.extractedFilesPath() / fmu.modelDescriptionPath());

  fmi2CallbackFunctions callbacks = {
      fmuStdOutLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance",
                                            fmi2ModelExchange,
                                            modelDescription.guid().c_str(),
                                            resource_path.c_str(),
                                            &callbacks,
                                            false,
                                            true);

  // Run the model for a day
  fmi2Status status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2EnterInitializationMode(comp);
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(1));
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2Terminate(comp);
  CHECK(status == fmi2OK);

  // Reset
  status = fmu.fmi.fmi2Reset(comp);
  CHECK(status == fmi2OK);

  // Run the model again for a day
  status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2EnterInitializationMode(comp);
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(1));
  CHECK(status == fmi2OK);
  status = fmu.fmi.fmi2Terminate(comp);
  CHECK(status == fmi2OK);
}
#endif
