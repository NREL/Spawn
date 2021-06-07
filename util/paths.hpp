#ifndef spawn_util_paths_hh_INCLUDED
#define spawn_util_paths_hh_INCLUDED

#include "filesystem.hpp"

namespace spawn {
  // todo Move idd paths, etc into here
  fs::path exe();

  fs::path exedir();
  
  fs::path mbl_home_dir();

  fs::path project_source_dir();
  
  fs::path project_binary_dir();
}



#endif


