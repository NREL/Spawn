#include "optimica/optimica.hpp"
#include "c_compiler/compiler.hpp"
#include "fmu/modeldescription.hpp"
#include "mbl/config.hpp"
#include "nlohmann/json.hpp"
#include "optimica/config.hpp"
#include "optimica/optimica_compile.h"
#include "util/config.hpp"
#include "util/conversion.hpp"
#include "util/env_vars.hpp"
#include "util/ziputil.hpp"
#include <c_compiler/embedded_files.hxx>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>

namespace spawn {

Optimica::Optimica()
{
  spdlog::cfg::load_env_levels();
  extractEmbeddedFiles();
  setOptimicaHome();
}

spawn_fs::path Optimica::generateEXE(std::string_view /*moInput*/,
                                     const spawn_fs::path & /*outputDir*/,
                                     const std::vector<spawn_fs::path> & /*modelicaPaths*/,
                                     const FMUType & /*fmuType*/)
{
  throw std::runtime_error("Spawn does not support generating executables with Optimica.");
}

spawn_fs::path Optimica::generateFMU(std::string_view moInput,
                                     const spawn_fs::path &outputDir,
                                     std::vector<spawn_fs::path> modelicaPaths,
                                     const FMUType &fmuType)
{
  const auto output_model_name = modelicaModelToFileName(moInput);
  const auto model_staging_dir = outputDir / output_model_name;
  const auto sources_dir = model_staging_dir / "sources";
  const auto binary_dir = model_staging_dir / "binaries" / fmi_platform();
  const auto model_description_path = model_staging_dir / "modelDescription.xml";
  auto fmu_path = outputDir / (output_model_name + ".fmu");

  // Cleanup
  if (!model_staging_dir.empty()) {
    spawn_fs::remove_all(model_staging_dir);
  }
  if (spawn_fs::exists(fmu_path)) {
    spawn_fs::remove_all(fmu_path);
  }

  setModelicaPaths(modelicaPaths);

  // If MBL is not in the Modelica path,
  // then add the bundled version of MBL
  if (currentMBLPath().empty()) {
    modelicaPaths.push_back(mbl_home_dir());
    setModelicaPaths(modelicaPaths);
  }

  // Invoke Optimica to generate C code
  generateC(moInput, model_staging_dir, fmuType);

  // Get the generated model description file and use it to extract the model identifier
  spawn::fmu::ModelDescription md(model_description_path);
  // TODO: make this crossplatform
  const auto model_lib_path = binary_dir / (md.modelIdentifier() + ".so");

  // Invoke C compiler to generate a binary
  generateBinary(sources_dir, model_lib_path);

  // Bundle GFortran with the FMU, since it is a required dependency, but not provided out of the box on most systems
  spawn_fs::copy(gfortranPath(), binary_dir);

  // Update permissions to work in a variety of scenarios, including from within containers
  const auto perm = spawn_fs::perms::owner_all | spawn_fs::perms::group_read | spawn_fs::perms::group_exec |
                    spawn_fs::perms::others_read | spawn_fs::perms::others_exec;
  chmodFilesInPath(binary_dir, perm);

  // Zip FMU and cleanup
  zip_directory(model_staging_dir.string(), fmu_path.string(), false);
  spawn_fs::remove_all(model_staging_dir);

  spdlog::trace("Model Compiled");

  resetModelicaPaths();

  return fmu_path;
}

void Optimica::makeModelicaExternalFunction(const std::vector<std::string> &parameters)
{
  for (const auto &param : parameters) {
    spdlog::trace("makeModelicalExternalFunction parameter {}", param);
  }

  if (parameters.size() < 3) {
    spdlog::error("unable to determine build target, aborting");
    return;
  }

  if (parameters[3] == "ceval") {
    spdlog::info("ceval: compiling executable from .c files");
  } else {
    spdlog::error("Unknown build target: '{}' aborting", parameters[5]);
    return;
  }

  std::map<std::string, std::string> arguments;

  for (const auto &param : parameters) {
    const auto equals_pos = param.find('=');

    if (equals_pos != std::string::npos) {
      arguments[param.substr(0, equals_pos)] = param.substr(equals_pos + 1);
    }
  }

  for (const auto &[lhs, rhs] : arguments) {
    spdlog::trace("Parsed make modelica arg '{}'='{}'", lhs, rhs);
  }

  spawn_fs::path fileToCompile = spawn_fs::path{"sources"} / arguments["FILE_NAME"];
  fileToCompile += ".c";

  const auto jmodelica_dir = spawn_fs::path{arguments["JMODELICA_HOME"]};
  // const auto embedded_files_temp_dir = jmodelica_dir.parent_path();
  const auto include_paths = includePaths();
  const auto runtime_libs = linkLibraries();

  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, false);

