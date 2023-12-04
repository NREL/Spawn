#ifndef CLI_CLI_HPP_INCLUDED
#define CLI_CLI_HPP_INCLUDED

#include "energyplus_coroutine/actuator_types.hpp"
#include "energyplus_coroutine/create_fmu.hpp"
#include "energyplus_coroutine/output_types.hpp"
#include "fmu/simulate.hpp"
#include "modelica/create_exe.hpp"
#include "modelica/create_fmu.hpp"
#include "modelica/simulate.hpp"
#include <CLI/CLI.hpp>

namespace spawn::cli {

class CLI
{
public:
  CLI();

  void parse(int argc, const char *argv[]); // NOLINT

private:
  void main_command();
  void modelica_command();
  void energyplus_command();
  void cc_command();
  void fmu_command();

  ::CLI::App app{"Spawn of EnergyPlus"};

  spawn::modelica::CreateFMU modelica_create_fmu;
  spawn::modelica::CreateEXE modelica_create_exe;
  spawn::modelica::Simulate modelica_simulate;
  spawn::energyplus::CreateFMU energyplus_create_fmu;
  spawn::energyplus::ListOutputTypes list_output_types;
  spawn::energyplus::ListActuatorTypes list_actuator_types;
  spawn::fmu::Simulate fmu_simulate;
};

} // namespace spawn::cli

#endif // CLI_CLI_HPP_INCLUDED
