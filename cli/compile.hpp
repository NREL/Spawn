#ifndef COMPILERCHAIN_HPP_INCLUDED
#define COMPILERCHAIN_HPP_INCLUDED

#include "../util/filesystem.hpp"
#include <string>
#include <vector>

namespace spawn {

enum class ModelicaCompilerType
{
  JModelica,
  Optimica
};

int modelicaToFMU(const std::string &moinput,
                  std::vector<std::string> modelicaPaths,
                  const ModelicaCompilerType &moType = ModelicaCompilerType::JModelica);

int compileMO(const std::string &moInput,
              const spawn_fs::path &outputDir,
              const std::vector<std::string> &modelicaPaths,
              const ModelicaCompilerType &moType);

int compileC(const spawn_fs::path &output_dir, const spawn_fs::path &jmodelica_dir, const spawn_fs::path &embedded_files_temp_dir);

void extractEmbeddedCompilerFiles(const spawn_fs::path &dir, const ModelicaCompilerType &moType);

void makeModelicaExternalFunction(const std::vector<std::string> &parameters);

} // namespace spawn

#endif // COMPILERCHAIN_HPP_INCLUDED
