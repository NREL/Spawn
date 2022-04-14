#include "fmugenerator.hpp"
#include "../submodules/EnergyPlus/src/EnergyPlus/InputProcessing/IdfParser.hh"
#include "../util/fmi_paths.hpp"
#include "../util/unique_id.hpp"
#include "input/input.hpp"
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include <pugixml.hpp>

using json = nlohmann::json;

namespace spawn {

void createModelDescription(const spawn::Input &input, const spawn_fs::path &savepath, const std::string &id);

void energyplusToFMU(const std::string &jsoninput,
                     bool nozip,
                     bool nocompress,
                     const std::string &outputpath,
                     const std::string &outputdir,
                     const spawn_fs::path &iddpath,
                     const spawn_fs::path &epfmupath)
{
  spawn::Input input(jsoninput);

  // We are going to copy the required files into an FMU staging directory,
  // also copy the json input file into root of the fmu staging directory,
  // and rewrite the json input file paths so that they reflect the new paths
  // contained within the fmu layout.

  // The default fmu output path
  auto fmuPath = spawn_fs::current_path() / (input.fmuBaseName() + ".fmu");

  // These are options to override the default output path
  if (!outputpath.empty()) {
    fmuPath = spawn_fs::path(outputpath);
  } else if (!outputdir.empty()) {
    fmuPath = spawn_fs::path(outputdir) / (input.fmuBaseName() + ".fmu");
  }

  const auto outputroot = fmuPath.parent_path();
  const auto fmuStagingPath = outputroot / fmuPath.stem();

  if (!spawn_fs::exists(outputroot)) {
    spawn_fs::create_directories(outputroot);
  }

  const auto id = std::string("epfmi_") + util::uniqueId();

  const auto modelDescriptionPath = fmuStagingPath / "modelDescription.xml";
  const auto fmuResourcesPath = fmuStagingPath / "resources";
  const auto fmuspawnPath = fmuResourcesPath / "model.spawn";
  const auto fmuidfPath = fmuResourcesPath / input.idfInputPath().filename();
  const auto fmuepwPath = fmuResourcesPath / input.epwInputPath().filename();
  const auto fmuiddPath = fmuResourcesPath / iddpath.filename();
  const auto fmuEPFMIPath = fmuStagingPath / fmi_lib_path(id);

  spawn_fs::remove_all(fmuPath);
  spawn_fs::remove_all(fmuStagingPath);

  // Create fmu staging area and copy files into it
  spawn_fs::create_directories(fmuStagingPath);
  spawn_fs::create_directories(fmuResourcesPath);
  spawn_fs::create_directories(fmuEPFMIPath.parent_path());

  std::cout << "Generating fmuEPFMIPath: " << fmuEPFMIPath << std::endl;

  spawn_fs::copy_file(epfmupath, fmuEPFMIPath, spawn_fs::copy_options::overwrite_existing);
  spawn_fs::copy_file(iddpath, fmuiddPath, spawn_fs::copy_options::overwrite_existing);
  spawn_fs::copy_file(input.epwInputPath(), fmuepwPath, spawn_fs::copy_options::overwrite_existing);
  spawn_fs::copy_file(input.idfInputPath(), fmuidfPath);

  createModelDescription(input, modelDescriptionPath, id);

  const auto relativeEPWPath = spawn_fs::relative(fmuepwPath, fmuResourcesPath);
  input.setEPWInputPath(relativeEPWPath);
  const auto relativeIdfPath = spawn_fs::relative(fmuidfPath, fmuResourcesPath);
  input.setIdfInputPath(relativeIdfPath);
  input.save(fmuspawnPath);

  if (!nozip) {
    zip_directory(fmuStagingPath.string(), fmuPath.string(), nocompress);
    spawn_fs::remove_all(fmuStagingPath);
  }
}

void createModelDescription(const spawn::Input &input, const spawn_fs::path &savepath, const std::string &id)
{
  pugi::xml_document doc;
  doc.load_string(modelDescriptionXMLText.c_str());

  auto fmiModelDescription = doc.child("fmiModelDescription");
  auto modelExchange = fmiModelDescription.child("ModelExchange");

  fmiModelDescription.attribute("modelName").set_value(id.c_str());
  fmiModelDescription.attribute("guid").set_value(id.c_str());
  modelExchange.attribute("modelIdentifier").set_value(id.c_str());

  auto xmlvariables = fmiModelDescription.child("ModelVariables");
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