  compiler.compile_and_link(fileToCompile);
  spawn_fs::create_directories(spawn_fs::path{"binaries"});

  // we'll name it .so regardless of platform because it's only for our use anyhow
  const auto launcherFileName = spawn_fs::path{"binaries"} / "spawn_exe_launcher";
  const auto exeFileName = spawn_fs::path{"binaries"} / arguments["FILE_NAME"];
  const auto soFileName = [&]() {
    auto result = exeFileName;
    result.replace_extension(exeFileName.extension().string() + ".so");
    return result;
  }();

  compiler.write_shared_object_file(soFileName, "", runtime_libs);
  spdlog::info("Modelical shared object output: {} exists {}", soFileName.string(), spawn_fs::exists(soFileName));

  // To support Windows this needs to be configured for extension
  spawn_c_compiler::embedded_files::extractFile(":/spawn_exe_launcher", "binaries");
  spawn_fs::rename(launcherFileName, exeFileName);

  spawn_fs::permissions(exeFileName, spawn_fs::perms::owner_exec);
}

void Optimica::setOptimicaHome()
{
  // Confusingly, Optimica internally still depends on the JMODELICA_HOME variable
  set_env("JMODELICA_HOME", optimicaHomeDir().string());
}

void Optimica::setModelicaPaths(const std::vector<spawn_fs::path> &paths)
{
  m_modelica_paths = paths;
  set_env("MODELICAPATH", pathVectorToPathString(m_modelica_paths));
}

void Optimica::resetModelicaPaths()
{
  setModelicaPaths(std::vector<spawn_fs::path>());
}

std::string Optimica::modelicaModelToFileName(std::string_view modelName) const
{
  std::string output_name;
  std::transform(begin(modelName), end(modelName), std::back_inserter(output_name), [](const auto &c) {
    if (c == '.') {
      return '_';
    } else {
      return c;
    }
  });
  return output_name;
}

spawn_fs::path Optimica::currentMBLPath() const
{
  for (const auto &p : m_modelica_paths) {
    auto path = spawn_fs::path(p);
    const auto package_path = path / "package.mo";
    const auto name = package_path.parent_path().stem().generic_string();
    if (spawn_fs::exists(package_path) && (name == "Buildings")) {
      return path;
    }
  }

  // Return an empty path if MBL is not found in path
  return {};
}

spawn_fs::path Optimica::optimicaHomeDir() const
{
  return m_temp_dir.dir() / "Optimica";
}

spawn_fs::path Optimica::gfortranPath() const
{
  return m_temp_dir.dir() / spawn::gfortranlib_embedded_path();
}

std::vector<spawn_fs::path> Optimica::globSourceFiles(const spawn_fs::path &sources_dir) const
{
  std::vector<spawn_fs::path> result;

  for (const auto &p : spawn_fs::directory_iterator(sources_dir)) {
    result.push_back(p.path());
  }

  // Exclude files that are not C source files
  result.erase(std::remove_if(begin(result), end(result), [](const auto &p) { return (p.extension() != ".c"); }),
               end(result));

  // Exclude ModelicaStrings_skipWhiteSpace.c
  // These symbols seem to be already defined within the runtime libraries
  // Excluding to avoid duplicate symbol error from compiler
  result.erase(
      std::remove_if(begin(result),
                     end(result),
                     [](const auto &p) { return (p.filename().string() == "ModelicaStrings_skipWhiteSpace.c"); }),
      end(result));

  result.erase(std::remove_if(begin(result),
                              end(result),
                              [](const auto &p) { return (p.filename().string() == "ModelicaStrings_compare.c"); }),
               end(result));

  return result;
}

