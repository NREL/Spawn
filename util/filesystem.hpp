#ifndef spawn_util_filesystem_hh_INCLUDED
#define spawn_util_filesystem_hh_INCLUDED

#if HAVE_FILESYSTEM_H
#include <filesystem>
namespace fs = std::filesystem;
#elif HAVE_EXP_FILESYSTEM_H
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
namespace std::experimental::filesystem {
// https://stackoverflow.com/questions/63899489/c-experimental-filesystem-has-no-relative-function
[[nodiscard]] inline path relative(path p, path base)
{
  // 1. convert p and base to absolute paths
  p = fs::absolute(p);
  base = fs::absolute(base);

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

#endif // spawn_util_filesystem_hh_INCLUDED
