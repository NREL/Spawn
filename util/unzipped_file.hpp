#ifndef spawn_unzipped_file_hpp_INCLUDED
#define spawn_unzipped_file_hpp_INCLUDED

#include <boost/filesystem/path.hpp>

namespace spawn {
namespace util {

  class Unzipped_File
  {
  public:
    Unzipped_File(const boost::filesystem::path &zipFile,
                  const boost::filesystem::path &fileToUnzip,
                  boost::filesystem::path outputPath);

    const boost::filesystem::path &unzippedFile() const noexcept
    {
      return m_unzippedFile;
    }

  private:
    boost::filesystem::path m_unzippedFile;
  };

} // namespace util
} // namespace spawn

#endif
