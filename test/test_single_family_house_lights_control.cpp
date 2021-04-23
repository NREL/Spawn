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

TEST_CASE("Test SingleFamilyHouse Lights")
{
  std::string spawn_input_string = fmt::format(
  R"(
    {{
      "version": "0.1",
      "EnergyPlus": {{
        "idf": "{idfpath}",
        "weather": "{epwpath}",
        "relativeSurfaceTolerance": 1.0e-10
      }},
      "fmu": {{
          "name": "MyBuilding.fmu",
          "version": "2.0",
          "kind"   : "ME"
      }},
      "model": {{
        "zones": [
           {{ "name": "LIVING ZONE" }}
        ],
        "schedules": [
           {{
              "name": "HOUSE LIGHTING",
              "unit": "1",
              "fmiName": "lighting input"
           }}
        ],
        "outputVariables": [
          {{
            "name":    "Lights Electricity Rate",
            "key":     "LIVING ZONE Lights",
            "fmiName": "lighting output"
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

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  constexpr std::array<const char*, 4> variable_names{
    "LIVING ZONE_T",
    "LIVING ZONE_QConSen_flow",
    "lighting input",
    "lighting output"
  };

  std::map<std::string, fmi2ValueReference> variable_refs;
  for (const auto name : variable_names) {
    variable_refs[name] = modelDescription.valueReference(name);
  }

  const auto lighting_input_ref = modelDescription.valueReference("lighting input");
  const auto lighting_output_ref = modelDescription.valueReference("lighting output");

  const std::array<fmi2ValueReference, 1> input_refs = {lighting_input_ref};
  const std::array<fmi2ValueReference, 1> output_refs = {lighting_output_ref};

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  std::array<fmi2Real, output_refs.size()> output_values;
  std::array<fmi2Real, input_refs.size()> input_values;

  input_values[0] = 0.0;
  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  CHECK( output_values[0] == Approx(0.0) );

  input_values[0] = 1.0;
  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  CHECK( output_values[0] == Approx(1000.0) );

  input_values[0] = 0.5;
  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  CHECK( output_values[0] == Approx(500.0) );

  input_values[0] = 0.0;
  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  CHECK( output_values[0] == Approx(0.0) );

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}

