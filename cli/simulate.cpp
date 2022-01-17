#include "simulate.hpp"
#include "../fmu/fmu.hpp"
#include "../fmu/logger.h"
#include "../fmu/modeldescription.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace spawn::fmu {

Sim::Sim(spawn_fs::path fmu_path) : m_fmu_path(std::move(fmu_path))
{
}

std::vector<FMU::Variable> Sim::initContinuousVariables()
{
  auto vars = m_fmu.getVariables();
  const auto end = std::remove_if(vars.begin(), vars.end(), [](const FMU::Variable &v) {
    return (v.variability != FMU::Variable::Variability::Continuous);
  });
  return {vars.begin(), end};
}

std::vector<fmi2ValueReference> Sim::initValueReferences()
{
  std::vector<fmi2ValueReference> vrs(m_continous_vars.size());
  std::transform(m_continous_vars.begin(), m_continous_vars.end(), vrs.begin(), [](const FMU::Variable &v) {
    return v.valueReference;
  });

  return vrs;
}

void Sim::run(const nlohmann::json &config)
{
  std::cout << "Simulating " << m_fmu_path << std::endl;

  double start = config["start"].get<double>();
  double stop = config["stop"].get<double>();
  double step = config["step"].get<double>();

  const auto guid = m_fmu.guid();
  fmi2CallbackFunctions callbacks = {
      fmuStdOutLogger, calloc, free, nullptr, nullptr}; // called by the model during simulation
  const auto comp = m_fmu.fmi.fmi2Instantiate(
      "test-instance", fmi2CoSimulation, guid.c_str(), m_resource_url.c_str(), &callbacks, false, true);
  if (!comp) throw std::runtime_error("Could not instantiate FMU");

  const auto tolerance = m_fmu.defaultTolerance();
  auto flag = m_fmu.fmi.fmi2SetupExperiment(comp, fmi2True, tolerance, start, fmi2True, stop);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI setupExperiment");

  flag = m_fmu.fmi.fmi2EnterInitializationMode(comp);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI enterInitializationMode");

  flag = m_fmu.fmi.fmi2ExitInitializationMode(comp);
  if (flag > fmi2Warning) throw std::runtime_error("Could not FMI exitInitializationMode");

  openLogs();

  double time = start;
  writeLogs(comp, time);
  while (time < stop) {
    if (step > stop - time) {
      step = stop - time;
    }
    flag = m_fmu.fmi.fmi2DoStep(comp, time, step, fmi2True);
    if (flag > fmi2Warning) throw std::runtime_error("Could not FMI doStep");
    writeLogs(comp, time);
    time += step;
  }

  // Close everything
  m_fmu.fmi.fmi2Terminate(comp);
  m_fmu.fmi.fmi2FreeInstance(comp);
  closeLogs();

  std::cout << "Simulation completed successfully " << std::endl;
}

void Sim::openLogs()
{
  m_csvout.open(m_fmu.modelIdentifier() + ".csv", std::fstream::out | std::fstream::trunc);
  m_csvout << "time,";
  for (const auto &var : m_continous_vars) {
    m_csvout << var.name << ",";
  }
  m_csvout << "\n";
}

void Sim::writeLogs(void *comp, const double &time)
{
  const auto flag = m_fmu.fmi.fmi2GetReal(comp, m_var_references.data(), m_var_references.size(), m_var_values.data());
  if (flag == fmi2OK) {
    m_csvout << time << ",";
    for (const auto &value : m_var_values) {
      m_csvout << value << ",";
    }
    m_csvout << "\n";
  }
}

void Sim::closeLogs()
{
  m_csvout.close();
}

} // namespace spawn::fmu
