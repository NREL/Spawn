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

TEST_CASE("Test SingleFamilyHouse as FMU")
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

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);
  const auto core_zn_t_ref = modelDescription.valueReference("LIVING ZONE_T");
  const auto core_zone_q_ref = modelDescription.valueReference("LIVING ZONE_QConSen_flow");

  fmi2ValueReference input_vr[] = {core_zn_t_ref};
  fmi2Real input_v[] = {294.15};
  status = fmu.fmi.fmi2SetReal(comp, input_vr, 1, input_v);
  REQUIRE(status == fmi2OK);

  // fmi2GetReal will start energyplus
  fmi2ValueReference output_vr[] = {core_zone_q_ref};
  fmi2Real output_v[1];
  status = fmu.fmi.fmi2GetReal(comp, output_vr, 1, output_v);
  REQUIRE(status == fmi2OK);

  // Without the previous call to fmi2GetReal this would start energyplus
  // In this case it just returns ok
  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_vr, 1, output_v);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(2));
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_vr, 1, output_v);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}

TEST_CASE("Test SingleFamilyHouse as FMU with early stop")
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

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);
  const auto core_zn_t_ref = modelDescription.valueReference("LIVING ZONE_T");
  const auto core_zone_q_ref = modelDescription.valueReference("LIVING ZONE_QConSen_flow");

  fmi2ValueReference input_vr[] = {core_zn_t_ref};
  fmi2Real input_v[] = {294.15};
  status = fmu.fmi.fmi2SetReal(comp, input_vr, 1, input_v);
  REQUIRE(status == fmi2OK);

  fmi2ValueReference output_vr[] = {core_zone_q_ref};
  fmi2Real output_v[1];
  status = fmu.fmi.fmi2GetReal(comp, output_vr, 1, output_v);
  REQUIRE(status == fmi2OK);

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  // Test stopping early before simulation has started to advance
  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}

TEST_CASE("Test surface IO")
{
  const auto fmu_file = create_single_family_house_fmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuNothingLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  fmi2Status status;

  constexpr std::array<const char*, 14> variable_names{
    "ATTIC ZONE_T",
    "LIVING ZONE_T",
    "Living:Ceiling_TFront",
    "Living:Ceiling_TBack",
    "Living:South_TFront",
    "Living:South_TBack",
    "Attic:LivingFloor_TFront",
    "Attic:LivingFloor_TBack",
    "Living:Ceiling_QFront_flow",
    "Living:Ceiling_QBack_flow",
    "Living:South_QFront_flow",
    "Living:South_QBack_flow",
    "Attic:LivingFloor_QFront_flow",
    "Attic:LivingFloor_QBack_flow"
  };

  std::map<std::string, fmi2ValueReference> variable_refs;
  for (const auto name : variable_names) {
    variable_refs[name] = modelDescription.valueReference(name);
  }

  fmu.fmi.fmi2ExitInitializationMode(comp);

  const std::array<fmi2ValueReference, 4> input_refs = {
    variable_refs["ATTIC ZONE_T"],
    variable_refs["LIVING ZONE_T"],
    variable_refs["Living:Ceiling_TFront"],
    variable_refs["Living:Ceiling_TBack"]
  };

  const std::array<fmi2ValueReference, 4> output_refs = {
    variable_refs["Living:Ceiling_QFront_flow"],
    variable_refs["Living:Ceiling_QBack_flow"],
    variable_refs["Attic:LivingFloor_QFront_flow"],
    variable_refs["Attic:LivingFloor_QBack_flow"]
  };

  std::array<fmi2Real, output_refs.size()> output_values;

  // Test active heating surface
  const std::array<fmi2Real, input_refs.size()> heating_input_values = {
    spawn::c_to_k(0.0), // Cold attic
    spawn::c_to_k(15.0), // Chilly living space
    spawn::c_to_k(25.0), // Active heating living space ceiling
    spawn::c_to_k(10.0) // Some heat leakage to back of living space ceiling
  };

  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), heating_input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  // negative heat flow from living space to surface.
  // ie heating moving from surface to space.
  CHECK(output_values[0] < 0.0);
  // also negative heat flow from attic space to back of ceiling surface.
  CHECK(output_values[1] < 0.0);
  // Check that matching surfaces agree
  CHECK((output_values[1] - output_values[2]) == Approx(0.0));
  CHECK((output_values[0] - output_values[3]) == Approx(0.0));

  // Test active cooling surface
  const std::array<fmi2Real, input_refs.size()> cooling_input_values = {
    spawn::c_to_k(35.0), // Hot attic
    spawn::c_to_k(25.0), // Warm living space
    spawn::c_to_k(10.0), // Active cooling living space ceiling
    spawn::c_to_k(20.0) // Some heat leakage to back of living space ceiling
  };

  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), cooling_input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2SetTime(comp, 60.0 * 10.0 + std::numeric_limits<float>::epsilon());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  // positive heat flow from living space to surface.
  CHECK(output_values[0] > 0.0);
  // also positive heat flow from attic space to back of ceiling surface.
  CHECK(output_values[1] > 0.0);
  // Check that matching surfaces agree
  CHECK((output_values[1] - output_values[2]) == Approx(0.0));
  CHECK((output_values[0] - output_values[3]) == Approx(0.0));

  status = fmu.fmi.fmi2Terminate(comp);
  CHECK(status == fmi2OK);
}

