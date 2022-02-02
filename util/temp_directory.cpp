#include "temp_directory.hpp"

#include <chrono>
#include <fmt/format.h>
#include <fstream>
#include <spdlog/spdlog.h>

namespace spawn::util {

// creates an RAII managed temporary directory
Temp_Directory::Temp_Directory(const std::string &t_prefix)
{
  static constexpr auto max_tries = 1000;
  for (int count = 0; count < max_tries; ++count) {
    const auto p =
        spawn_fs::temp_directory_path() /
        fmt::format(
            "{}-{}-{}", t_prefix, std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()), count);
    if (spawn_fs::create_directories(p)) {
      m_dir = p;
      return;
    }
  }
  abort(); // couldn't create dir
}

Temp_Directory::~Temp_Directory()
{
  try {
    spawn_fs::remove_all(m_dir);
  } catch (const std::exception &e) {
    spdlog::error("Error removing temporary files: {}", e.what());
  }
}

} // namespace spawn::util
