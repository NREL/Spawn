#ifndef spawn_fmu_sim_INCLUDED
#define spawn_fmu_sim_INCLUDED

#include "../fmu/fmu.hpp"
#include "../fmu/modeldescription.hpp"
#include "../util/filesystem.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

namespace spawn::fmu {

struct Simulate
{
  void operator()() const;

  spawn_fs::path fmu_path;
  double start{0.0};
  double stop{60.0};
  double step{0.001};
};

} // namespace spawn::fmu
#endif
