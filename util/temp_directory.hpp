#ifndef NREL_SPAWN_UTILITY_HPP
#define NREL_SPAWN_UTILITY_HPP

#include "./filesystem.hpp"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace spawn::util {
// creates an RAII managed temporary directory
// TODO merge this with compiler/utility after compiler branch is merged
struct Temp_Directory
{
  explicit Temp_Directory(const std::string &t_prefix = "spawn");
  ~Temp_Directory();

  [[nodiscard]] const spawn_fs::path &dir() const noexcept
  {
    return m_dir;
  }

  Temp_Directory(Temp_Directory &&) = delete;
  Temp_Directory(const Temp_Directory &) = delete;
  Temp_Directory &operator=(const Temp_Directory &) = delete;
  Temp_Directory &operator=(Temp_Directory &&) = delete;

private:
  spawn_fs::path m_dir;
};
} // namespace spawn::util

#endif
