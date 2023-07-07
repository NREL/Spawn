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
  [[nodiscard]] virtual spawn_fs::path create_fmu(std::string_view moInput,
                                                  const spawn_fs::path &outputDir,
                                                  std::vector<spawn_fs::path> modelicaPaths,
                                                  const spawn::fmu::FMUType &fmuType) = 0;

  // Not Implemented - Compile moInput to an executable
  [[nodiscard]] virtual spawn_fs::path create_exe(std::string_view moInput,
                                                  const spawn_fs::path &outputDir,
                                                  const std::vector<spawn_fs::path> &modelicaPaths,
                                                  const spawn::fmu::FMUType &fmuType) = 0;
};

} // namespace spawn::modelica

#endif // MODELICA_MODELICA_ENGINE_HPP_INCLUDED
