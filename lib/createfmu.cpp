#include "createfmu.hpp"

int createFMU(const std::string &jsoninput, bool nozip, bool nocompress) {
  spawn::Input input(jsoninput);

  // We are going to copy the required files into an FMU staging directory,
  // also copy the json input file into root of the fmu staging directory,
  // and rewrite the json input file paths so that they reflect the new paths
  // contained within the fmu layout.

  // FMU staging paths
  const auto fmuPath = input.basepath() / (input.fmuBaseName() + ".fmu");
  const auto fmuStagingPath = input.basepath() / input.fmuBaseName();
  const auto modelDescriptionPath = fmuStagingPath / "modelDescription.xml";
  const auto fmuResourcesPath = fmuStagingPath / "resources";
  const auto fmuidfPath = fmuResourcesPath / input.idfInputPath().filename();
  const auto fmuepwPath = fmuResourcesPath / input.epwInputPath().filename();
  const auto fmuiddPath = fmuResourcesPath / iddInstallPath().filename();
  const auto fmuspawnPath = fmuStagingPath / "model.spawn";
  const auto fmuEPFMIPath = fmuStagingPath / ("binaries/" + fmiplatform() + "/" + epfmiName());

  boost::filesystem::remove_all(fmuPath);
  boost::filesystem::remove_all(fmuStagingPath);

  // Create fmu staging area and copy files into it
  boost::filesystem::create_directories(fmuStagingPath);
  boost::filesystem::create_directories(fmuResourcesPath);
  boost::filesystem::create_directories(fmuEPFMIPath.parent_path());

  boost::filesystem::copy_file(epfmiInstallPath(), fmuEPFMIPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(iddInstallPath(), fmuiddPath, boost::filesystem::copy_option::overwrite_if_exists);
  boost::filesystem::copy_file(input.epwInputPath(), fmuepwPath, boost::filesystem::copy_option::overwrite_if_exists);

  auto jsonidf = spawn::idfToJSON(input.idfInputPath());
  adjustSimulationControl(jsonidf);
  addRunPeriod(jsonidf);
  removeUnusedObjects(jsonidf);
  addOutputVariables(jsonidf, input.outputVariables);
  spawn::jsonToIdf(jsonidf, fmuidfPath);

  createModelDescription(input, modelDescriptionPath);

  const auto relativeEPWPath = boost::filesystem::relative(fmuepwPath, fmuStagingPath);
  input.setEPWInputPath(relativeEPWPath);
  const auto relativeIdfPath = boost::filesystem::relative(fmuidfPath, fmuStagingPath);
  input.setIdfInputPath(relativeIdfPath);
  input.save(fmuspawnPath);

  if (! nozip) {
    zip_directory(fmuStagingPath.string(), fmuPath.string(), nocompress);
  }

  return 0;
}

