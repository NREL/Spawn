#ifndef SPAWN_SPAWN_HPP
#define SPAWN_SPAWN_HPP

/// \todo this generates the same file every time, breaking parallel testing
///       fix this with an output directory parameter passed into spawn?
std::filesystem::path create_fmu();

#endif // SPAWN_SPAWN_HPP
