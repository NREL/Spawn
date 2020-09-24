#include "../fmu/fmu.hpp"
#include "../fmu/modelexchange.hpp"
#include "testpaths.hpp"
#include "spawn.hpp"
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>


TEST_CASE("Test loading of FMI")
{
  const auto fmi_file = fmi_load_test();
  REQUIRE(boost::filesystem::exists(fmi_file));
  REQUIRE(boost::filesystem::is_regular(fmi_file));

  spawn::fmu::FMI2 fmi{fmi_load_test(), false};

  REQUIRE(fmi.fmi2GetVersion.has_value());


  CHECK(fmi.fmi2GetVersion() == std::string("TEST_VERSION"));

  CHECK(fmi.loadResults().successes.size() == 1);
  CHECK(fmi.loadResults().failures.size() == 43);
}

TEST_CASE("Test loading of FMI missing symbols")
{
  const auto fmi_file = fmi_load_test();
  REQUIRE(boost::filesystem::exists(fmi_file));
  REQUIRE(boost::filesystem::is_regular(fmi_file));

  REQUIRE_THROWS(spawn::fmu::FMI2{fmi_load_test(), true});
}

TEST_CASE("Test loading of Spawn Generated FMU")
{
  const auto fmu_file = create_fmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));
}

// Do nothing logger
void fmuLogger(fmi2ComponentEnvironment, fmi2String, fmi2Status, fmi2String, fmi2String, ...)
{
}

TEST_CASE("Test features of Spawn Generated FMU")
{
  spdlog::set_level(spdlog::level::trace);
  const auto fmu_file = create_fmu();
  spawn::fmu::FMU fmu{fmu_file, false}; // don't require all symbols
  CHECK(fmu.fmi.fmi2GetVersion() == std::string("2.0"));

  //// This is not the correct resource_path
  const auto resource_path = (fmu.extractedFilesPath() / "resources").string();
  fmi2CallbackFunctions callbacks = {fmuLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  fmu.fmi.fmi2Instantiate("test-instance", fmi2ModelExchange, "abc-guid", resource_path.c_str(), &callbacks, false, true);

  const auto dump_variable = [](const spawn::fmu::FMU::Variable &variable) {
    spdlog::trace(R"(Variable: {} {} "{}" {})",
        variable.name,
        variable.valueReference,
        variable.description,
        variable.to_string(variable.type));
  }; 

  for (const auto &variable : fmu.getVariables()) {
    dump_variable(variable);
  }

  CHECK(fmu.getVariables().size() == 19);
}

TEST_CASE("Test ModelExchange interaction")
{
  spdlog::set_level(spdlog::level::trace);
  const auto fmu_file = create_fmu();
  auto model = spawn::fmu::ModelExchange{fmu_file, "model", true, false};


  // get a variable reference we'll use multiple times
  const auto &core_zn_t = model.fmu.getVariableByName("Core_ZN_T");

  // set already looked up variable
  model.setVariable(core_zn_t, 42.3);

  // the next set of tests work with variables by name

  // Lights Schedule is a real, aka double, so this should throw
  REQUIRE_THROWS(model.getVariable<int>("Lights_Schedule"));

  // Unknown variable should also throw
  REQUIRE_THROWS(model.setVariable("Unknown_Variable", 2.0));
  REQUIRE_THROWS(model.getVariable<int>("Unknown Variable"));

  // Note, we cannot actually exchange variables here with the E+ FMI interface
  // unless we have an idf and experiment running

  // we must setup an experiment to be able to exchange variables and read from E+
  // model.fmu.fmi.fmi2SetupExperiment(model.component, false, 0.0, start_time, true, final_time);
  //
  // SEE ALSO examples/smi.cpp for examples
}



