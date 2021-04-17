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
#include "../util/fmi_paths.hpp"
#include "../util/filesystem.hpp"

#if defined _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#if defined ENABLE_MODELICA_COMPILER
#include "compilerchain.hpp"
#endif

using json = nlohmann::json;

fs::path exedir() {
  #if _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    return fs::path(szPath).parent_path();
  #else
    Dl_info info;
    dladdr("main", &info);
    return fs::path(info.dli_fname).parent_path();
  #endif
}

bool isInstalled() {
  return exedir().stem() == "bin";
}

fs::path iddInstallPath() {
  constexpr auto & iddFileName = "Energy+.idd";
  // Configuration in install tree
  auto iddInputPath = exedir() / "../etc" / iddFileName;

  // Configuration in a developer tree
  if (! fs::exists(iddInputPath)) {
    iddInputPath = exedir() / iddFileName;
  }

  return iddInputPath;
}

fs::path epfmiInstallPath() {
  if (auto candidate = exedir() / ("../lib/" + spawn::epfmi_filename()); fs::exists(candidate)) {
    return candidate;
  } else {
    return exedir() / spawn::epfmi_filename();
  }
}

fs::path jmodelicaHome() {
  if (isInstalled()) {
    return exedir() / "../JModelica/";
  } else {
    fs::path binary_dir(spawn::BINARY_DIR);
    return binary_dir / "JModelica/";
  }
}

fs::path mblPath() {
  if (isInstalled()) {
    return exedir() / "../modelica-buildings/Buildings/";
  } else {
    fs::path source_dir(spawn::SOURCE_DIR);
    return source_dir / "submodules/modelica-buildings/Buildings/";
  }
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

  auto versionOption = app.add_flag("-v,--version", "Print version info and exit");

#if defined ENABLE_MODELICA_COMPILER
  std::string moinput = "";
  auto compileOption =
      app.add_option("--compile", moinput,
                     "Compile Modelica model to FMU format", true);
#endif

  CLI11_PARSE(app, argc, argv);

  std::exception_ptr eptr;

  try {
    if (*createOption) {
      spawn::energyplusToFMU(jsoninput, nozip, nocompress, outputpath, outputdir, iddInstallPath(), epfmiInstallPath());
#if defined ENABLE_MODELICA_COMPILER
    } else if (*compileOption) {
      spawn::modelicaToFMU(moinput, mblPath(), jmodelicaHome());
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

