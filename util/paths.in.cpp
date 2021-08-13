#include "paths.hpp"
#include <iostream>

#if __has_include(<unistd.h>)
#include <unistd.h>
#include <dlfcn.h>
#endif

#if __has_include(<windows.h>)
#include <windows.h>
#endif

#if __has_include(<linux/limits.h>)
#include <linux/limits.h>
#endif

#if __has_include(<limits.h>)
#include <limits.h>
#endif

namespace spawn {
  fs::path exe() {
    auto get_path = [](){
      #if _WIN32
        TCHAR szPath[MAX_PATH];
        GetModuleFileName(nullptr, szPath, MAX_PATH);
        return fs::path(szPath);
      #else
        // if /proc/self/exe exists, this should be our best option
        char buf[PATH_MAX + 1];
        if (const auto result = readlink("/proc/self/exe", buf, sizeof(buf) - 1); result != -1) {
          return fs::path(std::begin(buf), std::next(buf, result));
        }

        // otherwise we'll dlopen ourselves and see where "main" exists
        Dl_info info;
        dladdr("main", &info);
        return fs::path(info.dli_fname);
      #endif
    };

    // canonical also makes the path absolute
    static const auto path = fs::canonical(get_path());
    return path;
  }
  
  fs::path exedir() {
    static const auto path = exe().parent_path();
    return path;
  }

  fs::path mbl_home_dir() {
    const auto exedirname = (--exedir().end())->string();

    if (exedirname == "bin") {
      return exedir() / "../etc/modelica-buildings";
    } else {
      return project_source_dir() / "submodules/modelica-buildings";
    }
  }

  fs::path project_source_dir() {
    return "${PROJECT_SOURCE_DIR}";
  }
  
  fs::path project_binary_dir() {
    return "${PROJECT_BINARY_DIR}";
  }
}
