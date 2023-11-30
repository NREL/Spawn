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

[[nodiscard]] spawn_fs::path
OpenModelicaEngine::create_fmu([[maybe_unused]] const std::string_view mo_input,
                               [[maybe_unused]] const spawn_fs::path &output_dir,
                               [[maybe_unused]] const std::vector<spawn_fs::path> &modelica_paths, // NOLINT
                               [[maybe_unused]] const std::vector<spawn_fs::path> &modelica_files,
                               [[maybe_unused]] const spawn::fmu::FMUType &fmu_type)
{
  for (const auto &file : modelica_files) {
    load_file(file);
  }

  if (!mbl_is_loaded()) {
    load_mbl();
  }

  auto command = fmt::format(R"(buildModelFMU({}, "2.0", "cs"))", mo_input);
  [[maybe_unused]] auto response = eval(command);

  return {};
}

[[nodiscard]] spawn_fs::path
OpenModelicaEngine::create_exe([[maybe_unused]] const std::string_view mo_input,
                               [[maybe_unused]] const spawn_fs::path &output_dir,
                               [[maybe_unused]] const std::vector<spawn_fs::path> &modelica_paths, // NOLINT
                               [[maybe_unused]] const std::vector<spawn_fs::path> &modelica_files,
                               [[maybe_unused]] const spawn::fmu::FMUType &fmu_type)
{
  return {};
}

std::string OpenModelicaEngine::eval(std::string_view command)
{
  void *response_data{nullptr};

  auto response_code = omc_Main_handleCommand(thread_data, mmc_mk_scon(std::string(command).c_str()), &response_data);
  if (response_code != 1) {
    throw std::runtime_error(fmt::format("OpenModelica failed to evaluate command: '{}'", command));
  }

  std::string response_string = MMC_STRINGDATA(response_data); // NOLINT
  return spawn::trim(response_string);
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

} // namespace spawn::openmodelica
