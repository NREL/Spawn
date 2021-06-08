#ifndef COMPILERCHAIN_HPP_INCLUDED
#define COMPILERCHAIN_HPP_INCLUDED

#include <string>
#include "../util/filesystem.hpp"

namespace spawn {

int compileMO(
  const std::string & moInput,
  const fs::path & outputDir,
  const fs::path & mblPath,
  const fs::path & jmodelica_dir,
  const fs::path & mslPath
);

int compileC(
  const fs::path & output_dir,
  const fs::path & jmodelica_dir
);

int modelicaToFMU(
  const std::string &moinput,
  const fs::path & mblPath,
  const fs::path & mslPath
);

void makeModelicaExternalFunction(const std::vector<std::string> &parameters);

} // namespace spawn

#endif // COMPILERCHAIN_HPP_INCLUDED

