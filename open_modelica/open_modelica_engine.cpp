#include "open_modelica/open_modelica_engine.hpp"
#include "mbl/config.hpp"
#include "open_modelica/strings.hpp"
#include "util/config.hpp"
#include "util/conversion.hpp"
#include "util/filesystem.hpp"
#include "util/strings.hpp"
#include <catch2/catch.hpp>
#include <fmt/format.h>
#include <iostream>
#include <nlohmann/json.hpp>
#include <omc/c/meta/meta_modelica.h>
#include <spdlog/spdlog.h>

extern "C" {
int omc_Main_handleCommand(void *threadData, void *imsg, void **omsg);
void *omc_Main_init(void *threadData, void *args);
}

namespace spawn::openmodelica {

OpenModelicaEngine::OpenModelicaEngine()
{
  MMC_INIT();
  // How is this deallocated?
  // Does the OpenModelica garbage collector do that with magic?
  // Is this a memory leak?
  MMC_ALLOC_AND_INIT_THREADDATA(thread_data); // NOLINT
  const auto args = mmc_mk_nil();             // NOLINT
  omc_Main_init(thread_data, args);
}

void OpenModelicaEngine::create_fmu([[maybe_unused]] const std::string_view mo_input,
                                    [[maybe_unused]] const spawn_fs::path &output_dir,
                                    [[maybe_unused]] const std::string_view modelica_path, // NOLINT
                                    [[maybe_unused]] const std::vector<spawn_fs::path> &modelica_files,
                                    [[maybe_unused]] const spawn::fmu::FMUType &fmu_type)
{
  for (const auto &file : modelica_files) {
    load_file(file);
  }

  if (!mbl_is_loaded()) {
    load_mbl();
  }

  const auto command = fmt::format(R"(buildModelFMU({}, "2.0", "cs"))", mo_input);
  const auto response = eval(command);

  spdlog::info("OpenModelica buildModel output: {}", response);
}

void OpenModelicaEngine::create_exe(const std::string_view mo_input,
                                    [[maybe_unused]] const spawn_fs::path &output_dir,
                                    [[maybe_unused]] const std::string_view modelica_path, // NOLINT
                                    const std::vector<spawn_fs::path> &modelica_files)
{
  for (const auto &file : modelica_files) {
    load_file(file);
  }

  if (!mbl_is_loaded()) {
    load_mbl();
  }

  const auto command = fmt::format(R"(buildModel({}))", mo_input);
  const auto response = eval(command);

  spdlog::info("OpenModelica buildModel output: {}", response);
}

void OpenModelicaEngine::simulate(const std::string_view model,                          // NOLINT
                                  [[maybe_unused]] const std::string_view modelica_path, // NOLINT
                                  const std::vector<spawn_fs::path> &modelica_files,
                                  [[maybe_unused]] const double &start_time, // NOLINT
                                  [[maybe_unused]] double stop_time,
                                  [[maybe_unused]] unsigned number_of_intervals,
                                  [[maybe_unused]] const double &tolerance,
                                  [[maybe_unused]] const std::string_view method)

{
  for (const auto &file : modelica_files) {
    load_file(file);
  }

  if (!mbl_is_loaded()) {
    load_mbl();
  }

  auto command =
      fmt::format(R"(simulate({}, startTime={}, stopTime={}, numberOfIntervals={}, tolerance={}, method="{}"))",
                  model,
                  start_time,
                  stop_time,
                  number_of_intervals,
                  tolerance,
                  method);
  const auto result = eval(command);

  spdlog::info("OpenModelica simulation output: {}", result);
}

void OpenModelicaEngine::clear_messages()
{
  void *response_data{nullptr};
  auto response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon("clearMessages()"), &response_data);
  if (response_code != 1) {
    throw std::runtime_error("OpenModelicaEngine failed to clear messages before evaluating command");
  }
}

void OpenModelicaEngine::log_last_message()
{
  void *response_data{nullptr};
  auto response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon("getErrorString()"), &response_data);
  if (response_code != 1) {
    throw std::runtime_error("OpenModelica failed to get error string after evaluating command");
  }
  std::string error_string = MMC_STRINGDATA(response_data); // NOLINT
  spawn::unquote(error_string);
  spawn::trim(error_string);
  if (!error_string.empty()) {
    const auto level = log_level(error_string);
    spdlog::log(level, "OpenModelica message: '{}'", error_string);
  }
}

std::string OpenModelicaEngine::eval(std::string_view command)
{
  spdlog::trace("OpenModelica is evaluating command: '{}'", command);

  clear_messages();

  void *response_data{nullptr};
  auto response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon(std::string(command).c_str()), &response_data);
  if (response_code != 1) {
    throw std::runtime_error(fmt::format("OpenModelica failed to evaluate command: '{}'", command));
  }
  std::string response_string = MMC_STRINGDATA(response_data); // NOLINT
  spawn::trim(response_string);

  log_last_message();

  return response_string;
}

void OpenModelicaEngine::load_file(const spawn_fs::path &file)
{
  const std::string command = fmt::format("loadFile(\"{}\")", file.string());
  eval(command);
}

void OpenModelicaEngine::load_mbl()
{
  load_file(mbl_home_dir() / "package.mo");
}

bool OpenModelicaEngine::mbl_is_loaded()
{
  const auto classes = spawn::open_modelica::parse_command_response(eval("getClassNames()"));

  return std::find(classes.begin(), classes.end(), "Buildings") != classes.end();
}

spdlog::level::level_enum OpenModelicaEngine::log_level(const std::string_view log_message)
{
  const std::map<std::string, spdlog::level::level_enum> log_level_map = {
      {"Error", spdlog::level::err}, {"Warning", spdlog::level::warn}, {"Notification", spdlog::level::info}};

  for (const auto &level : log_level_map) {
    if (log_message.rfind(level.first, 0) == 0) {
      return level.second;
    }
  }

  return spdlog::level::info;
}

} // namespace spawn::openmodelica
