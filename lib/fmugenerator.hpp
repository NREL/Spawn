#ifndef FMUGENERATOR_HH_INCLUDED
#define FMUGENERATOR_HH_INCLUDED

#include <string>
#include "../util/filesystem.hpp"

namespace spawn {

void energyplusToFMU(
  const std::string &jsoninput,
  bool nozip,
  bool nocompress,
  const std::string & outputpath,
  const std::string & outputdir,
  fs::path iddpath,
  fs::path epfmupath
);

} // namespace spawn

#endif // FMUGENERATOR_HH_INCLUDED

