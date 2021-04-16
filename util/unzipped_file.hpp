#ifndef spawn_unzipped_file_hpp_INCLUDED
#define spawn_unzipped_file_hpp_INCLUDED

#include "./filesystem.hpp"
#include <vector>

namespace spawn {
namespace util {

  class Unzipped_File
  {
  public:
    Unzipped_File(const fs::path &zipFile,
                  fs::path outputDir,
                  const std::vector<fs::path> &filesToUnzip);

    const fs::path &outputDir() const noexcept
    {
      return m_outputDir;
    }

    const std::vector<fs::path> &unzippedFiles() const noexcept {
      return m_unzippedFiles;
    }

  private:
    fs::path m_outputDir;
    std::vector<fs::path> m_unzippedFiles;
  };

} // namespace util
} // namespace spawn

#endif
