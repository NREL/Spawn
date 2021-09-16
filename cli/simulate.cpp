#include "simulate.hpp"
#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include <iostream>

namespace spawn {

void simulate(const fs::path & fmupath, const nlohmann::json options) {
  std::cout << "Simulating " << fmupath << std::endl;

  double start = 0.0;
  double stop = 1.0;
  double step = 0.001;

  //spawn::fmu::FMU fmu(fmupath, false, fs::current_path());
  spawn::fmu::FMU fmu(fmupath, false);

  const auto resource_path = fmu.extractedFilesPath() / "resources";
  const auto resource_url = std::string("file://") + resource_path.string();
  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  const auto guid = modelDescription.guid();
  fmi2CallbackFunctions callbacks = {fmuStdOutLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2CoSimulation, guid.c_str(), resource_url.c_str(), &callbacks, false, true);

  if (!comp) throw std::runtime_error("Could not instantiate FMU");

  auto flag = fmu.fmi.fmi2SetupExperiment(comp, fmi2True, 1.0E-6, start, fmi2True, stop);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI setupExperiment");

  flag = fmu.fmi.fmi2EnterInitializationMode(comp);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI enterInitializationMode");

  flag = fmu.fmi.fmi2ExitInitializationMode(comp);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI exitInitializationMode");

  //double time = start;
  //while (time < stop) {
  //  if (step > stop - time) {
  //    step = stop - time;
  //  }
  //  flag = fmu.fmi.fmi2DoStep(comp, time, step, fmi2True);
  //  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI doStep");
  //  time += step;
  //}

  //fmu.fmi.fmi2Terminate(comp);
  fmu.fmi.fmi2FreeInstance(comp);

  std::cout << "Simulation completed successfully " << std::endl;

  //defaultExp = getDefaultExperiment(md);
  //if (defaultExp) tolerance = getAttributeDouble(defaultExp, att_tolerance, &vs);
  //if (vs == valueDefined) {
  //    toleranceDefined = fmi2True;
  //}

  //fmi2Flag = fmu->setupExperiment(c, toleranceDefined, tolerance, tStart, fmi2True, tEnd);
  //if (fmi2Flag > fmi2Warning) {
  //    return error("could not initialize model; failed FMI setup experiment");
  //}
}

} // namespace spawn
