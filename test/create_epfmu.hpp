#ifndef CREATE_EPFMU_HPP
#define CREATE_EPFMU_HPP

#include "../util/filesystem.hpp"

/// \todo this generates the same file every time, breaking parallel testing
///       fix this with an output directory parameter passed into spawn?
fs::path create_epfmu();

fs::path create_single_family_house_fmu();

#endif // CREATE_EPFMU_HPP
