#include "util/filesystem.hpp"
#include "util/fmi_paths.hpp"
#include <array>
#include <iostream>

#if __has_include(<unistd.h>)
#include <dlfcn.h>
#include <unistd.h>
#endif

#if __has_include(<windows.h>)
#include <windows.h>
#endif

#if __has_include(<linux/limits.h>)
#include <linux/limits.h>
#endif

#if __has_include(<limits.h>)
#include <climits>
#endif

namespace spawn::cli {

spawn_fs::path exe_path()
{
  auto get_path = []() {
#if _WIN32
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(nullptr, szPath, MAX_PATH);
    return spawn_fs::path(szPath);
#else
    // if /proc/self/exe exists, this should be our best option
    std::array<char, PATH_MAX + 1> buf{};
    if (const auto result = ::readlink("/proc/self/exe", buf.data(), buf.size()); result != -1) {
      return spawn_fs::path(buf.begin(), std::next(buf.begin(), result));
    }

    // otherwise we'll dlopen ourselves and see where "main" exists
    Dl_info info;
    dladdr("main", &info);
    return spawn_fs::path(info.dli_fname);
#endif
  };

  // canonical also makes the path absolute
  static const auto path = spawn_fs::canonical(get_path());
  return path;
}

spawn_fs::path exe_dir()
{
  return exe_path().parent_path();
}

bool is_installed()
{
  return spawn::exe_dir().stem() == "bin";
}

spawn_fs::path idd_path()
{
  constexpr auto &iddFileName = "Energy+.idd";
  // Configuration in install tree
  auto iddInputPath = spawn::exe_dir() / "../etc" / iddFileName;

  // Configuration in a developer tree
  if (!spawn_fs::exists(iddInputPath)) {
    iddInputPath = spawn::exe_dir() / iddFileName;
  }

  return iddInputPath;
}

spawn_fs::path epfmi_path()
{
  auto candidate = spawn::exe_dir() / ("../lib/" + spawn::epfmi_filename());
  if (spawn_fs::exists(candidate)) {
    return candidate;
  } else {
    return spawn::exe_dir() / spawn::epfmi_filename();
  }
}

} // namespace spawn::cli
