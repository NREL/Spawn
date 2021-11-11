#include "paths.hpp"
#include "fmi_paths.hpp"
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
  
  fs::path exe_dir() {
    static const auto path = exe().parent_path();
    return path;
  }

  fs::path mbl_home_dir() {
    if (is_installed()) {
      return exe_dir() / "../etc/modelica-buildings/Buildings";
    } else {
      return project_source_dir() / "submodules/modelica-buildings/Buildings";
    }
  }

  bool is_installed() {
    return spawn::exe_dir().stem() == "bin";
  }
  
  fs::path idd_install_path() {
    constexpr auto & iddFileName = "Energy+.idd";
    // Configuration in install tree
    auto iddInputPath = spawn::exe_dir() / "../etc" / iddFileName;
  
    // Configuration in a developer tree
    if (! fs::exists(iddInputPath)) {
      iddInputPath = spawn::exe_dir() / iddFileName;
    }
  
    return iddInputPath;
  }
  
  fs::path epfmi_install_path() {
    const auto candidate = spawn::exe_dir() / ("../lib/" + spawn::epfmi_filename());
    if (fs::exists(candidate)) {
      return candidate;
    } else {
      return spawn::exe_dir() / spawn::epfmi_filename();
    }
  }
  
  fs::path msl_path() {
    fs::path p;
    if (is_installed()) {
      p = spawn::exe_dir() / "../etc/MSL/";
    } else {
      p = spawn::project_binary_dir() / "JModelica/ThirdParty/MSL/";
    }
  
    return p;
  }

  fs::path project_source_dir() {
    return "${PROJECT_SOURCE_DIR}";
  }
  
  fs::path project_binary_dir() {
    return "${PROJECT_BINARY_DIR}";
  }

  fs::path idd_path() {
    constexpr auto & iddfilename = "Energy+.idd";
    auto iddInputPath = exe_dir() / "../../resources" / iddfilename;
  
    if (! fs::exists(iddInputPath)) {
      iddInputPath = exe_dir() / iddfilename;
    }
  
    return iddInputPath;
  }
}
