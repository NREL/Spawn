#ifndef OPEN_MODELICA_ENGINE_HPP_INCLUDED
#define OPEN_MODELICA_ENGINE_HPP_INCLUDED

#include "modelica/modelica_engine.hpp"
#include <spdlog/common.h>

struct threadData_s;

namespace spawn::openmodelica {

class OpenModelicaEngine : modelica::ModelicaEngine
{
public:
  OpenModelicaEngine();
  OpenModelicaEngine(OpenModelicaEngine &&) = delete;
  OpenModelicaEngine(const OpenModelicaEngine &) = delete;
  OpenModelicaEngine &operator=(OpenModelicaEngine &&) = delete;
  OpenModelicaEngine &operator=(const OpenModelicaEngine &) = delete;
  ~OpenModelicaEngine() override = default;

  // Compile moInput to a Functional Mockup Unit
  void create_fmu(const std::string_view mo_input,
                  const spawn_fs::path &output_dir,
                  const std::string_view modelica_path,
                  const std::vector<spawn_fs::path> &modelica_files,
                  const spawn::fmu::FMUType &fmu_type) override;

  // Compile moInput to an executable
  void create_exe(const std::string_view mo_input,
                  const spawn_fs::path &output_dir,
                  const std::string_view modelica_path,
                  const std::vector<spawn_fs::path> &modelica_files) override;

  // Compile and simulate a model
  void simulate(const std::string_view model,
                const std::string_view modelica_path,
                const std::vector<spawn_fs::path> &modelica_files,
                const double &start_time,
                double stop_time,
                unsigned number_of_intervals,
                const double &tolerance,
                const std::string_view method) override;

  // Evaluate command using the OpenModelica script engine
  std::string eval(std::string_view command);

private:
  void load_file(const spawn_fs::path &file);
  void load_mbl();
  bool mbl_is_loaded();
  void clear_messages();
  void log_last_message();
  static spdlog::level::level_enum log_level(const std::string_view log_message);

  threadData_s *thread_data;
};

} // namespace spawn::openmodelica

#endif // OPEN_MODELICA_ENGINE_HPP_INCLUDED
