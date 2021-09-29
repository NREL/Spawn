#include "simulate.hpp"
#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include <iostream>
#include <fstream>

namespace spawn {

void simulate(const fs::path & fmupath, const nlohmann::json options) {
  std::cout << "Simulating " << fmupath << std::endl;

  double start = 0.0;
  double stop = 1.0;
  double step = 0.001;
  bool requireAllSymbols = false;

  //spawn::fmu::FMU fmu(fmupath, false, fs::current_path());
  spawn::fmu::FMU fmu(fmupath, requireAllSymbols);

  const auto resource_path = fmu.extractedFilesPath() / "resources";
  const auto resource_url = std::string("file://") + resource_path.string();
  const auto model_description_path = fmu.extractedFilesPath() / fmu.modelDescriptionPath();
  spawn::fmu::ModelDescription modelDescription(model_description_path);

  const auto guid = modelDescription.guid();
  fmi2CallbackFunctions callbacks = {fmuStdOutLogger, calloc, free, NULL, NULL}; // called by the model during simulation
  const auto comp = fmu.fmi.fmi2Instantiate("test-instance", fmi2CoSimulation, guid.c_str(), resource_url.c_str(), &callbacks, false, true);

  if (!comp) throw std::runtime_error("Could not instantiate FMU");

  const auto tolerance = modelDescription.defaultTolerance();

  auto flag = fmu.fmi.fmi2SetupExperiment(comp, fmi2True, tolerance, start, fmi2True, stop);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI setupExperiment");

  flag = fmu.fmi.fmi2EnterInitializationMode(comp);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI enterInitializationMode");

  flag = fmu.fmi.fmi2ExitInitializationMode(comp);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI exitInitializationMode");

  const auto vars = modelDescription.variables();
  std::vector<fmi2ValueReference> valueReferences;

  // Open a csv file for output
  // This is MVP. Consider far more advanced output strategies.
  // Apache Arrow has been mentioned which would support csv and Parquet format,
  // which people seem to like
  std::fstream csvout(fmu.modelIdentifier() + ".csv", std::fstream::out | std::fstream::trunc);
  csvout << "time,";
  for (const auto & var : vars) {
    csvout << var.first << ",";
    valueReferences.push_back(var.second);
  }
  csvout << "\n";
  std::vector<double> values(valueReferences.size());

  double time = start;
  while (time < stop) {
    if (step > stop - time) {
      step = stop - time;
    }

    // Get variable values and write to file
    flag = fmu.fmi.fmi2GetReal(comp, valueReferences.data(), vars.size(), values.data());
    if (flag == fmi2OK) {
      csvout << time << ",";
      for (const auto & value : values) {
        csvout << value << ",";
      }
      csvout << "\n";
    }

    flag = fmu.fmi.fmi2DoStep(comp, time, step, fmi2True);
    if (flag > fmi2Warning) throw std::runtime_error("Could not FMI doStep");
    time += step;
  }

  // Close everything
  fmu.fmi.fmi2Terminate(comp);
  fmu.fmi.fmi2FreeInstance(comp);
  csvout.close();

  std::cout << "Simulation completed successfully " << std::endl;
}

} // namespace spawn
