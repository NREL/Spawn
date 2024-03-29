#include "unzipped_file.hpp"
#include "filesystem.hpp"
#include <array>
#include <fmt/format.h>
#include <fstream>
#include <stdexcept>
#include <type_traits>
#include <zip.h>

namespace spawn::util {

std::unique_ptr<zip_t, decltype(&zip_discard)> open_zip(const spawn_fs::path &zipFile)
{
  int err{};
  auto zip = zip_open(zipFile.string().c_str(), ZIP_CHECKCONS | ZIP_RDONLY, &err); // NOLINT bitwise or with signed

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

std::unique_ptr<zip_file_t, decltype(&zip_fclose)> open_file(zip_t &zipFile, const spawn_fs::path &path)
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

std::vector<spawn_fs::path> zipped_files(zip_t &zipFile)
{
  std::vector<spawn_fs::path> results;

  const auto file_count = zip_get_num_entries(&zipFile, 0);

  for (auto i = static_cast<std::remove_const<decltype(file_count)>::type>(0); i < file_count; ++i) {
    const auto *name = zip_get_name(&zipFile, static_cast<zip_uint64_t>(i), ZIP_FL_ENC_GUESS);
    if (name != nullptr) {
      results.emplace_back(name);
    }
  }

  return results;
}

Unzipped_File::Unzipped_File(const spawn_fs::path &zipFile,
                             spawn_fs::path outputDir,
                             const std::vector<spawn_fs::path> &filesToUnzip)
    : m_outputDir{std::move(outputDir)}
{
  auto zip = open_zip(zipFile);

  const auto &files = [&]() {
    if (!filesToUnzip.empty()) {
      return filesToUnzip;
    } else {
      return zipped_files(*zip);
    }
  }();

  for (const auto &fileToUnzip : files) {
    auto file = open_file(*zip, fileToUnzip);
    constexpr auto buffer_size = 4096;
    std::array<char, buffer_size> buffer{};
    const auto read_bytes = [&]() { return zip_fread(file.get(), buffer.data(), buffer_size); };

    const auto unzippedFile = m_outputDir / fileToUnzip;
    spawn_fs::create_directories(unzippedFile.parent_path());
    std::ofstream ofs(unzippedFile.string(), std::ofstream::trunc | std::ofstream::binary);
    for (zip_int64_t bytesread = read_bytes(); bytesread > 0; bytesread = read_bytes()) {
      ofs.write(buffer.data(), bytesread);
    }

    m_unzippedFiles.push_back(unzippedFile);
  }
}
} // namespace spawn::util
