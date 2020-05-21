#ifndef spawn_unzipped_file_hpp_INCLUDED
#define spawn_unzipped_file_hpp_INCLUDED

#include <boost/filesystem/path.hpp>

namespace spawn {
namespace util {

  class Unzipped_File
  {
  public:
    Unzipped_File(const boost::filesystem::path &zipFile,
                  boost::filesystem::path outputDir,
                  const std::vector<boost::filesystem::path> &filesToUnzip
);

    const boost::filesystem::path &ouputDir() const noexcept
    {
      return m_outputDir;
    }

    const std::vector<boost::filesystem::path> &unzippedFiles() const noexcept {
      return m_unzippedFiles;
    }

  private:
    boost::filesystem::path m_outputDir;
    std::vector<boost::filesystem::path> m_unzippedFiles;
  };

} // namespace util
} // namespace spawn

#endif
