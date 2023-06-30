#ifndef OPTIMICA_HPP_INCLUDED
#define OPTIMICA_HPP_INCLUDED

#include "fmu/fmu_type.hpp"
#include "util/temp_directory.hpp"
#include <optimica/embedded_files.hxx>
#include <string>

namespace spawn {

class Optimica
{
public:
  Optimica();

  Optimica(Optimica &&) = delete;

  Optimica(const Optimica &) = delete;

  Optimica &operator=(Optimica &&) = delete;

  Optimica &operator=(const Optimica &) = delete;

  ~Optimica() = default;

  // Compile moInput to a Functional Mockup Unit
  [[nodiscard]] spawn_fs::path generateFMU(std::string_view moInput,
                                           const spawn_fs::path &outputDir,
                                           std::vector<spawn_fs::path> modelicaPaths,
                                           const FMUType &fmuType);

  // Not Implemented - Compile moInput to an executable
  [[nodiscard]] spawn_fs::path generateEXE(std::string_view moInput,
                                           const spawn_fs::path &outputDir,
                                           const std::vector<spawn_fs::path> &modelicaPaths,
                                           const FMUType &fmuType);

  // Act like a c compiler with Optimica headers/libraries in the path
  void makeModelicaExternalFunction(const std::vector<std::string> &parameters);

private:
  // Extract the supporting Optimica embedded files into m_temp_dir
  void extractEmbeddedFiles();

  // The Optimica compiler depends on the JMODELICA_HOME env variable
  void setOptimicaHome();

  // Set the Modelica path(s) used during compiling
  void setModelicaPaths(const std::vector<spawn_fs::path> &paths);

  // Remove all Modelica paths, returning to the original state
  void resetModelicaPaths();

  // Convert a Modelica model name (my.foo.model) to a suitable file name (my_foo_model)
  [[nodiscard]] std::string modelicaModelToFileName(std::string_view modelName) const;

  // Current MBL path, if m_modelica_paths does not include MBL,
  // then look for Spawn bundled MBL and return that.
  [[nodiscard]] spawn_fs::path currentMBLPath() const;

  // The location of the supporting Optimica files within the temp directory
  [[nodiscard]] spawn_fs::path optimicaHomeDir() const;

  // The location of the required gfortran library file within the temp directory
  [[nodiscard]] spawn_fs::path gfortranPath() const;

  // Glob for files on path and set the permissions to `perm`
  void chmodFilesInPath(const spawn_fs::path &path, const spawn_fs::perms perm) const;

  // Given a directory path, glob for C source files and return as a vector of paths
  [[nodiscard]] std::vector<spawn_fs::path> globSourceFiles(const spawn_fs::path &sources_dir) const;

  // If MBL is in the Modelica path, there are a few C source files that need to be compiled
  [[nodiscard]] std::vector<spawn_fs::path> additionalSource() const;

  // Rreturn a vector of paths to the required runtime libraries that must be linked
  [[nodiscard]] std::vector<spawn_fs::path> linkLibraries() const;

  // Return a vector of paths to include during C compile
  [[nodiscard]] std::vector<spawn_fs::path> includePaths() const;

  // Use the Optimica compiler to generate C source code
  void generateC(std::string_view moInput, const spawn_fs::path &outputDir, const FMUType &fmuType) const;

  // Given a directory of generated C source code, compile to a shared library
  void generateBinary(const spawn_fs::path &sources_dir, const spawn_fs::path &output_lib_path) const;

  // Temp directory to extract supporting embedded files
  util::Temp_Directory m_temp_dir;

  std::vector<spawn_fs::path> m_modelica_paths;
};

} // namespace spawn

#endif // OPTIMICA_HPP_INCLUDED
