#ifndef SPAWN_CONFIG_HXX_INCLUDED
#define SPAWN_CONFIG_HXX_INCLUDED

#include "filesystem.hpp"
#include <string>

namespace spawn {

bool is_installed();

std::string version_string();

std::string fmi_platform();

spawn_fs::path exe();

spawn_fs::path exe_dir();

spawn_fs::path epfmi_install_path();

spawn_fs::path project_source_dir();

spawn_fs::path project_binary_dir();

} // namespace spawn

#endif // SPAWN_CONFIG_HXX_INCLUDED
