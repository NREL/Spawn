#ifndef SPAWN_FMI_PATHS_HPP
#define SPAWN_FMI_PATHS_HPP

#include <string>
#include <fmt/format.h>

namespace spawn {
static std::string fmi_platform()
{
#ifdef __APPLE__
  return "darwin64";
#elif _WIN32
  return "win64";
#else
  return "linux64";
#endif
}

static std::string fmi_lib_ext()
{
#ifdef __APPLE__
  return "dylib";
#elif _WIN32
  return "dll";
#else
  return "so";
#endif
}

static std::string fmi_lib_prefix()
{
#ifdef __APPLE__
  return "lib";
#elif _WIN32
  return "";
#else
  return "lib";
#endif
}

static std::string fmi_lib_filename(const std::string &library_name)
{
  return fmt::format("{}{}.{}", fmi_lib_prefix(), library_name, fmi_lib_ext());
}

static boost::filesystem::path fmi_lib_path(const std::string &library_name)
{
  return boost::filesystem::path{"binaries"} / fmi_platform() / fmi_lib_filename(library_name);
}

}

#endif // SPAWN_FMI_PATHS_HPP
