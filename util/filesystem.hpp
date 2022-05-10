#ifndef spawn_util_filesystem_hh_INCLUDED
#define spawn_util_filesystem_hh_INCLUDED

#if HAVE_FILESYSTEM_H
#include <filesystem>
namespace spawn_fs = std::filesystem;
#elif HAVE_EXP_FILESYSTEM_H
#include <experimental/filesystem>
namespace spawn_fs = std::experimental::filesystem;
namespace std::experimental::filesystem {
// https://stackoverflow.com/questions/63899489/c-experimental-filesystem-has-no-relative-function
[[nodiscard]] inline path relative(path p, path base)
{
  // 1. convert p and base to absolute paths
  p = spawn_fs::absolute(p);
  base = spawn_fs::absolute(base);

  // 2. find first mismatch and shared root path
  auto mismatched = std::mismatch(p.begin(), p.end(), base.begin(), base.end());

  // 3. if no mismatch return "."
  if (mismatched.first == p.end() && mismatched.second == base.end()) return ".";

  auto it_p = mismatched.first;
  auto it_base = mismatched.second;

  path ret;

  // 4. iterate abase to the shared root and append "../"
  for (; it_base != base.end(); ++it_base)
    ret /= "..";

  // 5. iterate from the shared root to the p and append its parts
  for (; it_p != p.end(); ++it_p)
    ret /= *it_p;

  return ret;
}
} // namespace std::experimental::filesystem
#else
#error "no filesystem support"
#endif

namespace spawn {

[[nodiscard]] inline spawn_fs::path find_recursive(const spawn_fs::path &p, const spawn_fs::path &base)
{
  for (auto const &dir_entry : spawn_fs::recursive_directory_iterator(base)) {
    if (p.filename() == dir_entry.path().filename()) {
      return dir_entry.path();
    }
  }

  return spawn_fs::path();
}

} // namespace spawn

#endif // spawn_util_filesystem_hh_INCLUDED
