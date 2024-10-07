#ifndef SPAWN_CLI_CONFIG_HXX_INCLUDED
#define SPAWN_CLI_CONFIG_HXX_INCLUDED

#include "../util/filesystem.hpp"
#include <string>

namespace spawn::cli {

bool is_installed();

spawn_fs::path exe_path();

spawn_fs::path exe_dir();

spawn_fs::path idd_path();

spawn_fs::path epfmi_path();

} // namespace spawn::cli

#endif // SPAWN_CLI_CONFIG_HXX_INCLUDED