TEST_CASE("Test invalid surface IO")
{
  // The Living:Floor exterior boundary condition references itself,
  // which Spawn does not yet support
  // We should expect errors if we access the exterior of the Living:Floor surface
  const auto idfpath = single_family_house_idf_path();
  const auto epwpath = chicago_epw_path();

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
        "buildingSurfaceDetailed": [
           {{ "name"    : "Living:Floor" }}
        ]
      }}
    }}
  )", fmt::arg("idfpath", idfpath.generic_string()), fmt::arg("epwpath", epwpath.generic_string()));

  const auto fmu_file = create_epfmu(spawn_input_string);

  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuNothingLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  fmi2Status status;

  constexpr std::array<const char*, 2> variable_names{
    "Living:Floor_TBack",
    "Living:Floor_QBack_flow"
  };

  std::map<std::string, fmi2ValueReference> variable_refs;
  for (const auto name : variable_names) {
    variable_refs[name] = modelDescription.valueReference(name);
  }

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  CHECK(status == fmi2Error);

  const std::array<fmi2ValueReference, 1> input_refs = {
    variable_refs["Living:Floor_TBack"]
  };

  const std::array<fmi2ValueReference, 1> output_refs = {
    variable_refs["Living:Floor_QBack_flow"],
  };

  std::array<fmi2Real, output_refs.size()> output_values;

  // Test active heating surface
  const std::array<fmi2Real, input_refs.size()> input_values = {
    spawn::c_to_k(21.0), // Neutral temperature boundary condition at ground
    // doesn't matter because we are expecting an error
  };

  status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
  CHECK(status == fmi2OK);

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2Error);

  status = fmu.fmi.fmi2Terminate(comp);
  CHECK(status == fmi2Error);
}

TEST_CASE("Test Idempotence")
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

  constexpr std::array<const char*, 2> variable_names{
    "LIVING ZONE_T",
    "LIVING ZONE_QConSen_flow",
  };

  std::map<std::string, fmi2ValueReference> variable_refs;
  for (const auto name : variable_names) {
    variable_refs[name] = modelDescription.valueReference(name);
  }

  const std::array<fmi2ValueReference, 1> output_refs = {
    variable_refs["LIVING ZONE_QConSen_flow"],
  };
  std::array<fmi2Real, output_refs.size()> output_values;

  const std::array<fmi2ValueReference, 1> input_refs = {
    variable_refs["LIVING ZONE_T"],
  };
  std::array<fmi2Real, input_refs.size()> input_values{spawn::c_to_k(21.0)};

  std::vector<double> living_heat_flows;
  for (size_t i = 0; i < 20; ++i) {
    status = fmu.fmi.fmi2SetReal(comp, input_refs.data(), input_refs.size(), input_values.data());
    CHECK(status == fmi2OK);
    status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
    CHECK(status == fmi2OK);
    const auto living_heat_flow = output_values[0];
    living_heat_flows.push_back(living_heat_flow);
  }

  const auto q_max = std::max_element(living_heat_flows.begin(), living_heat_flows.end());
  const auto q_min = std::min_element(living_heat_flows.begin(), living_heat_flows.end());
  const auto living_heat_flows_diff = *q_max - *q_min;
  CHECK(living_heat_flows_diff <= 1e-5);

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}

