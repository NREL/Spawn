#ifndef SPAWN_SPAWN_HPP
#define SPAWN_SPAWN_HPP

#include "../util/filesystem.hpp"

/// \todo this generates the same file every time, breaking parallel testing
///       fix this with an output directory parameter passed into spawn?
fs::path create_fmu();

#endif // SPAWN_SPAWN_HPP
