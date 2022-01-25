#ifndef SPAWN_CONFIG_HXX_INCLUDED
#define SPAWN_CONFIG_HXX_INCLUDED

#include "filesystem.hpp"

namespace spawn {

bool is_installed();

std::string version_string();

std::string fmi_platform();

fs::path exe();

fs::path exe_dir();

fs::path mbl_home_dir();

fs::path idd_install_path();

fs::path epfmi_install_path();

fs::path msl_path();

fs::path project_source_dir();

fs::path project_binary_dir();

fs::path idd_path();

std::string gfortranlib_name();

fs::path gfortranlib_embedded_path();

} // namespace spawn

#endif // SPAWN_CONFIG_HXX_INCLUDED
