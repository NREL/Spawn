#ifndef SPAWN_FMI_PATHS_HPP
#define SPAWN_FMI_PATHS_HPP

#include <string>
#include <fmt/format.h>

namespace spawn {
[[nodiscard]] constexpr static std::string_view fmi_platform() noexcept
{
#ifdef __APPLE__
  return "darwin64";
#elif _WIN32
  return "win64";
#else
  return "linux64";
#endif
}

[[nodiscard]] constexpr static std::string_view fmi_lib_ext() noexcept
{
#ifdef __APPLE__
  return "dylib";
#elif _WIN32
  return "dll";
#else
  return "so";
#endif
}

[[nodiscard]] constexpr static std::string_view fmi_lib_prefix() noexcept
{
#ifdef __APPLE__
  return "";
#elif _WIN32
  return "";
#else
  return "";
#endif
}

[[nodiscard]] static std::string fmi_lib_filename(const std::string_view library_name)
{
  return fmt::format("{}{}.{}", fmi_lib_prefix(), library_name, fmi_lib_ext());
}

[[nodiscard]] static fs::path fmi_lib_path(const std::string_view library_name)
{
  return fs::path{"binaries"} / fmi_platform() / fmi_lib_filename(library_name);
}

[[nodiscard]] constexpr static std::string_view epfmi_basename() noexcept
{
  return "epfmi";
}

static inline std::string epfmi_filename()
{
  return fmi_lib_filename(epfmi_basename());
}

}

#endif // SPAWN_FMI_PATHS_HPP
