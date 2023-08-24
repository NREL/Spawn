#ifndef OPEN_MODELICA_ENGINE_HPP_INCLUDED
#define OPEN_MODELICA_ENGINE_HPP_INCLUDED

#include "modelica/modelica_engine.hpp"

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
  [[nodiscard]] spawn_fs::path create_fmu(const std::string_view mo_input,
                                          const spawn_fs::path &output_dir,
                                          const std::vector<spawn_fs::path> &modelica_paths,
                                          const std::vector<spawn_fs::path> &modelica_files,
                                          const spawn::fmu::FMUType &fmu_type) override;

  // Not Implemented - Compile moInput to an executable
  [[nodiscard]] spawn_fs::path create_exe(const std::string_view mo_input,
                                          const spawn_fs::path &output_dir,
                                          const std::vector<spawn_fs::path> &modelica_paths,
                                          const std::vector<spawn_fs::path> &modelica_files,
                                          const spawn::fmu::FMUType &fmu_type) override;

  std::string eval(std::string_view command);

private:
  void load_file(const spawn_fs::path &file);
  void load_mbl();
  bool mbl_is_loaded();

  threadData_s *thread_data;
};

} // namespace spawn::openmodelica

#endif // OPEN_MODELICA_ENGINE_HPP_INCLUDED
