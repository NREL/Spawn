#ifndef spawn_unzipped_file_hpp_INCLUDED
#define spawn_unzipped_file_hpp_INCLUDED

#include <vector>
#include <filesystem>

namespace spawn {
namespace util {

  class Unzipped_File
  {
  public:
    Unzipped_File(const std::filesystem::path &zipFile,
                  std::filesystem::path outputDir,
                  const std::vector<std::filesystem::path> &filesToUnzip);

    const std::filesystem::path &outputDir() const noexcept
    {
      return m_outputDir;
    }

    const std::vector<std::filesystem::path> &unzippedFiles() const noexcept {
      return m_unzippedFiles;
    }

  private:
    std::filesystem::path m_outputDir;
    std::vector<std::filesystem::path> m_unzippedFiles;
  };

} // namespace util
} // namespace spawn

#endif
