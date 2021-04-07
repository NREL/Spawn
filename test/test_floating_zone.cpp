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
  std::string spawn_input_string = fmt::format(
  R"(
    {{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}"
      }},
      "fmu": {{
          "name": "MyBuilding.fmu",
          "version": "2.0",
          "kind"   : "ME"
      }},
      "model": {{
        "outputVariables": [
          {{
            "name":    "Zone Mean Air Temperature",
            "key":     "GARAGE ZONE",
            "fmiName": "GARAGE ZONE Temp"
          }},
          {{
            "name":    "Zone Mean Air Temperature",
            "key":     "LIVING ZONE",
            "fmiName": "LIVING ZONE Temp"
          }},
          {{
            "name":    "Site Outdoor Air Drybulb Temperature",
            "key":     "Environment",
            "fmiName": "Outside Temp"
          }}

        ]
      }}
    }}
  )", fmt::arg("idfpath", single_family_house_idf_path().generic_string()), fmt::arg("epwpath", chicago_epw_path().generic_string()));

  const auto fmu_file_path = create_epfmu(spawn_input_string);
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

  constexpr std::array<const char*, 3> variable_names{
    "LIVING ZONE Temp",
    "GARAGE ZONE Temp",
    "Outside Temp"
  };

  std::map<std::string, fmi2ValueReference> variable_refs;
  for (const auto name : variable_names) {
    variable_refs[name] = modelDescription.valueReference(name);
  }

  const std::array<fmi2ValueReference, 3> output_refs = {
    variable_refs["LIVING ZONE Temp"],
    variable_refs["GARAGE ZONE Temp"],
    variable_refs["Outside Temp"]
  };

  std::array<fmi2Real, output_refs.size()> output_values;
  double time = 0.0;
  fmi2EventInfo info;

  while (time <= 60.0 * 60 * 24 * 365) {
    status = fmu.fmi.fmi2SetTime(comp, time);
    CHECK(status == fmi2OK);

    status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
    CHECK(status == fmi2OK);

    const auto living_temp = output_values[0];
    const auto garage_temp = output_values[1];
    const auto outside_temp = output_values[2];

    // Check that zone temp is sane
    CHECK(std::abs(garage_temp - outside_temp) < 15.0);
    CHECK(std::abs(living_temp - outside_temp) < 28.0);
    CHECK(living_temp > spawn::c_to_k(-25.0));
    CHECK(living_temp < spawn::c_to_k(52.0));
    CHECK(garage_temp > spawn::c_to_k(-25.0));
    CHECK(garage_temp < spawn::c_to_k(52.0));

    status = fmu.fmi.fmi2NewDiscreteStates(comp, &info);
    CHECK(status == fmi2OK);
    time = info.nextEventTime;
  }

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}


