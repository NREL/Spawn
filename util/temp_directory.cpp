#include "temp_directory.hpp"

#include <chrono>
#include <fstream>
#include <fmt/format.h>

namespace spawn::util {

  // creates an RAII managed temporary directory
  Temp_Directory::Temp_Directory(const std::string &t_prefix)
  {

    for (int count = 0; count < 1000; ++count) {
      const auto p = fs::temp_directory_path() /
              fmt::format("{}-{}-{}",
                          t_prefix,
                          std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()),
                          count);
      if (fs::create_directories(p)) {
        m_dir = p;
        return;
      }
    }
    abort(); // couldn't create dir
  }

  Temp_Directory::~Temp_Directory()
  {
    fs::remove_all(m_dir);
  }

} // namespace spawn
