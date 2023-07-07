#ifndef MODELICA_CREATE_FMU_HH_INCLUDED
#define MODELICA_CREATE_FMU_HH_INCLUDED

#include "fmu/fmu_type.hpp"
#include "util/filesystem.hpp"
#include <iostream>
#include <string>
#include <vector>
namespace spawn::modelica {

struct CreateFMU
{
  void operator()() const;

  std::vector<spawn_fs::path> modelica_path;
  bool optimica{false};
  std::string model;
  std::string fmu_type{"CS"};
};

} // namespace spawn::modelica

#endif // MODELICA_CREATE_FMU_HH_INCLUDED
