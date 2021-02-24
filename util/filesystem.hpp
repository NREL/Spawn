#ifndef spawn_util_filesystem_hh_INCLUDED
#define spawn_util_filesystem_hh_INCLUDED

#if HAVE_FILESYSTEM_H 
  #include <filesystem>
  namespace fs = std::filesystem;
#elif HAVE_EXP_FILESYSTEM_H
  #include <experimental/filesystem>
  namespace fs = std::experimental::filesystem;
#else
  #error "no filesystem support"
#endif

#endif // spawn_util_filesystem_hh_INCLUDED
