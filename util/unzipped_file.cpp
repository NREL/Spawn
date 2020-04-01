#include "unzipped_file.hpp"
#include <fmt/format.h>
#include <fstream>
#include <stdexcept>
#include <zip.h>


namespace spawn {
namespace util {

  std::unique_ptr<zip_t, decltype(&zip_discard)> open_zip(const boost::filesystem::path &zipFile)
  {
    int err{};
    auto zip = zip_open(zipFile.c_str(), ZIP_CHECKCONS | ZIP_RDONLY, &err);

    if (zip == nullptr) {
      zip_error_t errt{};
      zip_error_init_with_code(&errt, err);
      std::string error_string = zip_error_strerror(&errt);
      zip_error_fini(&errt);
      throw std::runtime_error(
          fmt::format("Error opening zipfile: '{}', error description: '{}'", zipFile.string(), error_string));
    }
    return {zip, zip_discard};
  }

  std::unique_ptr<zip_file_t, decltype(&zip_fclose)> open_file(zip_t &zipFile, const boost::filesystem::path &path)
  {
    auto *f = zip_fopen(&zipFile, path.string().c_str(), 0);

    if (f == nullptr) {
      auto *err = zip_get_error(&zipFile);
      std::string error_string = zip_error_strerror(err);
      zip_error_fini(err);
      throw std::runtime_error(
          fmt::format("Error opening file in zip: '{}', error description: '{}'", path.string(), error_string));
    }

    return {f, zip_fclose};
  }

  Unzipped_File::Unzipped_File(const boost::filesystem::path &zipFile,
                               const boost::filesystem::path &fileToUnzip,
                               boost::filesystem::path outputPath)
      : m_unzippedFile{std::move(outputPath)}
  {
    auto zip = open_zip(zipFile);
    auto file = open_file(*zip, fileToUnzip);

    constexpr auto buffer_size = 4096;
    char buffer[buffer_size];
    const auto read_bytes = [&]() { return zip_fread(file.get(), buffer, buffer_size); };
    std::ofstream ofs(m_unzippedFile.string(), std::ofstream::trunc);
    for (zip_int64_t bytesread = read_bytes(); bytesread > 0; bytesread = read_bytes()) {
      ofs.write(buffer, bytesread);
    }
  }

} // namespace util
} // namespace spawn