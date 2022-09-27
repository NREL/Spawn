#ifndef NREL_SPAWN_COMPILER_HPP
#define NREL_SPAWN_COMPILER_HPP

#include "../util/filesystem.hpp"
#include "../util/temp_directory.hpp"

#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

namespace spawn {

class Compiler
{
public:
  // If use_c_bridge_instead_of_stdlib is `true`, then all standard system
  // paths are ignored and only the embedded c_bridge library is available
  explicit Compiler(std::vector<spawn_fs::path> include_paths,
                    std::vector<std::string> flags,
                    bool use_c_bridge_instead_of_stdlib);

  // Adds the c_bridge library (dll/so) to the current PATH/LD_LIBRARY_PATH
  // to make it accessible to dynamically loaded libraries.
  void add_c_bridge_to_path();

  // adds a string source to the current compilation unit
  void compile_and_link(const std::string_view source);

  // adds a file source to the current compilation unit
  void compile_and_link(const spawn_fs::path &source_file);

  // Return path to extracted c_bridge library
  [[nodiscard]] const spawn_fs::path &get_c_bridge_path() const noexcept
  {
    return m_embeddedFiles.dir();
  }

  // Reset current compilation unit, necessary if writing multiple
  // output files from one Compiler instance
  void reset() noexcept
  {
    m_currentCompilation.reset();
  }

  // write LLVM bitcode of current compilation unit
  void write_bitcode(const spawn_fs::path &loc);

  // write platform specific object file for current compilation unit
  void write_object_file(const spawn_fs::path &loc);

  // write platform specific DLL/so module for current compilation unit
  // this is suitable for loading with dlopen / LoadLibrary
  void write_shared_object_file(const spawn_fs::path &loc,
                                const spawn_fs::path &sysroot,
                                const std::vector<spawn_fs::path> &additional_libs = {});

  [[nodiscard]] static spawn_fs::path append_shared_object_extension(spawn_fs::path path)
  {
    return path += shared_object_extension();
  }

  // Helper utility to choose between .so and .dll for a loadable module
  [[nodiscard]] static spawn_fs::path shared_object_extension();

private:
  std::string m_target_triple{get_target_triple()};
  const llvm::Target *m_target{get_target(m_target_triple)};
  llvm::TargetMachine *m_target_machine{
      get_target_machine(m_target, m_target_triple, get_CPU(), get_features(), get_OPT(), get_reloc_model())};
  std::vector<spawn_fs::path> m_include_paths;
  std::vector<std::string> m_flags;
  llvm::orc::ThreadSafeContext m_context{std::make_unique<llvm::LLVMContext>()};
  std::unique_ptr<llvm::Module> m_currentCompilation{initialize_module(m_context, m_target_machine)};

  spawn::util::Temp_Directory m_embeddedFiles;
  bool m_use_c_bridge_instead_of_stdlib = false;

  // we package and -I our own set of headers provided by the embedded clang
  // system -I headers are found automatically by the embedded clang
  [[nodiscard]] static std::unique_ptr<llvm::Module> compile(const spawn_fs::path &source,
                                                             llvm::LLVMContext &ctx,
                                                             const std::vector<spawn_fs::path> &include_paths,
                                                             const std::vector<std::string> &flags);

  [[nodiscard]] static std::string get_target_triple()
  {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

    return llvm::sys::getDefaultTargetTriple();
  }

  [[nodiscard]] static const llvm::Target *get_target(const std::string &target_triple)
  {
    std::string error;
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(target_triple, error);

    if (target == nullptr) {
      throw std::runtime_error("Unable to get target: " + error);
    }

    return target;
  }

  [[nodiscard]] static std::unique_ptr<llvm::Module> initialize_module(llvm::orc::ThreadSafeContext ctx,
                                                                       llvm::TargetMachine *target_machine)
  {
    assert(ctx.getContext());
    auto module = std::make_unique<llvm::Module>("Module", *ctx.getContext());
    module->setDataLayout(target_machine->createDataLayout());
    return module;
  }

  [[nodiscard]] static std::string get_CPU()
  {
    return "generic";
  }

  [[nodiscard]] static std::string get_features()
  {
    return "";
  }

  [[nodiscard]] static llvm::TargetOptions get_OPT()
  {
    return llvm::TargetOptions{};
  }

  [[nodiscard]] static llvm::Optional<llvm::Reloc::Model> get_reloc_model()
  {
    return {llvm::Reloc::Model::PIC_};
  }

  [[nodiscard]] static llvm::TargetMachine *get_target_machine(const llvm::Target *target,
                                                               const std::string &triple,
                                                               const std::string &cpu,
                                                               const std::string &features,
                                                               const llvm::TargetOptions opt,
                                                               const llvm::Optional<llvm::Reloc::Model> &reloc_model)
  {
    llvm::TargetMachine *machine = target->createTargetMachine(triple, cpu, features, opt, reloc_model);
    return machine;
  }
};

} // namespace spawn

#endif
