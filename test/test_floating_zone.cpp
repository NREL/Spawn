#include "../fmu/fmu.hpp"
#include "../fmu/modeldescription.hpp"
#include "../fmu/logger.h"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "paths.hpp"
#include "create_epfmu.hpp"
#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

TEST_CASE("Test SingleFamilyHouse with Floating Zone")
{
  const auto fmu_file_path = create_single_family_house_fmu();
  spawn::fmu::FMU fmu{fmu_file_path, false}; // don't require all symbols
  REQUIRE(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuNothingLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

  fmi2Status status; 

  status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  constexpr std::array<const char*, 1> variable_names{
    "GARAGE ZONE Temp"
  };

  std::map<std::string, fmi2ValueReference> variable_refs;
  for (const auto name : variable_names) {
    variable_refs[name] = modelDescription.valueReference(name);
  }

  const std::array<fmi2ValueReference, 1> output_refs = {
    variable_refs["GARAGE ZONE Temp"]
  };

  std::array<fmi2Real, output_refs.size()> output_values;
  double time = 0.0;
  fmi2EventInfo info;

  while (time <= 60.0 * 60 * 24 * 365) {
    status = fmu.fmi.fmi2SetTime(comp, time);
    CHECK(status == fmi2OK);

    status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
    CHECK(status == fmi2OK);

    // Check that zone temp is sane
    CHECK(output_values[0] > spawn::c_to_k(-10.0));
    CHECK(output_values[0] < spawn::c_to_k(35.0));

    status = fmu.fmi.fmi2NewDiscreteStates(comp, &info);
    CHECK(status == fmi2OK);
    time = info.nextEventTime;
  }

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}


