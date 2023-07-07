#include "cli/cli.hpp"
#include "energyplus/config.hpp"
#include "modelica/create_fmu.hpp"
#include "util/config.hpp"
#include <spdlog/spdlog.h>

#if defined ENABLE_MODELICA_COMPILER
#include "optimica/optimica_engine.hpp"
#endif

namespace spawn::cli {

CLI::CLI()
{
  main_command();
  modelica_command();
  energyplus_command();
  cc_command();
  fmu_command();
}

void CLI::parse(int argc, const char *argv[]) // NOLINT
{
  try {
    app.parse(argc, argv);
  } catch (const ::CLI::ParseError &e) {
    app.exit(e);
  }
}

void CLI::main_command()
{
  app.set_help_all_flag("-H, --expanded-help", "Show expanded help for all subcommands");

  auto print_version = []() { std::cout << "Spawn-" << spawn::version_string() << std::endl; };
  app.add_flag_callback("-v,--version", print_version, "Print version info and exit");

  auto make_verbose = []() { spdlog::set_level(spdlog::level::trace); };
  app.add_flag_callback("--verbose", make_verbose, "Use verbose logging");
}

void CLI::modelica_command()
{
#if defined ENABLE_MODELICA_COMPILER
  // Top level Modelica command
  auto modelica_command = app.add_subcommand("modelica", "Subcommand for Modelica operations");

  // Main Modelica options
  modelica_command->add_option("--modelica-path",
                               modelica_create_fmu.modelica_path,
                               "Define the Modelica search path, using ':' as a path separator");
  modelica_command->add_flag("--optimica", modelica_create_fmu.optimica, "Use Optimica compiler");

  // Modelica create FMU sub command
  auto create_fmu_command = modelica_command->add_subcommand("create-fmu", "Compile Modelica model to FMU format");
  create_fmu_command->add_option("MODEL", modelica_create_fmu.model, "Modelica model path");
  create_fmu_command->add_option("--fmu-type", modelica_create_fmu.fmu_type, "FMU Type, CS or ME");
  create_fmu_command->callback(std::ref(modelica_create_fmu));
#endif
}

void CLI::energyplus_command()
{
  // Top level energyplus command
  auto energyplus_command = app.add_subcommand("energyplus", "Subcommand for EnergyPlus related operations");

  // EnergyPlus version option
  auto version_callback = []() { std::cout << spawn::energyplus::version_string() << std::endl; };
  energyplus_command->add_flag_callback(
      "-v, --version", version_callback, "Print version info about the embedded EnergyPlus software");

  // EnergyPLus create FMU sub command
  constexpr auto create_fmu_doc = "Create a standalone FMU based on json input";
  auto create_fmu_command = energyplus_command->add_subcommand("create-fmu", create_fmu_doc);

  create_fmu_command->add_option("INPUT_FILE_PATH", energyplus_create_fmu.input_path, "Spawn input file");

  constexpr auto output_path_doc = "Full path including filename and extension where the fmu should be placed";
  create_fmu_command->add_option("--output-path", energyplus_create_fmu.output_path, output_path_doc);

  constexpr auto output_dir_doc = "Directory where the fmu should be placed. This path will be created if necessary";
  create_fmu_command->add_option("--output-dir", energyplus_create_fmu.output_dir, output_dir_doc);

  constexpr auto no_zip_doc = "Stage FMU files on disk without creating a zip archive";
  create_fmu_command->add_flag("--no-zip", energyplus_create_fmu.no_zip, no_zip_doc);

  constexpr auto no_compress_doc =
      "Skip compressing the contents of the fmu zip archive. An uncompressed zip archive will be created instead";
  create_fmu_command->add_flag("--no-compress", energyplus_create_fmu.no_compress, no_compress_doc);

  energyplus_create_fmu.epfmu_path = spawn::epfmi_install_path();
  energyplus_create_fmu.idd_path = spawn::idd_install_path();

  create_fmu_command->callback(std::ref(energyplus_create_fmu));

  // List output types sub command
  constexpr auto output_vars_doc = "Report the EnergyPlus output variables supported by this version of Spawn";
  auto output_vars_command = energyplus_command->add_subcommand("list-output-variables", output_vars_doc);
  output_vars_command->callback(std::ref(list_output_types));

  // List actuator types sub command
  spawn::energyplus::ListActuatorTypes list_actuator_types;
  constexpr auto actuators_doc = "Report the EnergyPlus output variables supported by this version of Spawn";
  auto actuators_command = energyplus_command->add_subcommand("list-actuators", actuators_doc);
  actuators_command->callback(std::ref(list_actuator_types));
}

void CLI::cc_command()
{
  auto cc_command = app.add_subcommand("cc", "Subcommand for C compiler with a 'make' style interface");
  cc_command->allow_extras();
  auto callback = [&cc_command]() {
    std::cout << "cli cc_command" << std::endl;
    auto optimica = spawn::optimica::OptimicaEngine();
    optimica.make_external_function(cc_command->remaining(true));
  };
  cc_command->callback(callback);
}

void CLI::fmu_command()
{
  // Top level FMU command
  auto fmu_command = app.add_subcommand("fmu", "Subcommand for FMU related operations");

  // Simulate FMU sub command
  auto simulate_command = fmu_command->add_subcommand("simulate", "Simulate an existing FMU");

  simulate_command->add_option("--start", fmu_simulate.start, "Simulation start time");
  simulate_command->add_option("--stop", fmu_simulate.stop, "Simulation stop time");
  simulate_command->add_option("--step", fmu_simulate.step, "Simulation step size");
  simulate_command->add_option("FMU_PATH", fmu_simulate.fmu_path, "FMU path");

  simulate_command->callback(std::ref(fmu_simulate));
}

} // namespace spawn::cli
