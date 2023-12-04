#include "create_exe.hpp"
#include "open_modelica/open_modelica_engine.hpp"

namespace spawn::modelica {

void CreateEXE::operator()() const
{
  spawn::openmodelica::OpenModelicaEngine open_modelica_engine;

  auto output_dir = spawn_fs::current_path();
  open_modelica_engine.create_exe(model, output_dir, modelica_path, modelica_files);
}

} // namespace spawn::modelica
