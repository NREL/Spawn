#include "fmugenerator.hpp"
#include "idf_to_json.hpp"
#include "idfprep.hpp"
#include "../util/fmi_paths.hpp"
#include "../util/unique_id.hpp"
#include "input/input.hpp"
#include "modelDescription.xml.hpp"
#include "ziputil.hpp"
#include <pugixml.hpp>

using json = nlohmann::json;

namespace spawn {

void createModelDescription(const spawn::Input &input, const spawn_fs::path &savepath, const std::string &id);
void copyIDFResourceFiles(const json &jsonidf, const spawn_fs::path &from, const spawn_fs::path &to);

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
  const auto fmuidfPath = fmuResourcesPath / (input.idfInputPath().stem().string() + ".spawn.idf");
  const auto fmuepwPath = fmuResourcesPath / input.epwInputPath().filename();
  const auto fmuiddPath = fmuResourcesPath / iddpath.filename();
  const auto fmuEPFMIPath = fmuStagingPath / fmi_lib_path(id);

  spawn_fs::remove_all(fmuPath);
  spawn_fs::remove_all(fmuStagingPath);

  // Create fmu staging area and copy files into it
  spawn_fs::create_directories(fmuStagingPath);
  spawn_fs::create_directories(fmuResourcesPath);
  spawn_fs::create_directories(fmuEPFMIPath.parent_path());

  auto idfjson = idf_to_json(input.idfInputPath());
  prepare_idf(idfjson, input);
  copyIDFResourceFiles(idfjson, input.idfInputPath().parent_path(), fmuResourcesPath);
  json_to_idf(idfjson, fmuidfPath);

  spawn_fs::copy_file(epfmupath, fmuEPFMIPath, spawn_fs::copy_options::overwrite_existing);
  spawn_fs::copy_file(iddpath, fmuiddPath, spawn_fs::copy_options::overwrite_existing);
  spawn_fs::copy_file(input.epwInputPath(), fmuepwPath, spawn_fs::copy_options::overwrite_existing);

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

spawn_fs::path findIDFResourceFile(const spawn_fs::path &p, const spawn_fs::path &base)
{
  // 1. Look in the base path directory (e.g. same directory as the idf)
  auto result = find_recursive(p, base);

  if (result.empty()) {
    // 2. Look one level up to account for Optimica generated subdirectories (e.g. resources/1/foo.idf)
    const auto candidate = base.parent_path();
    if (candidate.filename() == "resources") {
      result = find_recursive(p, candidate);
    }
  }

  return result;
}

void copyIDFResourceFiles(const json &jsonidf, const spawn_fs::path &from, const spawn_fs::path &to)
{
  // The purpose of this function is to copy resources (CSV files used by Schedule:File),
  // into the generated FMU.
  // Consider that the the idfInputPath may itself by located within the/an "outer" FMU

  // Identify csv files used by Schedule:File input objects within the idf
  const auto schedules = jsonidf.value("Schedule:File", nlohmann::json());

  for (const auto &[name, fields] : schedules.items()) {
    const auto file = fields.find("file_name");
    const auto resource = findIDFResourceFile(file->get<std::string>(), from);
    spawn_fs::copy_file(resource, to / resource.filename(), spawn_fs::copy_options::skip_existing);
  }
}

} // namespace spawn