void Optimica::chmodFilesInPath(const spawn_fs::path &path, const spawn_fs::perms perm) const
{
  for (const auto &entry : spawn_fs::directory_iterator{path}) {
    spawn_fs::permissions(entry, perm);
  }
}

std::vector<spawn_fs::path> Optimica::additionalSource() const
{
  std::vector<spawn_fs::path> result;

  // TODO: Need to get the mbl_path
  const auto mbl_path = currentMBLPath();
  for (const auto &entry : spawn_fs::directory_iterator(mbl_path / "Resources/src/ThermalZones" /
                                                        spawn::mbl_energyplus_version_string() / "C-Sources/")) {
    if (entry.path().extension() == ".c") {
      result.push_back(entry.path());
    }
  }

  return result;
}

std::vector<spawn_fs::path> Optimica::linkLibraries() const
{
  const auto t_msl_path = msl_path();
  const auto t_optimica_home = optimicaHomeDir();

  return {mbl_fmilib_path(),

          t_msl_path / "Modelica/Resources/Library/linux64/libzlib.a",
          t_msl_path / "Modelica/Resources/Library/linux64/libModelicaMatIO.a",
          t_msl_path / "Modelica/Resources/Library/linux64/libModelicaExternalC.a",
          t_msl_path / "Modelica/Resources/Library/linux64/libModelicaStandardTables.a",
          t_msl_path / "Modelica/Resources/Library/linux64/libModelicaIO.a",

          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_idas.a",
          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_nvecserial.a",
          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_arkode.a",
          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_ida.a",
          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_nvecopenmp.a",
          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_cvodes.a",
          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_cvode.a",
          t_optimica_home / "ThirdParty/Sundials/lib/libsundials_kinsol.a",
          t_optimica_home / "ThirdParty/Minpack/lib/libcminpack.a",

          t_optimica_home / "lib/RuntimeLibrary/libjmi.a",
          t_optimica_home / "lib/RuntimeLibrary/libblas.a",
          t_optimica_home / "lib/RuntimeLibrary/libjmi_evaluator_util.a",
          t_optimica_home / "lib/RuntimeLibrary/libfmi2.a",
          t_optimica_home / "lib/RuntimeLibrary/libfmi1_me.a",
          t_optimica_home / "lib/RuntimeLibrary/libfmi1_cs.a",
          t_optimica_home / "lib/RuntimeLibrary/libjmi_block_solver.a",
          t_optimica_home / "lib/RuntimeLibrary/libjmi_get_set_default.a",
          t_optimica_home / "lib/RuntimeLibrary/libjmi_get_set_lazy.a",
          t_optimica_home / "lib/RuntimeLibrary/liblapack.a",
          t_optimica_home / "lib/RuntimeLibrary/libModelicaUtilities.a",

          gfortranPath()};
}

std::vector<spawn_fs::path> Optimica::includePaths() const
{
  // TODO: Need to get the mbl_path
  const auto mbl_path = currentMBLPath();
  const auto t_optimica_home = optimicaHomeDir();

  std::vector<spawn_fs::path> result = {t_optimica_home / "include/RuntimeLibrary/",
                                        t_optimica_home / "include/RuntimeLibrary/LazyEvaluation",
                                        t_optimica_home / "include/RuntimeLibrary/module_include",
                                        // jmodelica_dir / "include/RuntimeLibrary/module_include",
                                        // jmodelica_dir / "include/RuntimeLibrary/zlib",
                                        t_optimica_home / "ThirdParty/FMI/2.0",
                                        // embedded_files_temp_dir / "usr/lib/llvm-10/lib/clang/10.0.0/include",
                                        // embedded_files_temp_dir / "usr/include",
                                        // embedded_files_temp_dir / "usr/include/linux",
                                        // embedded_files_temp_dir / "usr/include/x86_64-linux-gnu",
                                        spawn::msl_path() / "Modelica/Resources/C-Sources",
                                        // The mbl_paths are a hack because our compile rule is not aware of Modelica
                                        // annoations, such as the following....
                                        //    annotation (
                                        //      IncludeDirectory="modelica://Buildings/Resources/C-Sources",
                                        // Which is used by the Modelica external C objects
                                        // It would be good to generalize this so that it works for all Modelica
                                        // libraries that may use external objects
                                        mbl_path / "Resources/src/ThermalZones/" /
                                            spawn::mbl_energyplus_version_string() / "C-Sources/",
                                        mbl_path / "Resources/C-Sources",
                                        mbl_path / "Resources/src/fmi-library/include"};

  return result;
}

