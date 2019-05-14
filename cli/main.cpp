#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <pugixml.hpp>
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include <config.hxx>

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
  auto basepath = boost::filesystem::path(jsoninput).parent_path();
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
    epFMISourcePath = exedir / "libepfmi.dylib";
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
    epFMISourcePath = exedir / "libepfmi.so";
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

  auto variables = doc.child("fmiModelDescription").child("ModelVariables");

  int vr = 1;
  for (const auto &z : zones) {
    auto zoneName = z.at("name").get<std::string>();

    auto afloName = zoneName + "_AFlo";
    auto aflo = variables.append_child("ScalarVariable");
    aflo.append_attribute("name") = afloName.c_str();
    aflo.append_attribute("valueReference") = vr;
    aflo.append_attribute("description") = "Floor area";
    aflo.append_attribute("causality") = "parameter";
    aflo.append_attribute("variability") = "fixed";
    aflo.append_attribute("initial") = "exact";

    auto real = aflo.append_child("Real");
    real.append_attribute("quantity") = "area";
    real.append_attribute("unit") = "m2";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("start") = 12.0;

    ++vr;

    auto vName = zoneName + "_V";
    auto v = variables.append_child("ScalarVariable");
    v.append_attribute("name") = vName.c_str();
    v.append_attribute("valueReference") = "Volume";
    v.append_attribute("description") = vr;
    v.append_attribute("causality") = "parameter";
    v.append_attribute("variability") = "fixed";
    v.append_attribute("initial") = "exact";

    real = v.append_child("Real");
    real.append_attribute("quantity") = "volume";
    real.append_attribute("unit") = "m3";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("start") = 36.0;

    ++vr;

    auto mSenFacName = zoneName + "_mSenFac";
    auto mSenFac = variables.append_child("ScalarVariable");
    mSenFac.append_attribute("name") = mSenFacName.c_str();
    mSenFac.append_attribute("valueReference") = vr;
    mSenFac.append_attribute("description") = "Factor for scaling sensible thermal mass of volume";
    mSenFac.append_attribute("causality") = "parameter";
    mSenFac.append_attribute("variability") = "fixed";
    mSenFac.append_attribute("initial") = "exact";

    real = mSenFac.append_child("Real");
    real.append_attribute("quantity") = "valume";
    real.append_attribute("unit") = "m3";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("start") = 1.0;

    ++vr;

    auto tName = zoneName + "_T";
    auto t = variables.append_child("ScalarVariable");
    t.append_attribute("name") = tName.c_str();
    t.append_attribute("valueReference") = vr;
    t.append_attribute("description") = "Temperature of the zone air";
    t.append_attribute("causality") = "input";
    t.append_attribute("variability") = "continuous";

    real = t.append_child("Real");
    real.append_attribute("quantity") = "ThermodynamicTemperature";
    real.append_attribute("unit") = "degC";
    real.append_attribute("relativeQuantity") = "false";
    real.append_attribute("start") = 0.0;

    ++vr;

    auto qConSen_flowName = zoneName + "_QConSen_flow";
    auto qConSen_flow = variables.append_child("ScalarVariable");
    qConSen_flow.append_attribute("name") = qConSen_flowName.c_str();
    qConSen_flow.append_attribute("valueReference") = vr;
    qConSen_flow.append_attribute("description") = "Convective sensible heat added to the zone";
    qConSen_flow.append_attribute("causality") = "output";
    qConSen_flow.append_attribute("variability") = "discrete";
    qConSen_flow.append_attribute("initial") = "calculated";

    real = qConSen_flow.append_child("Real");
    real.append_attribute("quantity") = "Power";
    real.append_attribute("unit") = "W";
    real.append_attribute("relativeQuantity") = "false";

    ++vr;
  }

  doc.save_file(modelDescriptionPath.c_str());

  //void zip_directory(const std::string& inputdir, const std::string& output_filename);
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

