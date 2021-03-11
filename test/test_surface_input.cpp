#include "../fmu/fmu.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "../fmu/logger.h"
#include "paths.hpp"
#include "create_epfmu.hpp"
#include <catch2/catch.hpp>
#include <iostream>

// Do nothing logger
inline void fmuLogger(fmi2ComponentEnvironment, fmi2String, fmi2Status, fmi2String, fmi2String, ...)
{
}

TEST_CASE("Test surface temperature input")
{
  const auto fmu_file = create_epfmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuNothingLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  // 21 is the value reference of the Core_ZN_Floor_T input as defined in the model description xml
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  fmi2ValueReference refs[1] = {21};
  fmi2Real vals[1] = {294.15};
  fmu.fmi.fmi2SetReal(comp, refs, 1, vals);
  fmu.fmi.fmi2ExitInitializationMode(comp);
  fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(365));
  fmu.fmi.fmi2Terminate(comp);
}

