#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <pugixml.hpp>
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include <config.hxx>
#include <FMI/Variables.hpp>

#if defined _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

using json = nlohmann::json;

void createFMU(const std::string &jsoninput) {
  json j;

  std::ifstream fileinput(jsoninput);
  if (!fileinput.fail()) {
    // deserialize from file
    fileinput >> j;
  } else {
    // Try to parse command line input as json string
    j = json::parse(jsoninput, nullptr, false);
  }

  json idf;
  json idd;
  json weather;
  json zones;
  json fmuname;

  if (j.is_discarded()) {
    std::cout << "Cannot parse json: '" << jsoninput << "'" << std::endl;
  } else {
    try {
      idf = j.at("EnergyPlus").at("idf");
      idd = j.at("EnergyPlus").at("idd");
      weather = j.at("EnergyPlus").at("weather");
      zones = j.at("zones");
      fmuname = j.at("fmu").at("name");
    } catch (...) {
      std::cout << "Invalid json input: '" << jsoninput << "'" << std::endl;
      return;
    }
  }

  // Input paths
  auto basepath = boost::filesystem::canonical(boost::filesystem::path(jsoninput).parent_path());
  auto fmupath = boost::filesystem::path(fmuname.get<std::string>());
  if (! fmupath.is_absolute()) {
    fmupath = basepath / fmupath;
  }
  auto idfInputPath = boost::filesystem::path(idf.get<std::string>());
  if (! idfInputPath.is_absolute()) {
    idfInputPath = basepath / idfInputPath;
  }
  auto epwInputPath = boost::filesystem::path(weather.get<std::string>());
  if (! epwInputPath.is_absolute()) {
    epwInputPath = basepath / epwInputPath;
  }
  auto iddInputPath = boost::filesystem::path(idd.get<std::string>());
  if (! iddInputPath.is_absolute()) {
    iddInputPath = basepath / iddInputPath;
  }

  // Output paths
  auto fmuStaggingPath = fmupath.parent_path() / fmupath.stem();
  auto modelDescriptionPath = fmuStaggingPath / "modelDescription.xml";
  auto resourcesPath = fmuStaggingPath / "resources";
  auto idfPath = resourcesPath / idfInputPath.filename();
  auto epwPath = resourcesPath / epwInputPath.filename();
  auto iddPath = resourcesPath / iddInputPath.filename();

  boost::filesystem::path epFMIDestPath;
  boost::filesystem::path epFMISourcePath;

  #ifdef __APPLE__
    Dl_info info;
    dladdr("main", &info);
    auto exedir = boost::filesystem::path(info.dli_fname).parent_path();
    epFMISourcePath = exedir / "../lib/libepfmi.dylib";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.dylib";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/darwin64/libepfmi.dylib";
  #elif _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    auto exedir = boost::filesystem::path(szPath).parent_path();
    epFMISourcePath = exedir / "epfmi.dll";
    epFMIDestPath = fmuStaggingPath / "binaries/win64/epfmi.dll";
  #else
    Dl_info info;
    dladdr("main", &info);
    auto exedir = boost::filesystem::path(info.dli_fname).parent_path();
    epFMISourcePath = exedir / "../lib/libepfmi.so";
    if (! boost::filesystem::exists(epFMISourcePath)) {
      epFMISourcePath = exedir / "libepfmi.so";
    }
    epFMIDestPath = fmuStaggingPath / "binaries/linux64/libepfmi.so";
  #endif

  boost::filesystem::create_directories(fmuStaggingPath);
  boost::filesystem::create_directories(resourcesPath);
  boost::filesystem::create_directories(epFMIDestPath.parent_path());

  boost::filesystem::copy_file(epFMISourcePath, epFMIDestPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(idfInputPath, idfPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(iddInputPath, iddPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(epwInputPath, epwPath, boost::filesystem::copy_option::overwrite_if_exists);

  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto xmlvariables = doc.child("fmiModelDescription").child("ModelVariables");

  const auto epvariables = EnergyPlus::FMI::parseVariables(idfPath.string());

  for (const auto & varpair : epvariables) {
    const auto valueReference = varpair.first;
    const auto var = varpair.second;

    auto scalarVar = xmlvariables.append_child("ScalarVariable");
    const auto xmlVarName = var.zoneName + "_" + var.varName;
    scalarVar.append_attribute("name") = xmlVarName.c_str();
    scalarVar.append_attribute("valueReference") = std::to_string(valueReference).c_str();
    scalarVar.append_attribute("causality") = var.causality().c_str();
    if (! var.description.empty()) scalarVar.append_attribute("description") = var.description.c_str();
    if (! var.variability.empty()) scalarVar.append_attribute("variability") = var.variability.c_str();
    if (! var.initial.empty()) scalarVar.append_attribute("initial") = var.initial.c_str();

    auto real = scalarVar.append_child("Real");
    real.append_attribute("relativeQuantity") = var.relativeQuantity ? "true" : "false";
    if (! var.quantity.empty()) real.append_attribute("quantity") = var.quantity.c_str();
    if (! var.unit.empty()) real.append_attribute("unit") = var.unit.c_str();
    if (var.type != EnergyPlus::FMI::VariableType::OUTPUT) real.append_attribute("start") = std::to_string(var.start).c_str();
  }

  doc.save_file(modelDescriptionPath.c_str());

  zip_directory(fmuStaggingPath.string(), fmupath.string());
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  std::string jsoninput = "spawn.json";
  auto createOption =
      app.add_option("-c,--create", jsoninput,
                     "Create a standalone FMU based on json input", true);

  auto versionOption =
    app.add_flag("-v,--version", "Print version info and exit");

  CLI11_PARSE(app, argc, argv);

  if (*createOption) {
    createFMU(jsoninput);
  }

  if (*versionOption) {
    std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
  }

  return 0;
}

