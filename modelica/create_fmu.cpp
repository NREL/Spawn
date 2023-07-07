#include "create_fmu.hpp"
#if defined ENABLE_MODELICA_COMPILER
#include "optimica/optimica_engine.hpp"
#endif

namespace spawn::modelica {

void CreateFMU::operator()() const
{
#if defined ENABLE_MODELICA_COMPILER
  spawn::optimica::OptimicaEngine optimica_engine;

  auto output_dir = spawn_fs::current_path();
  [[maybe_unused]] auto path = optimica_engine.create_fmu(model, output_dir, modelica_path, fmu::toFMUType(fmu_type));
#endif
}

} // namespace spawn::modelica
