#ifndef FMUGENERATOR_HH_INCLUDED
#define FMUGENERATOR_HH_INCLUDED

#include <string>
#include <boost/filesystem.hpp>

namespace spawn {

int generatefmu(
  const std::string &jsoninput,
  bool nozip,
  bool nocompress,
  const std::string & outputpath,
  boost::filesystem::path iddpath,
  boost::filesystem::path epfmupath
);

} // namespace spawn

#endif // FMUGENERATOR_HH_INCLUDED

