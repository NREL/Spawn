#include "simulate.hpp"
#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace spawn::fmu {

void Simulate::operator()() const
{
  FMU fmu{fmu_path, false};

  spawn_fs::path resource_path{fmu.extractedFilesPath() / "resources"};
  std::string resource_url{std::string("file://") + resource_path.string()};

  // All FMU variables
  auto all_vars = fmu.getVariables();

  // All continuous FMU variables
  const auto cont_vars_end = std::remove_if(all_vars.begin(), all_vars.end(), [](const FMU::Variable &v) {
    return (v.variability != FMU::Variable::Variability::Continuous);
  });
  const decltype(all_vars) cont_vars = {all_vars.begin(), cont_vars_end};

  // Continuous variable value references
  std::vector<fmi2ValueReference> value_refs(cont_vars.size());
  std::transform(
      cont_vars.begin(), cont_vars.end(), value_refs.begin(), [](const FMU::Variable &v) { return v.valueReference; });

  // Container for the continous variable's values
  std::vector<double> values{value_refs.size(), 0.0, std::allocator<double>()};

  // Open output file
  std::fstream csvout;
  csvout.open(fmu.modelIdentifier() + ".csv", std::fstream::out | std::fstream::trunc);
  csvout << "time,";
  for (const auto &var : cont_vars) {
    csvout << var.name << ",";
  }
  csvout << "\n";

  // Function to write current variable values to file
  auto writeLogs = [&](void *comp, const double &time) {
    const auto flag = fmu.fmi.fmi2GetReal(comp, value_refs.data(), value_refs.size(), values.data());
    if (flag == fmi2OK) {
      csvout << time << ",";
      for (const auto &value : values) {
        csvout << value << ",";
      }
      csvout << "\n";
    }
  };

  // Start simulation by going through initialization sequence
  std::cout << "Simulating " << fmu_path << std::endl;

  const auto guid = fmu.guid();
  fmi2CallbackFunctions callbacks = {
      fmuStdOutLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation

  const auto comp = fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2CoSimulation, guid.c_str(), resource_url.c_str(), &callbacks, false, true);
  if (comp == nullptr) {
    throw std::runtime_error("Could not instantiate FMU");
  }

  const auto tolerance = fmu.defaultTolerance();
  auto flag = fmu.fmi.fmi2SetupExperiment(comp, fmi2True, tolerance, start, fmi2True, stop);
  if (flag > fmi2Warning) {
    throw std::runtime_error("Could not FMI setupExperiment");
  }

  flag = fmu.fmi.fmi2EnterInitializationMode(comp);
  if (flag > fmi2Warning) {
    throw std::runtime_error("Could not FMI enterInitializationMode");
  }

  flag = fmu.fmi.fmi2ExitInitializationMode(comp);
  if (flag > fmi2Warning) {
    throw std::runtime_error("Could not FMI exitInitializationMode");
  }

  double time = start;
  // current_step may be modified relative to the original user input
  // so that we don't advance beyond the stop time
  auto current_step_size = step;

  writeLogs(comp, time);

  // Iterate through time
  while (time < stop) {
    if (current_step_size > stop - time) {
      current_step_size = stop - time;
    }
    flag = fmu.fmi.fmi2DoStep(comp, time, current_step_size, fmi2True);
    if (flag > fmi2Warning) {
      throw std::runtime_error("FMI 'doStep' failed");
    }
    writeLogs(comp, time);
    time += step;
  }

  // Close everything
  fmu.fmi.fmi2Terminate(comp);
  fmu.fmi.fmi2FreeInstance(comp);
  csvout.close();

  std::cout << "Simulation completed successfully " << std::endl;
}

} // namespace spawn::fmu
