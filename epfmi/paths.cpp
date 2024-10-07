#include "util/filesystem.hpp"
#include "util/fmi_paths.hpp"
#if defined _WIN32
#include <windows.h>
#else
#include <cstdio>
#include <dlfcn.h>
#endif

namespace spawn::epfmi {

// define the function module so we can get its address
#if defined _WIN32
HMODULE module()
{
  HMODULE hModule = NULL;
  // hModule is NULL if GetModuleHandleEx fails.
  GetModuleHandleEx(
      GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCTSTR)module, &hModule);
  return hModule;
}
#else
bool module()
{
  return true;
}
#endif

spawn_fs::path module_path()
{
  spawn_fs::path path;
#if defined _WIN32
  TCHAR szPath[MAX_PATH];
  if (GetModuleFileName(module(), szPath, MAX_PATH)) {
    path = toPath(szPath);
  }
#else
  Dl_info info;
  if (dladdr("module", &info) != 0) {
    path = info.dli_fname;
  }
#endif

  return spawn_fs::canonical(path);
}

spawn_fs::path idd_path()
{
  return module_path().parent_path() / "../../resources" / "Energy+.idd";
}

} // namespace spawn::epfmi
