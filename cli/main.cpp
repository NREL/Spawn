#include <CLI/CLI.hpp>
#include "third_party/nlohmann/json.hpp"
#include <stdio.h>
#include <boost/filesystem.hpp>
#include <pugixml.hpp>
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include "EnergyPlus/api/runtime.hpp"
#include "../epfmi/Variables.hpp"
#include <config.hxx>


#include "EnergyPlus/InputProcessing/IdfParser.hh"
#include "EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"

#if defined _WIN32
#include <windows.h>
#else
#include <stdio.h>
#include <dlfcn.h>
#endif

using json = nlohmann::json;

int createFMU(const std::string &jsoninput, bool nozip) {
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
      zones = j.at("model").at("zones");
      fmuname = j.at("fmu").at("name");
    } catch (...) {
      std::cout << "Invalid json input: '" << jsoninput << "'" << std::endl;
      return 1;
    }
  }

  // Input paths
  auto spawnInputPath = boost::filesystem::canonical(boost::filesystem::path(jsoninput));
  auto basepath = spawnInputPath.parent_path();
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

  // Do the input paths exist?
  bool missingFile = false;
  if (! boost::filesystem::exists(idfInputPath)) {
    std::cout << "The specified idf input file does not exist, " << idfInputPath << "." << std::endl;
    missingFile = true;
  }
  if (! boost::filesystem::exists(epwInputPath)) {
    std::cout << "The specified epw input file does not exist, " << epwInputPath << "." << std::endl;
    missingFile = true;
  }
  if (! boost::filesystem::exists(iddInputPath)) {
    std::cout << "The specified idd input file does not exist, " << iddInputPath << "." << std::endl;
    missingFile = true;
  }

  if (missingFile) {
    return 1;
  }

  // Output paths
  auto fmuStaggingPath = fmupath.parent_path() / fmupath.stem();
  auto modelDescriptionPath = fmuStaggingPath / "modelDescription.xml";
  auto resourcesPath = fmuStaggingPath / "resources";
  auto idfPath = resourcesPath / idfInputPath.filename();
  auto epwPath = resourcesPath / epwInputPath.filename();
  auto iddPath = resourcesPath / iddInputPath.filename();
  auto spawnPath = resourcesPath / spawnInputPath.filename();

  boost::filesystem::path epFMIDestPath;
  boost::filesystem::path epFMISourcePath;

  boost::filesystem::remove(fmupath);

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
  boost::filesystem::copy_file(spawnInputPath, spawnPath, boost::filesystem::copy_option::overwrite_if_exists);

  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto xmlvariables = doc.child("fmiModelDescription").child("ModelVariables");

    ::IdfParser parser;
    const auto embeddedEpJSONSchema = EnergyPlus::EmbeddedEpJSONSchema::embeddedEpJSONSchema();
    ::nlohmann::json schema;// = json::from_cbor(embeddedEpJSONSchema.first, embeddedEpJSONSchema.second);
    auto jsonidf = parser.mydecode("path/to/file", schema);

  //const auto epvariables = parseVariables(idfPath.string(),spawnInputPath.string());

  //for (const auto & varpair : epvariables) {
  //  const auto valueReference = varpair.first;
  //  const auto var = varpair.second;

  //  auto scalarVar = xmlvariables.append_child("ScalarVariable");
	//	for (const auto & attribute : var.scalar_attributes) {
  //  	scalarVar.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
	//	}

  //  auto real = scalarVar.append_child("Real");
	//	for (const auto & attribute : var.real_attributes) {
  //  	real.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
	//	}
  //}

  //doc.save_file(modelDescriptionPath.c_str());

  //if (! nozip) {
  //  zip_directory(fmuStaggingPath.string(), fmupath.string());
  //}
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  bool nozip = false;

  std::string jsoninput = "spawn.json";
  auto createOption =
      app.add_option("-c,--create", jsoninput,
                     "Create a standalone FMU based on json input", true);
  
  auto zipOption = app.add_flag("--no-zip", nozip, "Skip compressing the contents of the fmu into a zip archive");
  zipOption->needs(createOption);

  auto versionOption =
    app.add_flag("-v,--version", "Print version info and exit");

  CLI11_PARSE(app, argc, argv);

  if (*createOption) {
    auto result = createFMU(jsoninput, nozip);
    if (result) {
      return result;
    }
  }

  if (*versionOption) {
    std::cout << "Spawn-" << spawn::VERSION_STRING << std::endl;
  }

  return 0;
}

