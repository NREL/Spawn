#include "../lib/actuatortypes.hpp"
#include "../lib/fmugenerator.hpp"
#include "../lib/outputtypes.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../util/filesystem.hpp"
#include "../util/fmi_paths.hpp"
#include "../util/paths.hpp"
#include <CLI/CLI.hpp>
#include <algorithm>
#include <config.hxx>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <iterator>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <stdlib.h>
#include <vector>

#if defined _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined ENABLE_MODELICA_COMPILER
#include "compile.hpp"
#endif
#include "simulate.hpp"

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

int main(int argc, const char *argv[])
{
  CLI::App app{"Spawn of EnergyPlus"};

  auto versionOption = app.add_flag("-v,--version", "Print version info and exit");
  auto verboseOption = app.add_flag("--verbose", "Use verbose logging");

  std::string jsonInput = "spawn.json";
  auto createOption = app.add_option("-c,--create", jsonInput, "Create a standalone FMU based on json input", true);

  std::string outputPath;
  auto outputPathOption = app.add_option("--output-path",
                                         outputPath,
                                         "Full path including filename and extension where the fmu should be placed. "
                                         "Intermediate directories will be created if necessary",
                                         true);
  outputPathOption->needs(createOption);

  std::string outputDir;
  auto outputDirOption =
      app.add_option("--output-dir",
                     outputDir,
                     "Directory where the fmu should be placed. This path will be created if necessary",
                     true);
  outputDirOption->needs(createOption);
  outputDirOption->excludes(outputPathOption);
  outputPathOption->excludes(outputDirOption);

  bool nozip = false;
  auto zipOption = app.add_flag("--no-zip", nozip, "Stage FMU files on disk without creating a zip archive");
  zipOption->needs(createOption);

  bool nocompress = false;
  auto compressOption = app.add_flag(
      "--no-compress",
      nocompress,
      "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead");
  compressOption->needs(createOption);

  auto outputVarsOption =
      app.add_flag("--output-vars", "Report the EnergyPlus output variables supported by this version of Spawn");

  auto actuatorsOption =
      app.add_flag("--actuators", "Report the EnergyPlus actuators supported by this version of Spawn");

#if defined ENABLE_MODELICA_COMPILER
  auto modelicaCommand = app.add_subcommand("modelica", "Subcommand for Modelica operations");
  std::string moinput = "";
  auto createModelicaFMUOption =
      modelicaCommand->add_option("--create-fmu", moinput, "Compile Modelica model to FMU format", true);

  std::vector<std::string> modelicaPaths;
  auto modelicaPathsOption =
      modelicaCommand->add_option("--modelica-path", modelicaPaths, "Additional Modelica search paths");
  modelicaPathsOption->needs(createModelicaFMUOption);

  bool optimica = false;
  auto optimicaOption = modelicaCommand->add_flag("--optimica", optimica, "Use Optimica compiler");
  optimicaOption->needs(createModelicaFMUOption);

  bool jmodelica = false;
  auto jmodelicaOption = modelicaCommand->add_flag("--jmodelica", jmodelica, "Use JModelica compiler");
  jmodelicaOption->needs(createModelicaFMUOption);

  auto makeOption = app.add_flag("-f", "compile a Modelica external function, acting like 'make'");
#endif

  auto fmuCommand = app.add_subcommand("fmu", "Subcommand for FMU related operations");
  std::string fmuinput;
  double fmustart = 0.0;
  double fmustop = 60.0;
  double fmustep = 0.001;
  auto fmuSimulateOption = fmuCommand->add_option("--simulate", fmuinput, "Simulate the FMU located at the given path");
  auto fmuStartOption = fmuCommand->add_option("--start", fmustart, "Simulation start time");
  fmuStartOption->needs(fmuSimulateOption);
  auto fmuStopOption = fmuCommand->add_option("--stop", fmustop, "Simulation stop time");
  fmuStopOption->needs(fmuSimulateOption);
  auto fmuStepOption = fmuCommand->add_option("--step", fmustep, "Simulation step size");
  fmuStepOption->needs(fmuSimulateOption);

  auto energyplusCommand = app.add_subcommand("energyplus", "Subcommand for EnergyPlus related operations");
  auto energyplusVersionOption =
      energyplusCommand->add_flag("-v, --version", "Print version info about the embedded EnergyPlus software");

  app.allow_extras();

  CLI11_PARSE(app, argc, argv);

  std::exception_ptr eptr;

  try {
    if (*verboseOption) {
      spdlog::set_level(spdlog::level::trace);
    } else {
      spdlog::set_pattern("%v");
      spdlog::set_level(spdlog::level::info);
    }

    if (*createOption) {
      spawn::energyplusToFMU(
          jsonInput, nozip, nocompress, outputPath, outputDir, spawn::idd_install_path(), spawn::epfmi_install_path());
#if defined ENABLE_MODELICA_COMPILER
    } else if (*createModelicaFMUOption) {
      if (optimica) {
        spawn::modelicaToFMU(moinput, modelicaPaths, spawn::ModelicaCompilerType::Optimica);
      } else {
        spawn::modelicaToFMU(moinput, modelicaPaths);
      }
    } else if (*makeOption) {
      spawn::makeModelicaExternalFunction(app.remaining(true));
#endif
    } else if (*versionOption) {
      std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
    } else if (*energyplusVersionOption) {
      std::cout << EnergyPlus::DataStringGlobals::VerString << std::endl;
    } else if (*outputVarsOption) {
      std::cout << nlohmann::json(outputtypes).dump(4) << std::endl;
    } else if (*actuatorsOption) {
      std::cout << nlohmann::json(actuatortypes).dump(4) << std::endl;
    } else if (*fmuSimulateOption) {
      spawn::fmu::Sim sim(fmuinput);
      nlohmann::json config;
      config["start"] = fmustart;
      config["stop"] = fmustop;
      config["step"] = fmustep;
      sim.run(config);
    }

  } catch (...) {
    eptr = std::current_exception();
  }

  if (eptr) {
    handle_eptr(eptr);
    return 1;
  } else {
    return 0;
  }
}
