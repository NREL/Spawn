#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>
#include <stdio.h>
#include <boost/filesystem.hpp>
#include "modelDescription.xml.hpp"
#include "pugixml.hpp"

using json = nlohmann::json;

void createHandler(const std::string &jsoninput) {
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

  if (j.is_discarded()) {
    std::cout << "Cannot parse json: '" << jsoninput << "'" << std::endl;
  } else {
    try {
      idf = j.at("EnergyPlus").at("idf");
      idd = j.at("EnergyPlus").at("idd");
      weather = j.at("EnergyPlus").at("weather");
      zones = j.at("zones");
    } catch (...) {
      std::cout << "Invalid json input: '" << jsoninput << "'" << std::endl;
      return;
    }
  }

  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto variables = doc.child("fmiModelDescription").child("ModelVariables");

  for (const auto &z : zones) {
    std::cout << z.at("name") << std::endl;
    auto var = variables.append_child("ScalarVariable");
  }

  doc.save_file("modelDescription.xml");
}

int main(int argc, const char *argv[]) {
  CLI::App app{"Spawn of EnergyPlus"};

  std::string jsoninput = "spawn.json";
  auto createOption =
      app.add_option("-c,--create", jsoninput,
                     "Create a standalone FMU based on json input", true);

  CLI11_PARSE(app, argc, argv);

  if (*createOption) {
    createHandler(jsoninput);
  }

  return 0;
}

