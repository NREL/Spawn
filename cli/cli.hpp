#ifndef CLI_CLI_HPP_INCLUDED
#define CLI_CLI_HPP_INCLUDED

#include "energyplus/actuator_types.hpp"
#include "energyplus/create_fmu.hpp"
#include "energyplus/output_types.hpp"
#include "fmu/simulate.hpp"
#include "modelica/create_fmu.hpp"
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
  spawn::energyplus::CreateFMU energyplus_create_fmu;
  spawn::energyplus::ListOutputTypes list_output_types;
  spawn::energyplus::ListActuatorTypes list_actuator_types;
  spawn::fmu::Simulate fmu_simulate;
};

} // namespace spawn::cli

#endif // CLI_CLI_HPP_INCLUDED
