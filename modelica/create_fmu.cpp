#include "create_fmu.hpp"
#if defined ENABLE_MODELICA_COMPILER
#include "optimica/optimica_engine.hpp"
#endif
#include "open_modelica/open_modelica_engine.hpp"

namespace spawn::modelica {

void CreateFMU::operator()() const
{
  if (optimica) {
#if defined ENABLE_MODELICA_COMPILER
    spawn::optimica::OptimicaEngine optimica_engine;

    auto output_dir = spawn_fs::current_path();
    [[maybe_unused]] auto path =
        optimica_engine.create_fmu(model, output_dir, modelica_path, {}, fmu::toFMUType(fmu_type));
#endif
  } else {
    spawn::openmodelica::OpenModelicaEngine open_modelica_engine;

    auto output_dir = spawn_fs::current_path();
    [[maybe_unused]] auto path =
        open_modelica_engine.create_fmu(model, output_dir, modelica_path, modelica_files, fmu::toFMUType(fmu_type));
  }
}

} // namespace spawn::modelica
