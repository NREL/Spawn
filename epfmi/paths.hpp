#ifndef SPAWN_EPFMI_CONFIG_HPP_INCLUDED
#define SPAWN_EPFMI_CONFIG_HPP_INCLUDED

#include "util/filesystem.hpp"

namespace spawn::epfmi {

// These paths are from the perspective of a packaged EnergyPlus FMU.
// This is different than the paths in the spawn::cli namespace, which
// are from the perspective of the installed Spawn cli.
//
// If this/these functions are called outside of a packaged FMU,
// that contains the epfmi library, then the returned paths are probably not accurate.

spawn_fs::path idd_path();

} // namespace spawn::epfmi

#endif // SPAWN_EPFMI_CONFIG_HPP_INCLUDED
