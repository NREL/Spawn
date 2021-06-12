#include "../lib/fmugenerator.hpp"
#include "../lib/outputtypes.hpp"
#include "../lib/actuatortypes.hpp"
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <config.hxx>
#include <stdlib.h>
#include <spdlog/spdlog.h>
#include "../util/fmi_paths.hpp"
#include "../util/filesystem.hpp"
#include "../util/paths.hpp"

#if defined _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined ENABLE_MODELICA_COMPILER
#include "compilerchain.hpp"
#endif

using json = nlohmann::json;


bool isInstalled() {
  return spawn::exedir().stem() == "bin";
}

fs::path iddInstallPath() {
  constexpr auto & iddFileName = "Energy+.idd";
  // Configuration in install tree
  auto iddInputPath = spawn::exedir() / "../etc" / iddFileName;

  // Configuration in a developer tree
  if (! fs::exists(iddInputPath)) {
    iddInputPath = spawn::exedir() / iddFileName;
  }

  return iddInputPath;
}

fs::path epfmiInstallPath() {
  const auto candidate = spawn::exedir() / ("../lib/" + spawn::epfmi_filename());
  if (fs::exists(candidate)) {
    return candidate;
  } else {
    return spawn::exedir() / spawn::epfmi_filename();
  }
}

fs::path mblPath() {
  fs::path p;

  if (isInstalled()) {
    p = spawn::exedir() / "../etc/modelica-buildings/Buildings/";
  } else {
    p = spawn::project_source_dir() / "submodules/modelica-buildings/Buildings/";
  }

  return p.lexically_normal();
}

fs::path mslPath() {
  fs::path p;
  if (isInstalled()) {
    p = spawn::exedir() / "../etc/MSL/";
  } else {
    p = spawn::project_binary_dir() / "JModelica/ThirdParty/MSL/";
  }

  return p.lexically_normal();
}

void handle_eptr(std::exception_ptr eptr) {
  try {
    if (eptr) {
      std::rethrow_exception(eptr);
    }
  } catch(const std::exception& e) {
    fmt::print("Spawn encountered an error:\n\"{}\"\n", e.what());
  }
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  auto versionOption = app.add_flag("-v,--version", "Print version info and exit");
  auto verboseOption = app.add_flag("--verbose", "Use verbose logging");

  std::string jsoninput = "spawn.json";
  auto createOption =
      app.add_option("-c,--create", jsoninput,
                     "Create a standalone FMU based on json input", true);

  std::string outputpath;
  auto outputPathOption =
      app.add_option("--output-path", outputpath,
                     "Full path including filename and extension where the fmu should be placed. Intermediate directories will be created if necessary", true);
  outputPathOption->needs(createOption);

  std::string outputdir;
  auto outputDirOption =
      app.add_option("--output-dir", outputdir,
                     "Directory where the fmu should be placed. This path will be created if necessary", true);
  outputDirOption->needs(createOption);

  outputDirOption->excludes(outputPathOption);
  outputPathOption->excludes(outputDirOption);

  bool nozip = false;
  auto zipOption = app.add_flag("--no-zip", nozip, "Stage FMU files on disk without creating a zip archive");
  zipOption->needs(createOption);

  bool nocompress = false;
  auto compressOption = app.add_flag("--no-compress", nocompress, "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead.");
  compressOption->needs(createOption);

  auto outputVarsOption = app.add_flag("--output-vars", "Report the EnergyPlus output variables supported by this version of Spawn.");

  auto actuatorsOption = app.add_flag("--actuators", "Report the EnergyPlus actuators supported by this version of Spawn.");

#if defined ENABLE_MODELICA_COMPILER
  auto modelicaCommand = app.add_subcommand("modelica", "Subcommand for interacting Modelica operations.");
  std::string moinput = "";
  auto createModelicaFMUOption =
      modelicaCommand->add_option("--create-fmu", moinput,
                     "Compile Modelica model to FMU format", true);

  bool optimica = false;
  auto optimicaOption = modelicaCommand->add_flag("--optimica", optimica, "Use Optimica compiler.");
  optimicaOption->needs(createModelicaFMUOption);

  bool jmodelica = false;
  auto jmodelicaOption = modelicaCommand->add_flag("--jmodelica", jmodelica, "Use JModelica compiler.");
  jmodelicaOption->needs(createModelicaFMUOption);

  auto makeOption = app.add_flag("-f", "compile a Modelica external function, acting like 'make'");
#endif

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
      spawn::energyplusToFMU(jsoninput, nozip, nocompress, outputpath, outputdir, iddInstallPath(), epfmiInstallPath());
#if defined ENABLE_MODELICA_COMPILER
    } else if (*createModelicaFMUOption) {
      if (optimica) {
        spawn::modelicaToFMU(moinput, mblPath(), mslPath(), spawn::ModelicaCompilerType::Optimica);
      } else {
        spawn::modelicaToFMU(moinput, mblPath(), mslPath());
      }
    } else if (*makeOption) {
      spawn::makeModelicaExternalFunction(app.remaining(true));
#endif
    } else if (*versionOption) {
      std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
    } else if (*outputVarsOption) {
      std::cout << nlohmann::json(outputtypes).dump(4) << std::endl;
    } else if (*actuatorsOption) {
      std::cout << nlohmann::json(actuatortypes).dump(4) << std::endl;
    }
  } catch(...) {
    eptr = std::current_exception();
  }

  if (eptr) {
    handle_eptr(eptr);
    return 1;
  } else {
    return 0;
  }
}

