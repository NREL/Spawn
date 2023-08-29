#ifndef ENERGYPLUS_CREATE_FMU_HH_INCLUDED
#define ENERGYPLUS_CREATE_FMU_HH_INCLUDED

#include "../util/filesystem.hpp"
#include <string>

namespace spawn::energyplus {

struct CreateFMU
{
  void operator()() const;

  spawn_fs::path input_path;
  bool no_zip{false};
  bool no_compress{false};
  spawn_fs::path output_path;
  spawn_fs::path output_dir;
  spawn_fs::path idd_path;
  spawn_fs::path epfmu_path;
};

} // namespace spawn::energyplus

#endif // ENERGYPLUS_CREATE_FMU_HH_INCLUDED
