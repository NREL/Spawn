#ifndef spawn_util_paths_hh_INCLUDED
#define spawn_util_paths_hh_INCLUDED

#include "filesystem.hpp"

namespace spawn {
bool is_installed();

fs::path exe();

fs::path exe_dir();

fs::path mbl_home_dir();

fs::path idd_install_path();

fs::path epfmi_install_path();

fs::path msl_path();

fs::path project_source_dir();

fs::path project_binary_dir();

fs::path idd_path();
} // namespace spawn

#endif
