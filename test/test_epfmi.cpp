#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "create_epfmu.hpp"
#include "paths.hpp"
#include <catch2/catch.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

TEST_CASE("Test loading of Spawn Generated FMU")
{
  const auto fmu_file = create_epfmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));
}

TEST_CASE("Test one year simulation")
{
  const auto fmu_file = create_epfmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {
      fmuNothingLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  auto *const comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  fmu.fmi.fmi2ExitInitializationMode(comp);
  fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(365));
  fmu.fmi.fmi2Terminate(comp);
}

TEST_CASE("Test two year simulation")
{
  const auto fmu_file = create_epfmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {
      fmuNothingLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  fmu.fmi.fmi2ExitInitializationMode(comp);
  fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(365 * 2));
  fmu.fmi.fmi2Terminate(comp);
}

TEST_CASE("Test invalid input")
{
  const auto fmu_file = create_epfmu();
  spawn::fmu::FMU fmu{fmu_file, false};
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));
  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();

  fmi2CallbackFunctions callbacks = {
      fmuNothingLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

  fmi2Status status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  CHECK(status == fmi2OK);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);
  const auto core_zn_t_ref = modelDescription.valueReference("Core_ZN_T");
  std::array<fmi2ValueReference, 1> vr{core_zn_t_ref};
  // 0.0 actually does not halt EnergyPlus,
  // but slightly below absolute zero and EnergyPlus fails. lala land.
  // The fmi2SetReal function will return ok, because there is minimal validation
  std::array<fmi2Real, 1> v{-10.0};
  status = fmu.fmi.fmi2SetReal(comp, vr.data(), 1U, v.data());
  CHECK(status == fmi2OK);

  // But as soon as EnergyPlus iterates which happens upon fmi2ExitInitializationMode
  // an error is returned.
  // Also there will be a log "EnergyPlus is not running"
  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2Error);

  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(365));
  REQUIRE(status == fmi2Error);

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2Error);
}
