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
  for (const auto &path : paths) {
    ss << path.string();
    ss << ":";
  }
  auto result = ss.str();
  result.pop_back();
  return result;
}

#endif // SPAWN_CONVERSION_HPP
