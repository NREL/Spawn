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
  const auto idfpath = project_source_dir() / "submodules/modelica-buildings/Buildings/Resources/Data/ThermalZones/EnergyPlus/Examples/SingleFamilyHouse_TwoSpeed_ZoneAirBalance/SingleFamilyHouse_TwoSpeed_ZoneAirBalance.idf";
  const auto epwpath = project_source_dir() / "submodules/modelica-buildings/Buildings/Resources/weatherdata/USA_IL_Chicago-OHare.Intl.AP.725300_TMY3.epw";

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
        "zones": [
          {{ "name": "LIVING ZONE" }}
        ]
      }}
    }}
  )", fmt::arg("idfpath", idfpath.generic_string()), fmt::arg("epwpath", epwpath.generic_string()));

  const auto spawn_input_path = testdir() / "SingleFamilyHouse.json";
  std::ofstream spawn_input_file(spawn_input_path);
  spawn_input_file << spawn_input_string << std::endl;
  spawn_input_file.close();

  const auto fmu_file_path = testdir() / "SingleFamilyHouse.fmu";
  const auto cmd = spawnexe() + " --create " + spawn_input_path.generic_string() + " --no-compress --output-path " + fmu_file_path.generic_string();
  system(cmd.c_str());

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
