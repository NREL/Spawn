#include "../fmu/fmi2.hpp"
#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include "../util/config.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "paths.hpp"
#include "util/env_vars.hpp"
#include <catch2/catch.hpp>

#if defined ENABLE_COMPILER

TEST_CASE("Test Zone Multiplier", "[.]")
{
  spawn::set_env("SPAWNPATH", spawnexe().parent_path());

  const std::string model_name = "Zone_Multiplier";
  const std::string fmu_name = "Zone_Multiplier.fmu";
  const auto model_dir = spawn::project_binary_dir() / "mbl/test/models/Zone_Multiplier.mo";

  const auto cmd = spawnexe().string() + " modelica --modelica-path " + model_dir.string() +
                   " create-fmu --fmu-type ME " + model_name;
  const auto result = system(cmd.c_str()); // NOLINT
  REQUIRE(result == 0);

  spawn::fmu::FMU fmu{fmu_name, false};

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
  CHECK(status == fmi2Error);
}

#endif // ENABLE_COMPILER
