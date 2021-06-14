#ifndef COMPILERCHAIN_HPP_INCLUDED
#define COMPILERCHAIN_HPP_INCLUDED

#include <string>
#include <vector>
#include "../util/filesystem.hpp"

namespace spawn {

enum class ModelicaCompilerType {
  JModelica,
  Optimica
};

int modelicaToFMU(
  const std::string &moinput,
  const fs::path & mblPath,
  const fs::path & mslPath,
  const ModelicaCompilerType & moType = ModelicaCompilerType::JModelica
);

int compileMO(
  const std::string & moInput,
  const fs::path & outputDir,
  const fs::path & mblPath,
  const fs::path & modelicaHome,
  const fs::path & mslPath,
  const ModelicaCompilerType & moType
);

int compileC(
  const fs::path & output_dir,
  const fs::path & jmodelica_dir
);

void extractEmbeddedCompilerFiles(
  const fs::path & dir,
  const ModelicaCompilerType & moType
);

void makeModelicaExternalFunction(const std::vector<std::string> &parameters);

} // namespace spawn

#endif // COMPILERCHAIN_HPP_INCLUDED

