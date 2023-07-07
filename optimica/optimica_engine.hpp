#ifndef OPTIMICA_HPP_INCLUDED
#define OPTIMICA_HPP_INCLUDED

#include "fmu/fmu_type.hpp"
#include "modelica/modelica_engine.hpp"
#include "util/temp_directory.hpp"
#include <optimica/embedded_files.hxx>
#include <string>

namespace spawn::optimica {

class OptimicaEngine : modelica::ModelicaEngine
{
public:
  OptimicaEngine();

  OptimicaEngine(OptimicaEngine &&) = delete;

  OptimicaEngine(const OptimicaEngine &) = delete;

  OptimicaEngine &operator=(OptimicaEngine &&) = delete;

  OptimicaEngine &operator=(const OptimicaEngine &) = delete;

  ~OptimicaEngine() override = default;

  // Compile moInput to a Functional Mockup Unit
  [[nodiscard]] spawn_fs::path create_fmu(std::string_view input,
                                          const spawn_fs::path &output_dir,
                                          std::vector<spawn_fs::path> modelica_paths,
                                          const fmu::FMUType &fmu_type) override;

  // Not Implemented - Compile moInput to an executable
  [[nodiscard]] spawn_fs::path create_exe(std::string_view mo_input,
                                          const spawn_fs::path &output_dir,
                                          const std::vector<spawn_fs::path> &modelica_paths,
                                          const fmu::FMUType &fmu_type) override;

  // Act like a C compiler with Optimica headers/libraries in the path
  void make_external_function(const std::vector<std::string> &parameters);

private:
  // Extract the supporting Optimica embedded files into m_temp_dir
  void extract_embedded_files();

  // The Optimica compiler depends on the JMODELICA_HOME env variable
  void set_optimica_home();

  // Set the Modelica path(s) used during compiling
  void set_modelica_paths(const std::vector<spawn_fs::path> &paths);

  // Remove all Modelica paths, returning to the original state
  void reset_modelica_paths();

  // Convert a Modelica model name (my.foo.model) to a suitable file name (my_foo_model)
  [[nodiscard]] std::string modelica_model_to_filename(std::string_view model_name) const;

  // Current MBL path, if m_modelica_paths does not include MBL,
  // then look for Spawn bundled MBL and return that.
  [[nodiscard]] spawn_fs::path current_mbl_path() const;

  // The location of the supporting Optimica files within the temp directory
  [[nodiscard]] spawn_fs::path optimica_home_dir() const;

  // The location of the required gfortran library file within the temp directory
  [[nodiscard]] spawn_fs::path gfortran_path() const;

  // Glob for files on path and set the permissions to `perm`
  void chmod_files_in_path(const spawn_fs::path &path, const spawn_fs::perms perm) const;

  // Given a directory path, glob for C source files and return as a vector of paths
  [[nodiscard]] std::vector<spawn_fs::path> glob_source_files(const spawn_fs::path &sources_dir) const;

  // If MBL is in the Modelica path, there are a few C source files that need to be compiled
  [[nodiscard]] std::vector<spawn_fs::path> additional_source() const;

  // Rreturn a vector of paths to the required runtime libraries that must be linked
  [[nodiscard]] std::vector<spawn_fs::path> link_libraries() const;

  // Return a vector of paths to include during C compile
  [[nodiscard]] std::vector<spawn_fs::path> include_paths() const;

  // Use the Optimica compiler to generate C source code
  void generate_c(std::string_view input, const spawn_fs::path &output_dir, const fmu::FMUType &fmu_type) const;

  // Given a directory of generated C source code, compile to a shared library
  void generate_binary(const spawn_fs::path &sources_dir, const spawn_fs::path &output_lib_path) const;

  // Temp directory to extract supporting embedded files
  util::Temp_Directory m_temp_dir;

  std::vector<spawn_fs::path> m_modelica_paths;
};

} // namespace spawn::optimica

#endif // OPTIMICA_HPP_INCLUDED
