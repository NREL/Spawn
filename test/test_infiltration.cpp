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

TEST_CASE("Test Infiltration")
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
        "outputVariables": [
          {{
            "name":    "Zone Mean Air Temperature",
            "key":     "Thermal Zone 1",
            "fmiName": "zone 1 temp"
          }},
          {{
            "name":    "Zone Mean Air Temperature",
            "key":     "Thermal Zone 2",
            "fmiName": "zone 2 temp"
          }}
        ],
        "emsActuators": [
          {{
            "variableName"  : "THERMAL ZONE 1 189.1-2009 - OFFICE - WHOLEBUILDING - MD OFFICE - CZ4-8 INFILTRATION",
            "componentType" : "Zone Infiltration",
            "controlType"   : "Air Exchange Flow Rate",
            "unit"          : "m3/s",
            "fmiName"       : "zone 1 infiltration"
          }},
          {{
            "variableName"  : "THERMAL ZONE 2 189.1-2009 - OFFICE - WHOLEBUILDING - MD OFFICE - CZ4-8 INFILTRATION 1",
            "componentType" : "Zone Infiltration",
            "controlType"   : "Air Exchange Flow Rate",
            "unit"          : "m3/s",
            "fmiName"       : "zone 2 infiltration"
          }}
        ]
      }}
    }}
  )", fmt::arg("idfpath", two_zones_idf_path().generic_string()), fmt::arg("epwpath", chicago_epw_path().generic_string()));

  const auto fmu_file_path = create_epfmu(spawn_input_string);
  spawn::fmu::FMU fmu{fmu_file_path, false}; // don't require all symbols
  REQUIRE(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuStdOutLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

  fmi2Status status; 

  status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  REQUIRE(status == fmi2OK);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  constexpr std::array<const char*, 4> variable_names{
    "zone 1 temp",
    "zone 2 temp",
    "zone 1 infiltration",
    "zone 2 infiltration"
  };

  std::map<std::string, fmi2ValueReference> variable_refs;
  for (const auto name : variable_names) {
    variable_refs[name] = modelDescription.valueReference(name);
  }

  const auto zone_1_temp_ref = modelDescription.valueReference("zone 1 temp");
  const auto zone_2_temp_ref = modelDescription.valueReference("zone 2 temp");
  const auto zone_1_infiltration_ref = modelDescription.valueReference("zone 1 infiltration");
  const auto zone_2_infiltration_ref = modelDescription.valueReference("zone 2 infiltration");

  const std::array<fmi2ValueReference, 2> input_refs = {zone_1_infiltration_ref, zone_2_infiltration_ref};
  const std::array<fmi2ValueReference, 2> output_refs = {zone_1_temp_ref, zone_2_temp_ref};

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  std::array<fmi2Real, output_refs.size()> output_values;
  std::array<fmi2Real, input_refs.size()> input_values;

  // Initially the zones should have the same temperature
  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  CHECK( output_values[0] > spawn::c_to_k(10.0) );
  CHECK( output_values[1] > spawn::c_to_k(10.0) );
  CHECK( output_values[0] < spawn::c_to_k(30.0) );
  CHECK( output_values[1] < spawn::c_to_k(30.0) );
  CHECK( output_values[1] < spawn::c_to_k(30.0) );
  CHECK( std::abs(output_values[0] - output_values[1]) < 0.1 );

  // After "turning off" infiltration and simulating for a day, the zones should still have the same temperature
  input_values[0] = 0.0;
  input_values[1] = 0.0;
  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(1));
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  CHECK( output_values[0] > spawn::c_to_k(10.0) );
  CHECK( output_values[1] > spawn::c_to_k(10.0) );
  CHECK( output_values[0] < spawn::c_to_k(30.0) );
  CHECK( output_values[1] < spawn::c_to_k(30.0) );
  CHECK( output_values[1] < spawn::c_to_k(30.0) );
  CHECK( std::abs(output_values[0] - output_values[1]) < 0.1 );

  // After setting different infiltration rates and simulating for a day, the zone temperatures should diverge
  input_values[0] = 0.0;
  input_values[1] = 0.1;
  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(2));
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  CHECK( output_values[0] > spawn::c_to_k(10.0) );
  CHECK( output_values[1] > spawn::c_to_k(10.0) );
  CHECK( output_values[0] < spawn::c_to_k(30.0) );
  CHECK( output_values[1] < spawn::c_to_k(30.0) );
  CHECK( output_values[1] < spawn::c_to_k(30.0) );
  // It is winter, and zone 2 has cold air infiltrating the zone,
  // so zone 2 is colder than zone 1
  CHECK( std::abs(output_values[0] - output_values[1]) > 1.0 );

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}

