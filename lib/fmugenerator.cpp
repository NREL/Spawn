#include "fmugenerator.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/DataStringGlobals.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/EmbeddedEpJSONSchema.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../submodules/EnergyPlus/src/EnergyPlus/UtilityRoutines.hh"
#include "../submodules/EnergyPlus/third_party/nlohmann/json.hpp"
#include "../util/compare.hpp"
#include "../util/filesystem.hpp"
#include "../util/fmi_paths.hpp"
#include "idf_to_json.hpp"
#include "idfprep.hpp"
#include "input/input.hpp"
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include <fmt/format.h>
#include <pugixml.hpp>

using json = nlohmann::json;

namespace spawn {

void createModelDescription(const spawn::Input &input, const fs::path &savepath);

void energyplusToFMU(const std::string &jsoninput,
                     bool nozip,
                     bool nocompress,
                     const std::string &outputpath,
                     const std::string &outputdir,
                     const fs::path &iddpath,
                     const fs::path &epfmupath)
{
  spawn::Input input(jsoninput);

  // We are going to copy the required files into an FMU staging directory,
  // also copy the json input file into root of the fmu staging directory,
  // and rewrite the json input file paths so that they reflect the new paths
  // contained within the fmu layout.

  // The default fmu output path
  auto fmuPath = fs::current_path() / (input.fmuBaseName() + ".fmu");

  // These are options to override the default output path
  if (!outputpath.empty()) {
    fmuPath = fs::path(outputpath);
  } else if (!outputdir.empty()) {
    fmuPath = fs::path(outputdir) / (input.fmuBaseName() + ".fmu");
  }

  const auto outputroot = fmuPath.parent_path();
  const auto fmuStagingPath = outputroot / fmuPath.stem();

  if (!fs::exists(outputroot)) {
    fs::create_directories(outputroot);
  }

  const auto modelDescriptionPath = fmuStagingPath / "modelDescription.xml";
  const auto fmuResourcesPath = fmuStagingPath / "resources";
  const auto fmuspawnPath = fmuResourcesPath / "model.spawn";
  const auto fmuidfPath = fmuResourcesPath / input.idfInputPath().filename();
  const auto fmuepwPath = fmuResourcesPath / input.epwInputPath().filename();
  const auto fmuiddPath = fmuResourcesPath / iddpath.filename();
  const auto fmuEPFMIPath = fmuStagingPath / fmi_lib_path(epfmi_basename());

  fs::remove_all(fmuPath);
  fs::remove_all(fmuStagingPath);

  // Create fmu staging area and copy files into it
  fs::create_directories(fmuStagingPath);
  fs::create_directories(fmuResourcesPath);
  fs::create_directories(fmuEPFMIPath.parent_path());

  fs::copy_file(epfmupath, fmuEPFMIPath, fs::copy_options::overwrite_existing);
  fs::copy_file(iddpath, fmuiddPath, fs::copy_options::overwrite_existing);
  fs::copy_file(input.epwInputPath(), fmuepwPath, fs::copy_options::overwrite_existing);
  fs::copy_file(input.idfInputPath(), fmuidfPath);

  createModelDescription(input, modelDescriptionPath);

  const auto relativeEPWPath = fs::relative(fmuepwPath, fmuResourcesPath);
  input.setEPWInputPath(relativeEPWPath);
  const auto relativeIdfPath = fs::relative(fmuidfPath, fmuResourcesPath);
  input.setIdfInputPath(relativeIdfPath);
  input.save(fmuspawnPath);

  if (!nozip) {
    zip_directory(fmuStagingPath.string(), fmuPath.string(), nocompress);
    fs::remove_all(fmuStagingPath);
  }
}

void createModelDescription(const spawn::Input &input, const fs::path &savepath)
{
  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto xmlvariables = doc.child("fmiModelDescription").child("ModelVariables");

  const auto variables = parseVariables(input);

  for (const auto &varpair : variables) {
    const auto var = varpair.second;

    auto scalarVar = xmlvariables.append_child("ScalarVariable");
    for (const auto &attribute : var.scalar_attributes) {
      scalarVar.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
    }

    auto real = scalarVar.append_child("Real");
    for (const auto &attribute : var.real_attributes) {
      real.append_attribute(attribute.first.c_str()) = attribute.second.c_str();
    }
  }

  doc.save_file(savepath.c_str());
}

} // namespace spawn
