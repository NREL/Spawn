#include "create_fmu.hpp"
#include "open_modelica/open_modelica_engine.hpp"

namespace spawn::modelica {

void CreateFMU::operator()() const
{
  spawn::openmodelica::OpenModelicaEngine open_modelica_engine;

  auto output_dir = spawn_fs::current_path();
  [[maybe_unused]] auto path =
      open_modelica_engine.create_fmu(model, output_dir, modelica_path, modelica_files, fmu::toFMUType(fmu_type));
}

} // namespace spawn::modelica
