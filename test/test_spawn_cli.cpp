#include "paths.hpp"
#include "create_epfmu.hpp"
#include "../lib/actuatortypes.hpp"
#include "../util/filesystem.hpp"
#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../util/filesystem.hpp"
#include "../util/math.hpp"
#include "../util/paths.hpp"
#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"
#include <catch2/catch.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

TEST_CASE( "Spawn shows help" ) {
  const auto cmd = spawnexe() + " --help";
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}

// This is the main requirement of spawn executable,
// generate an FMU for a given EnergyPlus model
TEST_CASE( "Spawn creates an FMU" ) {
  fs::path created_fmu;
  REQUIRE_NOTHROW(created_fmu = create_epfmu());
  CHECK(fs::is_regular_file(created_fmu));
  CHECK(fs::file_size(created_fmu) > 0);
}

#if defined ENABLE_MODELICA_COMPILER
TEST_CASE( "Spawn is able to compile a simple Modelica model" ) {
  const auto cmd = spawnexe() + " --compile Buildings.Controls.OBC.CDL.Continuous.Validation.Line";
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
  // Glob that binary exists. ie
  // <build_dir>/Buildings_Controls_OBC_CDL_Continuous_Validation_Line/binaries/Buildings_Controls_OBC_CDL_Continuous_Validation_Line.so
  // using correct shared library extension for platform
}

TEST_CASE( "Spawn is able to compile a Modelica model that uses external functions" ) {
  const auto cmd = spawnexe() + " --compile Buildings.ThermalZones.EnergyPlus.Validation.ThermalZone.OneZoneOneYear";
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}
#endif

TEST_CASE( "Spawn lists the correct actuators" ) {
  // copy idf and modify to generate edd file
  const auto idf_path = spawn::project_source_dir() / "submodules/EnergyPlus/testfiles/RefBldgSmallOfficeNew2004_Chicago.idf";
  const auto new_idf_path = testdir() / "actuators.idf";
  fs::copy(idf_path, new_idf_path, fs::copy_options::overwrite_existing);

  std::ofstream new_idf;
  new_idf.open(new_idf_path, std::ios::app);
  new_idf << R"(
    Output:EnergyManagementSystem,
      NotByUniqueKeyNames,    ! Actuator Availability Dictionary Reporting
      None,    ! Internal Variable Availability Dictionary Reporting
      None; ! EnergyPlus Runtime Language Debug Output Level
  )";
  new_idf.close();

  // Minimal Spawn input
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
  )", fmt::arg("idfpath", new_idf_path.generic_string()), fmt::arg("epwpath", chicago_epw_path().generic_string()));


  // Run a basic simulation to generate a edd file
  const auto fmu_file_path = create_epfmu(spawn_input_string);
  spawn::fmu::FMU fmu{fmu_file_path, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuNothingLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);
  fmu.fmi.fmi2SetupExperiment(comp, false, 0.0, 0.0, false, 0.0);
  fmu.fmi.fmi2ExitInitializationMode(comp);
  fmu.fmi.fmi2SetTime(comp, spawn::days_to_seconds(1));
  fmu.fmi.fmi2Terminate(comp);

  const auto edd_file_path = resource_path + "/../eplusout/eplusout.edd";
  std::fstream edd_file;
  edd_file.open(edd_file_path, std::ios::in);
  std::vector<std::string> edd_actuators;
  std::string line;
  // discard the first line
  std::getline(edd_file, line);
  // remaining lines are actuator names
  while (std::getline(edd_file, line)) {
    // lines look like this....
    // EnergyManagementSystem:Actuator Available, *,Surface,Surface Inside Temperature,[C]
    // cleanup what we don't want
    std::string prefix("EnergyManagementSystem:Actuator Available, *,");
    line.erase(0, prefix.length());
    auto pos = line.find(",");
    line.erase(0,pos + 1);
    pos = line.find(",");
    line.erase(pos);
    edd_actuators.push_back(line);
  }
  edd_file.close();

  const json json_actuators(actuatortypes);
  std::vector<std::string> actuators;
  for (const auto a : json_actuators) {
    actuators.push_back(a["controlType"].get<std::string>());
  }

  // Check if each of the EnergyPlus reported actuators
  // are reported by spawn
  for (const auto edd_act : edd_actuators) {
    const auto it = std::find(actuators.begin(), actuators.end(), edd_act);
    INFO("'" << edd_act << "'" << " is supported by energylus but not reported by spawn --actuators");
    CHECK(it != actuators.end());
  }

  // Check if each of the Spawn reported actuators
  // are supported by Energyplus
  for (const auto act : actuators) {
    const auto it = std::find(edd_actuators.begin(), edd_actuators.end(), act);
    INFO("'" << act << "'" << " is reported by 'spawn --actuators' but not supported by EnergyPlus");
    CHECK(it != edd_actuators.end());
  }
}

/**
TEST_CASE( "Spawn simulates an FMU" ) {
  // Well it can't actually do this yet. People are using
  // JModelica, Dymola, or pyfmi to simulate the spawn FMU,
  // but there is work underway to bake an installer in directly
  const auto cmd = spawnexe() + " --simulate " + testcase1();
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}

 This is not a supported feature, but it is desirable
 and not a lot of additional effort to have spawn generate an
 FMU for co simulation. This is equivalent to the existing energyplus
 external interface feature. Not a requirement of the spawn archicture
 because spawn is based on FMU for model exchange.
TEST_CASE( "Spawn creates an FMU for co simulation" ) {
  const auto cmd = spawnexe() + "--create-cosim " + testcase1();
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}

 Only works on linux right now, because it involves packaging
 a complicated compiler toolchain
TEST_CASE( "Spawn compiles modelica files" ) {
  const auto cmd = spawnexe() + " --compile " + model1();
  const auto result = system(cmd.c_str());
  REQUIRE(result == 0);
}
**/

