#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "create_epfmu.hpp"
#include "paths.hpp"

#include <array>
#include <catch2/catch.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

TEST_CASE("Test Zone Capacitance Multiplier")
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
          "name": "ZoneCapacitanceMultiplier.fmu",
          "version": "2.0",
          "kind"   : "ME"
      }},
      "model": {{
        "zones": [
           {{ "name": "Perimeter_ZN_1" }},
           {{ "name": "Perimeter_ZN_2" }}
        ]
      }}
    }}
  )",
      fmt::arg("idfpath",
               (testdir() / "models/RefBldgSmallOfficeNew2004_Chicago_ZoneCapacitanceMultiplier.idf").string()),
      fmt::arg("epwpath", chicago_epw_path().string()));

  const auto fmu_file_path = create_epfmu(spawn_input_string);
  spawn::fmu::FMU fmu{fmu_file_path, false}; // don't require all symbols
  REQUIRE(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  std::cout << "resource_path: " << resource_path << std::endl;
  fmi2CallbackFunctions callbacks = {
      fmuNothingLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

  fmi2Status status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  REQUIRE(status == fmi2OK);

  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  // Begin test mSenFac
  // Perimeter_ZN_1_mSenFac
  const auto zn_1_msenfac_ref = modelDescription.valueReference("Perimeter_ZN_1_mSenFac");
  const auto zn_2_msenfac_ref = modelDescription.valueReference("Perimeter_ZN_2_mSenFac");

  const std::array<fmi2ValueReference, 2> output_refs = {zn_1_msenfac_ref, zn_2_msenfac_ref};

  status = fmu.fmi.fmi2ExitInitializationMode(comp);
  REQUIRE(status == fmi2OK);

  std::array<fmi2Real, output_refs.size()> output_values{};

  status = fmu.fmi.fmi2GetReal(comp, output_refs.data(), output_refs.size(), output_values.data());
  CHECK(status == fmi2OK);
  // Perimeter_ZN_1 is designed to have twice as much thermal capacity due to
  // ZoneCapacitanceMultiplier:ResearchSpecial input
  CHECK(output_values[0] / output_values[1] == Approx(2.0));

  status = fmu.fmi.fmi2Terminate(comp);
  REQUIRE(status == fmi2OK);
}
