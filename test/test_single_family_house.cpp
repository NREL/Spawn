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

TEST_CASE("Test SingleFamilyHouse")
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
        "emsActuators": [
          {{
            "variableName"  : "LIVING ZONE People",
            "componentType" : "People",
            "controlType"   : "Number of People",
            "unit"          : "1",
            "fmiName"       : "living zone people"
          }}
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
  std::cout << "resource_path: " << resource_path << std::endl;
  fmi2CallbackFunctions callbacks = {fmuNothingLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

  fmi2Status status; 

  status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  REQUIRE(status == fmi2OK);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  // Begin test lighting
  const auto lighting_input_ref = modelDescription.valueReference("lighting input");
  const auto lighting_output_ref = modelDescription.valueReference("lighting output");

  const std::array<fmi2ValueReference, 1> lighting_input_refs = {lighting_input_ref};
  const std::array<fmi2ValueReference, 1> lighting_output_refs = {lighting_output_ref};

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  std::array<fmi2Real, lighting_output_refs.size()> lighting_output_values;
  std::array<fmi2Real, lighting_input_refs.size()> lighting_input_values;

  lighting_input_values[0] = 0.0;
  status = fmu.fmi.fmi2SetReal(comp, lighting_input_refs.data(), lighting_input_refs.size(), lighting_input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, lighting_output_refs.data(), lighting_output_refs.size(), lighting_output_values.data());
  CHECK(status == fmi2OK);
  CHECK( lighting_output_values[0] == Approx(0.0) );

  lighting_input_values[0] = 1.0;
  status = fmu.fmi.fmi2SetReal(comp, lighting_input_refs.data(), lighting_input_refs.size(), lighting_input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, lighting_output_refs.data(), lighting_output_refs.size(), lighting_output_values.data());
  CHECK(status == fmi2OK);
  CHECK( lighting_output_values[0] == Approx(1000.0) );

  lighting_input_values[0] = 0.5;
  status = fmu.fmi.fmi2SetReal(comp, lighting_input_refs.data(), lighting_input_refs.size(), lighting_input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, lighting_output_refs.data(), lighting_output_refs.size(), lighting_output_values.data());
  CHECK(status == fmi2OK);
  CHECK( lighting_output_values[0] == Approx(500.0) );

  lighting_input_values[0] = 0.0;
  status = fmu.fmi.fmi2SetReal(comp, lighting_input_refs.data(), lighting_input_refs.size(), lighting_input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, lighting_output_refs.data(), lighting_output_refs.size(), lighting_output_values.data());
  CHECK(status == fmi2OK);
  CHECK( lighting_output_values[0] == Approx(0.0) );

  // Begin test latent heat
  const auto qlat_flow_ref = modelDescription.valueReference("LIVING ZONE_QLat_flow");
  const auto qpeo_flow_ref = modelDescription.valueReference("LIVING ZONE_QPeo_flow");
  const auto people_input_ref = modelDescription.valueReference("living zone people");
  const std::array<fmi2ValueReference, 2> latent_output_refs = {qlat_flow_ref, qpeo_flow_ref};
  const std::array<fmi2ValueReference, 1> latent_input_refs = {people_input_ref};

  std::array<fmi2Real, latent_output_refs.size()> latent_output_values;
  std::array<fmi2Real, latent_input_refs.size()> latent_input_values;

  status = fmu.fmi.fmi2GetReal(comp, latent_output_refs.data(), latent_output_refs.size(), latent_output_values.data());
  CHECK(status == fmi2OK);
  // QLAT_FLOW should be zero
  CHECK( latent_output_values[0] == Approx(0.0) );
  // QPEO_FLOW is also zero
  CHECK( latent_output_values[1] == Approx(0.0) );

  latent_input_values[0] = 5.0;
  status = fmu.fmi.fmi2SetReal(comp, latent_input_refs.data(), latent_input_refs.size(), latent_input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, latent_output_refs.data(), latent_output_refs.size(), latent_output_values.data());
  CHECK(status == fmi2OK);
  // QLAT_FLOW should be non zero
  CHECK( latent_output_values[0] > 10.0 );
  // QPEO_FLOW is also non zero
  CHECK( latent_output_values[1] > 10.0 );

  // QLAT and QPEO for 5 people should be reasonable
  CHECK( latent_output_values[0] < 1000.0 );
  CHECK( latent_output_values[1] < 1000.0 );

  // Ratio of latent heat, should be reasonable
  const auto latent_fraction = latent_output_values[0] / latent_output_values[1];
  CHECK( latent_fraction < 0.7 );
  CHECK( latent_fraction > 0.3 );

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}

