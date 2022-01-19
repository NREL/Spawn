#include "ziputil.hpp"
#include "zip.h"
#include <cerrno>
#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <fmt/format.h>

#if _MSC_VER
#include "./msvc/dirent.h"
#else
#include <dirent.h>
#endif

bool is_dir(const std::string &dir)
{
  struct stat st{};
  ::stat(dir.c_str(), &st);
  return S_ISDIR(st.st_mode); // NOLINT
}

void walk_directory(const std::string &startdir, const std::string &inputdir, zip_t *zipper, bool no_compression)
{
  DIR *dp = ::opendir(inputdir.c_str());
  if (dp == nullptr) {
    throw std::runtime_error("Failed to open input directory: " + std::string(::strerror(errno)));
  }

  struct dirent *dirp = nullptr;

  const auto name = [](const dirent *directory) {
    return std::string_view{static_cast<const char *>(directory->d_name)};
  };

  while ((dirp = readdir(dp)) != nullptr) { // NOLINT we want the POSIX version here, sorry!
    if (name(dirp) != std::string(".") && name(dirp) != std::string("..")) {
      std::string fullname = fmt::format("{}/{}", inputdir, name(dirp));
      if (is_dir(fullname)) {
        if (zip_dir_add(zipper, fullname.substr(startdir.length() + 1).c_str(), ZIP_FL_ENC_UTF_8) < 0) {
          throw std::runtime_error("Failed to add directory to zip: " + std::string(zip_strerror(zipper)));
        }
        walk_directory(startdir, fullname, zipper, no_compression);
      } else {
        zip_source_t *source = zip_source_file(zipper, fullname.c_str(), 0, 0);
        if (source == nullptr) {
          throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
        }
        const auto index =
            zip_file_add(zipper, fullname.substr(startdir.length() + 1).c_str(), source, ZIP_FL_ENC_UTF_8);
        if (index < 0) {
          zip_source_free(source);
          throw std::runtime_error("Failed to add file to zip: " + std::string(zip_strerror(zipper)));
        } else if (no_compression) {
          zip_set_file_compression(zipper, static_cast<zip_uint64_t >(index), ZIP_CM_STORE, 0);
        }
      }
    }
  }
  ::closedir(dp);
}

void zip_directory(const std::string &inputdir, const std::string &output_filename, bool no_compression)
{
  int errorp{};
  zip_t *zipper = zip_open(output_filename.c_str(), ZIP_CREATE | ZIP_EXCL, &errorp); // NOLINT need bitwise or for lib
  if (zipper == nullptr) {
    zip_error_t ziperror;
    zip_error_init_with_code(&ziperror, errorp);
    throw std::runtime_error("Failed to open output file " + output_filename + ": " + zip_error_strerror(&ziperror));
  }

  try {
    walk_directory(inputdir, inputdir, zipper, no_compression);
  } catch (...) {
    zip_close(zipper);
    throw;
  }

  zip_close(zipper);
}
