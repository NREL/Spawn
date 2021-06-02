#include "../fmu/fmu.hpp"
#include "../fmu/modeldescription.hpp"
#include "../fmu/logger.h"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "paths.hpp"
#include "create_epfmu.hpp"
#include <catch2/catch.hpp>
#include <nlohmann/json.hpp>

#include <unistd.h>

TEST_CASE("Test example file", "[!hide]")
{
  const auto testfileDirectory = project_source_dir() / "submodules/EnergyPlus/testfiles";
  const auto simulationDirectory = project_binary_dir() / "testfile-sims";

  SECTION("clean simulation working directory") {
    fs::remove_all(simulationDirectory);
  }

  for (const auto & entry : fs::directory_iterator(testfileDirectory)) {
    const auto p = entry.path();

    if (p.extension() != ".idf") {
      continue;
    }

    if (p.filename().generic_string()[0] == '_') {
      continue;
    }

    SECTION(fmt::format("named: {}", p.filename().generic_string())) {
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
          }}
        }}
      )", fmt::arg("idfpath", p.generic_string()), fmt::arg("epwpath", chicago_epw_path().generic_string()));

      const auto fmu_file_path = create_epfmu(spawn_input_string);
      spawn::fmu::FMU fmu{fmu_file_path, false, simulationDirectory / p.stem().generic_string()}; // don't require all symbols
      REQUIRE(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

      const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
      fmi2CallbackFunctions callbacks = {fmuNothingLogger, calloc, free, NULL, NULL}; // called by the model during simulation
      const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

      fmi2Status status; 

      status = fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
      REQUIRE(status == fmi2OK);

      const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
      spawn::fmu::ModelDescription modelDescription(model_description_path);

      status = fmu.fmi.fmi2ExitInitializationMode(comp);
      REQUIRE(status == fmi2OK);

      status = fmu.fmi.fmi2Terminate(comp);
      REQUIRE(status == fmi2OK);
    }
  }
}

