#ifndef SPAWN_CONVERSION_HPP
#define SPAWN_CONVERSION_HPP

#include "util/filesystem.hpp"
#include <cassert>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

inline std::size_t as_size_t(const long long value)
{
  assert(value >= 0);
  return static_cast<std::size_t>(value);
}

inline std::string pathVectorToPathString(std::vector<spawn_fs::path> paths)
{
  std::stringstream ss;
  std::ostream_iterator<std::string> it(ss, ":");
  std::copy(paths.begin(), paths.end(), it);
  return ss.str();
}

#endif // SPAWN_CONVERSION_HPP
