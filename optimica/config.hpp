#ifndef SPAWN_OPTIMICA_CONFIG_HXX_INCLUDED
#define SPAWN_OPTIMICA_CONFIG_HXX_INCLUDED

#include "util/filesystem.hpp"

namespace spawn {

spawn_fs::path msl_path();

std::string gfortranlib_name();

spawn_fs::path gfortranlib_embedded_path();

} // namespace spawn

#endif // SPAWN_OPTIMICA_CONFIG_HXX_INCLUDED
