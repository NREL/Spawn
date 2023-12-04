#include "mbl/config.hpp"
#include "open_modelica/open_modelica_engine.hpp"
#include "open_modelica/strings.hpp"
#include "util/filesystem.hpp"
#include "util/strings.hpp"
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omc/c/meta/meta_modelica.h>

extern "C" {
int omc_Main_handleCommand(void *threadData, void *imsg, void **omsg);
void *omc_Main_init(void *threadData, void *args);
}

const std::string one_zone_one_year =
    "Buildings.ThermalZones." + spawn::mbl_energyplus_version_string() + ".Validation.ThermalZone.OneZoneOneYear";

TEST_CASE("Low level OpenModelica API can make very simple API calls") // NOLINT
{
  MMC_INIT();
  threadData_t *thread_data = nullptr;
  MMC_ALLOC_AND_INIT_THREADDATA(thread_data); // NOLINT

  const auto args = mmc_mk_nil(); // NOLINT
  omc_Main_init(thread_data, args);

  std::string command;
  int response_code{0};
  void *response_data{nullptr};
  std::string response_string;

  command = "getModelicaPath()";
  response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data); // NOLINT
  CHECK(!response_string.empty());

  command = "getErrorString()";
  response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data); // NOLINT

  spawn::trim(response_string);
  spawn::unquote(response_string);
  CHECK(response_string.empty()); // NOLINT
}

TEST_CASE("Low level OpenModelica API is able to compile a simple model") // NOLINT
{
  MMC_INIT();
  threadData_t *thread_data = nullptr;
  MMC_ALLOC_AND_INIT_THREADDATA(thread_data); // NOLINT

  const auto args = mmc_mk_nil(); // NOLINT
  omc_Main_init(thread_data, args);

  constexpr auto model_text = R"(
    model Test
      Real x, y;
    equation
      x = 5.0+time; y = 6.0;
    end Test;
  )";

  std::string command;
  int response_code{0};
  void *response_data{nullptr};
  std::string response_string;

  // Load the model from a string
  command = fmt::format(R"(loadString("{}"))", model_text);
  response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data); // NOLINT
  CHECK(spawn::trim(response_string) == "true");

  // Compile the model
  command = "buildModelFMU(Test)";
  response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data); // NOLINT
  const auto model_path = spawn::open_modelica::parse_command_response(response_string)[0];
  CHECK(spawn_fs::exists(model_path));

  // Make sure there are no errors
  command = "getErrorString()";
  response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data); // NOLINT

  spawn::trim(response_string);
  spawn::unquote(response_string);
  std::cout << "response_string: " << response_string << std::endl;
  CHECK(response_string.empty());
}

TEST_CASE("OpenModelicaEngine is able to evaluate the simple getVersion command") // NOLINT
{
  spawn::openmodelica::OpenModelicaEngine om;

  CHECK(!om.eval("getVersion()").empty());
}

TEST_CASE("OpenModelicaEngine is able to get and set the compiler") // NOLINT
{
  spawn::openmodelica::OpenModelicaEngine om;

  CHECK(om.eval("setCompiler(\"spawn\")") == "true");
  CHECK(om.eval("getCompiler()") == "\"spawn\"");
}

TEST_CASE("OpenModelicaEngine is able to set the ModelicaPath") // NOLINT
{
  spawn::openmodelica::OpenModelicaEngine om;

  // Notice the path doesn't need to make sense
  CHECK(om.eval("setModelicaPath(\"alkjlkjl:abc\")") == "true");
  CHECK(om.eval("getModelicaPath()") == "\"alkjlkjl:abc\"");
}

TEST_CASE("OpenModelicaEngine is able to compile a simple model") // NOLINT
{
  spawn::openmodelica::OpenModelicaEngine om;

  constexpr auto model_text = R"(
    model Test
      Real x, y;
    equation
      x = 5.0+time; y = 6.0;
    end Test;
  )";

  std::string command;
  std::string response;

  // Load the model from a string
  command = fmt::format(R"(loadString("{}"))", model_text);
  response = om.eval(command);

  command = "buildModel(Test)";
  response = om.eval(command);

  command = "getErrorString()";
  response = om.eval(command);
}

TEST_CASE("OpenModelicaEngine is able to compile a model from the Modelica standard library") // NOLINT
{
  spawn::openmodelica::OpenModelicaEngine om;

  om.eval("installPackage(Modelica)");
  om.eval("buildModel(Modelica.Blocks.Examples.PID_Controller)");
}

TEST_CASE("OpenModelicaEngine is able to compile the single family home example") // NOLINT
{
  spawn::openmodelica::OpenModelicaEngine om;

  om.create_fmu(one_zone_one_year, spawn_fs::current_path(), {}, {}, spawn::fmu::FMUType::CS);
}
