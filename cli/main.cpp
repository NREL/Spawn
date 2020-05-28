#include "../util/platform.hpp"
#include "../lib/fmugenerator.hpp"
#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <boost/filesystem.hpp>
#include <config.hxx>
#include <boost/algorithm/string.hpp>
#include <stdlib.h>
#include "../util/fmi_paths.hpp"

#if defined _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

#if defined ENABLE_MODELICA_COMPILER
#include "compilerchain.hpp"
#endif

using json = nlohmann::json;

boost::filesystem::path exedir() {
  #if _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    return boost::filesystem::path(szPath).parent_path();
  #else
    Dl_info info;
    dladdr("main", &info);
    return boost::filesystem::path(info.dli_fname).parent_path();
  #endif
}

bool isInstalled() {
  return exedir().stem() == "bin";
}

boost::filesystem::path iddInstallPath() {
  constexpr auto & iddfilename = "Energy+.idd";
  // Configuration in install tree
  auto iddInputPath = exedir() / "../etc" / iddfilename;

  // Configuration in a developer tree
  if (! boost::filesystem::exists(iddInputPath)) {
    iddInputPath = exedir() / iddfilename;
  }

  return iddInputPath;
}

boost::filesystem::path epfmiInstallPath() {
  const auto candidate = exedir() / ("../lib/" + spawn::epfmiName());
  if (boost::filesystem::exists(candidate)) {
    return candidate;
  } else {
    return exedir() / spawn::epfmiName();
  }
}

boost::filesystem::path jmodelicaHome() {
  if (isInstalled()) {
    return exedir() / "../JModelica/";
  } else {
    boost::filesystem::path binary_dir(spawn::BINARY_DIR);
    //return binary_dir / "modelica/JModelica-prefix/src/JModelica/";
    return binary_dir / "JModelica/";
  }
}

boost::filesystem::path mblPath() {
  if (isInstalled()) {
    return exedir() / "../modelica-buildings/Buildings/";
  } else {
    boost::filesystem::path binary_dir(spawn::BINARY_DIR);
    return binary_dir / "modelica-buildings/Buildings/";
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
      app.add_option("-o,--output-path", outputpath,
                     "Path where fmu should be placed", true);
  outputPathOption->needs(createOption);

  bool nozip = false;
  auto zipOption = app.add_flag("--no-zip", nozip, "Stage FMU files on disk without creating a zip archive");
  zipOption->needs(createOption);

  bool nocompress = false;
  auto compressOption = app.add_flag("--no-compress", nocompress, "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead.");
  compressOption->needs(createOption);

  auto versionOption =
    app.add_flag("-v,--version", "Print version info and exit");

#if defined ENABLE_MODELICA_COMPILER
  std::string moinput = "";
  auto compileOption =
      app.add_option("--compile", moinput,
                     "Compile Modelica model to FMU format", true);
#endif

  CLI11_PARSE(app, argc, argv);

  if (*createOption) {
    auto result = spawn::energyplusToFMU(jsoninput, nozip, nocompress, outputpath, iddInstallPath(), epfmiInstallPath());
    if (result) {
      return result;
    }
#if defined ENABLE_MODELICA_COMPILER
  } else if (*compileOption) {
    auto result = spawn::modelicaToFMU(moinput, mblPath(), jmodelicaHome());
    if (result) {
      return result;
    }
#endif
  } else if (*versionOption) {
    std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
  }

  return 0;
}

