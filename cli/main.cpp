#include "../energyplus/actuatortypes.hpp"
#include "../energyplus/fmugenerator.hpp"
#include "../energyplus/outputtypes.hpp"
#include "../fmu/simulate.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../util/config.hpp"
#include "../util/filesystem.hpp"
#include "../util/fmi_paths.hpp"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <vector>

#if defined _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined ENABLE_MODELICA_COMPILER
#include "optimica/optimica.hpp"
#endif

using json = nlohmann::json;

void handle_eptr(std::exception_ptr eptr)
{
  try {
    if (eptr) {
      std::rethrow_exception(std::move(eptr));
    }
  } catch (const std::exception &e) {
    fmt::print("Spawn encountered an error:\n\"{}\"\n", e.what());
  }
}

void main_command(CLI::App &app, [[maybe_unused]] nlohmann::json &user_input)
{
  app.set_help_all_flag("-H, --expanded-help", "Show expanded help for all subcommands");

  auto print_version = []() { std::cout << "Spawn-" << spawn::version_string() << std::endl; };
  app.add_flag_callback("-v,--version", print_version, "Print version info and exit");

  auto make_verbose = []() {};
  app.add_flag_callback("--verbose", make_verbose, "Use verbose logging");
}

void modelica_command(CLI::App &app, nlohmann::json &user_input)
{
#if defined ENABLE_MODELICA_COMPILER
  auto modelica_command = app.add_subcommand("modelica", "Subcommand for Modelica operations");
  modelica_command->add_option(
      "--modelica-path", user_input["modelica"]["modelica-path"], "Additional Modelica search paths");
  modelica_command->add_flag("--optimica", user_input["modelica"]["optimica"], "Use Optimica compiler");

  auto create_fmu_command = modelica_command->add_subcommand("create-fmu", "Compile Modelica model to FMU format");
  create_fmu_command->add_option("Model", user_input["modelica"]["create-fmu"]["model"], "Modelica model path");
  create_fmu_command->add_option("--fmu-type", user_input["modelica"]["create-fmu"]["fmu-type"], "FMU Type, CS or ME");

  create_fmu_command->callback(
      [&user_input]() { std::cout << "create-fmu callback input: " << user_input << std::endl; });
#else
  // get rid of the unused variable warning without breaking the check for the
  // other ifdef'd block
  [[maybe_unused]] const auto &app_ref = app;
  [[maybe_unused]] const auto &user_input_ref = user_input;
#endif
}

void energyplus_command(CLI::App &app, nlohmann::json &user_input)
{
  auto energyplus_command = app.add_subcommand("energyplus", "Subcommand for EnergyPlus related operations");
  energyplus_command->add_flag("-v, --version", "Print version info about the embedded EnergyPlus software");

  auto fmu_command = energyplus_command->add_subcommand("create-fmu", "Create a standalone FMU based on json input");

  auto &fmu_input_path = user_input["energyplus"]["create-fmu"]["input"];
  fmu_command->add_option("INPUT_FILE_PATH", fmu_input_path, "Spawn input file");

  auto &fmu_output_path = user_input["energyplus"]["create-fmu"]["output-path"];
  constexpr auto output_path_doc = "Full path including filename and extension where the fmu should be placed";
  fmu_command->add_option("--output-path", fmu_output_path, output_path_doc);

  auto &fmu_output_dir = user_input["energyplus"]["create-fmu"]["output-dir"];
  constexpr auto output_dir_doc = "Directory where the fmu should be placed. This path will be created if necessary";
  fmu_command->add_option("--output-dir", fmu_output_dir, output_dir_doc);

  auto &no_zip = user_input["energyplus"]["create-fmu"]["no-zip"];
  constexpr auto no_zip_doc = "Stage FMU files on disk without creating a zip archive";
  fmu_command->add_flag("--no-zip", no_zip, no_zip_doc);

  auto &no_compress = user_input["energyplus"]["create-fmu"]["no-compress"];
  constexpr auto no_compress_doc =
      "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead";
  fmu_command->add_flag("--no-compress", no_compress, no_compress_doc);

  constexpr auto output_vars_doc = "Report the EnergyPlus output variables supported by this version of Spawn";
  auto output_vars_command = energyplus_command->add_subcommand("list-output-variables", output_vars_doc);
  auto output_vars_callback = [&user_input]() { std::cout << "output vars input: " << user_input << std::endl; };
  output_vars_command->callback(output_vars_callback);

  constexpr auto actuators_doc = "Report the EnergyPlus output variables supported by this version of Spawn";
  auto actuators_command = energyplus_command->add_subcommand("list-actuators", actuators_doc);
  auto actuators_vars_callback = [&user_input]() { std::cout << "actuators input: " << user_input << std::endl; };
  actuators_command->callback(actuators_vars_callback);
}

