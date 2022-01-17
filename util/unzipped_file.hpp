#ifndef spawn_unzipped_file_hpp_INCLUDED
#define spawn_unzipped_file_hpp_INCLUDED

#include "./filesystem.hpp"
#include <vector>

namespace spawn {
namespace util {

  class Unzipped_File
  {
  public:
    Unzipped_File(const spawn_fs::path &zipFile,
                  spawn_fs::path outputDir, const std::vector<spawn_fs::path> &filesToUnzip);

    [[nodiscard]] const spawn_fs::path &outputDir() const noexcept
    {
      return m_outputDir;
    }

    [[nodiscard]] const std::vector<spawn_fs::path> &unzippedFiles() const noexcept
    {
      return m_unzippedFiles;
    }

  private:
    spawn_fs::path m_outputDir;
    std::vector<spawn_fs::path> m_unzippedFiles;
  };

} // namespace util
} // namespace spawn

#endif
