#include "simulate.hpp"
#include "open_modelica/open_modelica_engine.hpp"

namespace spawn::modelica {

void Simulate::operator()() const
{
  spawn::openmodelica::OpenModelicaEngine open_modelica_engine;

  auto output_dir = spawn_fs::current_path();

  open_modelica_engine.simulate(
      model, modelica_path, modelica_files, start_time, stop_time, number_of_intervals, tolerance, method);
}

} // namespace spawn::modelica