void cc_command(CLI::App &app, [[maybe_unused]] nlohmann::json &user_input)
{
  auto cc_command = app.add_subcommand("cc", "Subcommand for C compiler with a 'make' style interface");
  cc_command->allow_extras();
}

void fmu_command(CLI::App &app, nlohmann::json &user_input)
{
  auto fmu_command = app.add_subcommand("fmu", "Subcommand for FMU related operations");
  auto simulate_command = fmu_command->add_subcommand("simulate", "Simulate an existing FMU");

  auto &start = user_input["fmu"]["simulate"]["start"].get_ref<json::number_float_t &>();
  simulate_command->add_option("--start", start, "Simulation start time");
  auto &stop = user_input["fmu"]["simulate"]["stop"].get_ref<json::number_float_t &>();
  simulate_command->add_option("--stop", stop, "Simulation stop time");
  auto &step = user_input["fmu"]["simulate"]["step"].get_ref<json::number_float_t &>();
  simulate_command->add_option("--step", step, "Simulation step size");
  auto &fmu_path = user_input["fmu"]["simulate"]["fmu_path"];
  simulate_command->add_option("FMU_PATH", fmu_path, "FMU path");

  auto callback = [&user_input]() { std::cout << "fmu_command input: " << user_input << std::endl; };
  simulate_command->callback(callback);
}

int main(int argc, const char *argv[]) // NOLINT exception may escape from main
{
  spdlog::cfg::load_env_levels();

  // clang-format off
  nlohmann::json user_input = {
    {"verbose", false},
    {"modelica",
      {
        {"modelica-path", ""},
        {"optimica", false},
        {"create-fmu",
          {
            {"model", ""},
            {"fmu-type", toString(spawn::fmu::FMUType::CS)}
          }
        }
      }
    },
    {"energyplus",
      {
        {"create-fmu",
          {
            {"input", ""},
            {"output-path", ""},
            {"output-dir", ""},
            {"no-zip", false},
            {"no-compress", false}
          }
        }
      }
    },
    {"cc", {}},
    {"fmu",
      {
        {"simulate",
          {
            {"fmu_path", ""},
            {"start", 0.0},
            {"stop", 60.0},
            {"step", 0.001}
          }
        }
      }
    }
  };
  // clang-format on

  CLI::App app{"Spawn of EnergyPlus"};

  main_command(app, user_input);
  modelica_command(app, user_input);
  energyplus_command(app, user_input);
  cc_command(app, user_input);
  fmu_command(app, user_input);

  std::exception_ptr eptr;

  try {
    CLI11_PARSE(app, argc, argv);
  } catch (...) {
    eptr = std::current_exception();
  }

  if (eptr) {
    handle_eptr(eptr);
    return 1;
  } else {
    return 0;
  }

  //    if (*verboseOption) {
  //      spdlog::set_level(spdlog::level::trace);
  //    } else {
  //      spdlog::set_pattern("%v");
  //      spdlog::set_level(spdlog::level::info);
  //    }
  //
  //    if (*createOption) {
  //      spawn::energyplusToFMU(
  //          jsonInput, nozip, nocompress, outputPath, outputDir, spawn::idd_install_path(),
  //          spawn::epfmi_install_path());
  // #if defined ENABLE_MODELICA_COMPILER
  //    } else if (*createFMUCommand) {
  //      auto optimica = spawn::Optimica();
  //      std::ignore =
  //          optimica.generateFMU(moinput, spawn_fs::current_path(), modelicaPaths, spawn::fmu::toFMUType(fmuType));
  //    } else if (*makeOption) {
  //      auto optimica = spawn::Optimica();
  //      optimica.makeModelicaExternalFunction(app.remaining(true));
  // #endif
  //    } else if (*versionOption) {
  //      std::cout << "Spawn-" << spawn::version_string() << std::endl;
  //    } else if (*energyplusVersionOption) {
  //      std::cout << EnergyPlus::DataStringGlobals::VerString << std::endl;
  //    } else if (*outputVarsOption) {
  //      std::cout << nlohmann::json(outputtypes).dump(4) << std::endl;
  //    } else if (*actuatorsOption) {
  //      std::cout << nlohmann::json(actuatortypes).dump(4) << std::endl;
  //    } else if (*fmuSimulateOption) {
  //      spawn::fmu::Sim sim(fmuinput);
  //      nlohmann::json config;
  //      config["start"] = fmustart;
  //      config["stop"] = fmustop;
  //      config["step"] = fmustep;
  //      sim.run(config);
  //    }
  //
}
