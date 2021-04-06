#include "../fmu/fmu.hpp"
#include "../fmu/modeldescription.hpp"
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
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  fmi2Status status;

  const auto core_zn_wall_east_t_front_ref = modelDescription.valueReference("Core_ZN_wall_east_TFront");
  const auto core_zn_wall_east_t_back_ref = modelDescription.valueReference("Core_ZN_wall_east_TBack");
  fmi2ValueReference temp_vr[] = {core_zn_wall_east_t_front_ref, core_zn_wall_east_t_back_ref};
  fmi2Real temp[] = {293.15,293.15};
  status = fmu.fmi.fmi2SetReal(comp, temp_vr, 2, temp);
  CHECK(status == fmi2OK);

  fmu.fmi.fmi2ExitInitializationMode(comp);

  // This is an interzone surface
  // Need tests for other surface types
  const auto core_zn_wall_east_q_front_ref = modelDescription.valueReference("Core_ZN_wall_east_QFront_flow");
  const auto core_zn_wall_east_q_back_ref = modelDescription.valueReference("Core_ZN_wall_east_QBack_flow");
  fmi2ValueReference q_vr[] = {core_zn_wall_east_q_front_ref, core_zn_wall_east_q_back_ref};
  fmi2Real q[2];

  for (int day = 1; day <= 8; ++day) {
    fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(day));
    status = fmu.fmi.fmi2GetReal(comp, q_vr, 2, q);
    CHECK(status == fmi2OK);
  }

  fmu.fmi.fmi2Terminate(comp);
}

