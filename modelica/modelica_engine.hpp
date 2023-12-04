#ifndef MODELICA_MODELICA_ENGINE_HPP_INCLUDED
#define MODELICA_MODELICA_ENGINE_HPP_INCLUDED

#include "fmu/fmu_type.hpp"
#include "util/filesystem.hpp"
#include <string>
#include <vector>

namespace spawn::modelica {

class ModelicaEngine
{
public:
  ModelicaEngine() = default;

  ModelicaEngine(ModelicaEngine &&) = delete;

  ModelicaEngine(const ModelicaEngine &) = delete;

  ModelicaEngine &operator=(ModelicaEngine &&) = delete;

  ModelicaEngine &operator=(const ModelicaEngine &) = delete;

  virtual ~ModelicaEngine() = default;

  // Compile moInput to a Functional Mockup Unit
  virtual void create_fmu(const std::string_view mo_input,
                          const spawn_fs::path &output_dir,
                          const std::string_view modelica_path,
                          const std::vector<spawn_fs::path> &modelica_files,
                          const spawn::fmu::FMUType &fmu_type) = 0;

  // Compile moInput to an executable
  virtual void create_exe(const std::string_view mo_input,
                          const spawn_fs::path &output_dir,
                          const std::string_view modelica_path,
                          const std::vector<spawn_fs::path> &modelica_files) = 0;

  // Compile and simulate a model
  virtual void simulate(const std::string_view model,
                        const std::string_view modelica_path,
                        const std::vector<spawn_fs::path> &modelica_files,
                        const double &start_time,
                        double stop_time,
                        unsigned number_of_intervals,
                        const double &tolerance,
                        const std::string_view method) = 0;
};

} // namespace spawn::modelica

#endif // MODELICA_MODELICA_ENGINE_HPP_INCLUDED
