#ifndef MODELICA_CREATE_EXE_HH_INCLUDED
#define MODELICA_CREATE_EXE_HH_INCLUDED

#include "util/filesystem.hpp"
#include <iostream>
#include <string>
#include <vector>
namespace spawn::modelica {

struct CreateEXE
{
  void operator()() const;

  std::string modelica_path;
  std::vector<spawn_fs::path> modelica_files;
  std::string model;
};

} // namespace spawn::modelica

#endif // MODELICA_CREATE_EXE_HH_INCLUDED
