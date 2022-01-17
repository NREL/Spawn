#ifndef spawn_util_paths_hh_INCLUDED
#define spawn_util_paths_hh_INCLUDED

#include "filesystem.hpp"

namespace spawn {
bool is_installed();

spawn_fs::path exe();

spawn_fs::path exe_dir();

spawn_fs::path mbl_home_dir();

spawn_fs::path idd_install_path();

spawn_fs::path epfmi_install_path();

spawn_fs::path msl_path();

spawn_fs::path project_source_dir();

spawn_fs::path project_binary_dir();

spawn_fs::path idd_path();
} // namespace spawn

#endif
