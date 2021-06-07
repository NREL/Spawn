#ifndef CREATE_EPFMU_HPP
#define CREATE_EPFMU_HPP

#include "../util/filesystem.hpp"

fs::path single_family_house_idf_path();

fs::path two_zones_idf_path();

fs::path chicago_epw_path();

/// \todo this generates the same file every time, breaking parallel testing
///       fix this with an output directory parameter passed into spawn?
fs::path create_epfmu();

fs::path create_epfmu(const std::string & input_string);

fs::path create_single_family_house_fmu();

#endif // CREATE_EPFMU_HPP