void Optimica::extractEmbeddedFiles()
{
  // TODO: This includes both jmodelica and optimica,
  // it would be better to select only the compiler files we need
  for (const auto &file : spawn_optimica::embedded_files::fileNames()) {
    spawn_optimica::embedded_files::extractFile(file, m_temp_dir.dir().string());
  }

  // The embedded filesystem does not preserve permission so this is an ugly but important step
  // To support Windows this needs to be configured for extension
  const spawn_fs::path licenseExecutable = m_temp_dir.dir() / "Optimica/lib/LicensingEncryption/linux/leif_mlle";
  spawn_fs::permissions(licenseExecutable, spawn_fs::perms::owner_exec | spawn_fs::perms::owner_read);

  // To support Windows this needs to be configured for extension
  const spawn_fs::path jmiEvaluatorExecutable = m_temp_dir.dir() / "Optimica/bin/jmi_evaluator";
  spawn_fs::permissions(jmiEvaluatorExecutable, spawn_fs::perms::owner_exec | spawn_fs::perms::owner_read);
}

void Optimica::generateC(std::string_view moInput, const spawn_fs::path &outputDir, const FMUType &fmuType) const
{
  try {
    // Only primitive data types can be passed from C++ to Java
    // so JSON is serialized and converted to a raw character array
    nlohmann::json j;
    j["model"] = moInput;
    j["outputDir"] = outputDir.generic_string();
    j["mslDir"] = msl_path().generic_string();
    j["fmuType"] = toString(fmuType);
    j["modelicaPaths"] = m_modelica_paths;

    std::string params = j.dump();
    spdlog::trace("Optimica::generateC called with params: {}", params);

    std::vector<char> cparams(params.begin(), params.end());
    cparams.resize(params.size() + 1, 0); // make sure we have a null terminator

    graal_isolate_t *isolate = nullptr;
    graal_isolatethread_t *thread = nullptr;

    optimica_create_isolate(nullptr, &isolate, &thread);
    optimica_compile(thread, cparams.data());
  } catch (...) {
    throw std::runtime_error("Optimica compiler encountered a fatal error.");
  }
}

void Optimica::generateBinary(const spawn_fs::path &sources_dir, const spawn_fs::path &output_lib_path) const
{
  if (!spawn_fs::is_directory(sources_dir)) {
    throw std::runtime_error("Attempt to compile Optimica generated C code from an unexpected path location.");
  }

  // Source files to compile
  auto source_files = globSourceFiles(sources_dir);

  const auto source_to_add = additionalSource();
  source_files.insert(source_files.end(), source_to_add.begin(), source_to_add.end());

  // Libs to link
  const auto link_libs = linkLibraries();

  // include paths
  auto include_paths = includePaths();

  const std::vector<std::string> flags{"-Wno-enum-conversion",
                                       "-Wno-incompatible-pointer-types-discards-qualifiers",
                                       "-Wno-logical-not-parentheses",
                                       "-fPIC"};

  spawn::Compiler compiler(include_paths, flags, false);

  std::for_each(begin(source_files), end(source_files), [&](const auto &path) { compiler.compile_and_link(path); });

  spawn_fs::create_directories(output_lib_path.parent_path());
  compiler.write_shared_object_file(output_lib_path, "", link_libs);
}

} // namespace spawn
