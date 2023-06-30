#ifndef SPAWN_ENV_VARS_HH_INCLUDED
#define SPAWN_ENV_VARS_HH_INCLUDED

#include "util/filesystem.hpp"
#include <cstdlib>
#include <iterator>
#include <string>
#include <vector>

#ifdef _MSC_VER
#include <Windows.h>
#endif

namespace spawn {

inline void set_env(const std::string &name, const std::string &value)
{
#ifdef _MSC_VER
  SetEnvironmentVariable(name, value);
#else
  setenv(name.c_str(), value.c_str(), 1);
#endif
}

} // namespace spawn

#endif // SPAWN_ENV_VARS_HH_INCLUDED
