#ifndef SPAWN_MBL_CONFIG_HXX_INCLUDED
#define SPAWN_MBL_CONFIG_HXX_INCLUDED

#include "util/filesystem.hpp"
#include <string>

namespace spawn {

spawn_fs::path mbl_home_dir();

spawn_fs::path mbl_fmilib_path();

std::string mbl_energyplus_version_string();

} // namespace spawn

#endif // SPAWN_MBL_CONFIG_HXX_INCLUDED
