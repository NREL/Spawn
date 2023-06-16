#include "open_modelica/strings.hpp"
#include "util/filesystem.hpp"

#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omc/c/meta/meta_modelica.h>

extern "C" {
int omc_Main_handleCommand(void *threadData, void *imsg, void **omsg);
void *omc_Main_init(void *threadData, void *args);
}

TEST_CASE("Test OpenModelica is able to compile a simple model")
{
  MMC_INIT();
  MMC_TRY_TOP()

  const auto args = mmc_mk_nil();
  // threadData is defined by the MMC_TRY_TOP macro
  omc_Main_init(threadData, args);

  constexpr auto model_text = R"(
    model Test
      Real x, y;
    equation
      x = 5.0+time; y = 6.0;
    end Test;
  )";

  std::string command;
  int response_code;
  void *response_data{nullptr};
  std::string response_string;

  // Load the model from a string
  command = fmt::format(R"(loadString("{}"))", model_text);
  response_code = omc_Main_handleCommand(threadData, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data);
  CHECK(response_string == "true\n");

  // Compile the model
  command = "buildModel(Test)";
  response_code = omc_Main_handleCommand(threadData, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data);
  const auto model_path = spawn::open_modelica::parse_command_response(response_string)[0];
  CHECK(spawn_fs::exists(model_path));

  // Make sure there are no errors
  command = "getErrorString()";
  response_code = omc_Main_handleCommand(threadData, mmc_mk_scon(command.c_str()), &response_data);
  CHECK(response_code == 1);
  response_string = MMC_STRINGDATA(response_data);
  CHECK(response_string == "\"\"\n");

  MMC_CATCH_TOP();
}
