#ifndef FMUGENERATOR_HH_INCLUDED
#define FMUGENERATOR_HH_INCLUDED

#include "../util/filesystem.hpp"
#include <string>

namespace spawn {

void energyplusToFMU(const std::string &jsoninput,
                     bool nozip,
                     bool nocompress,
                     const std::string &outputpath,
                     const std::string &outputdir,
                     const spawn_fs::path &iddpath,
                     const spawn_fs::path &epfmupath);

} // namespace spawn

#endif // FMUGENERATOR_HH_INCLUDED
