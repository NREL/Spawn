#ifndef MODELICA_SIMULATE_HH_INCLUDED
#define MODELICA_SIMULATE_HH_INCLUDED

#include "modelica/modelica_engine.hpp"
#include "util/filesystem.hpp"
#include <iostream>
#include <string>
#include <vector>
namespace spawn::modelica {

struct Simulate
{
  void operator()() const;

  std::string model;
  std::string modelica_path;
  std::vector<spawn_fs::path> modelica_files;
  double start_time{0.0};
  double stop_time{31536000.0};
  unsigned number_of_intervals{500};
  double tolerance{1e-6};
  std::string method{"dassl"};
};

} // namespace spawn::modelica

#endif // MODELICA_SIMULATE_HH_INCLUDED
