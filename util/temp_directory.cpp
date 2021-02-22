#include "temp_directory.hpp"

#include <chrono>
#include <fstream>

namespace spawn {
namespace util {

  // creates an RAII managed temporary directory
  Temp_Directory::Temp_Directory(const std::string &t_prefix)
  {

    for (int count = 0; count < 1000; ++count) {
      const auto p = std::filesystem::temp_directory_path() /
                     (std::string{t_prefix} + std::string{'-'} +
                      std::to_string(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) + '-' +
                      std::to_string(count));
      if (std::filesystem::create_directories(p)) {
        m_dir = p;
        return;
      }
    }
    abort(); // couldn't create dir
  }

  Temp_Directory::~Temp_Directory()
  {
    std::filesystem::remove_all(m_dir);
  }

} // namespace util
} // namespace spawn
