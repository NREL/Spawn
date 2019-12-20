#ifndef NREL_SPAWN_COMPILER_HPP
#define NREL_SPAWN_COMPILER_HPP

#include "JIT.hpp"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"
#include <boost/filesystem.hpp>

namespace nrel {

class Compiler
{
public:
  explicit Compiler(std::vector<boost::filesystem::path> include_paths, std::vector<std::string> flags)
      : m_include_paths{std::move(include_paths)}, m_flags{std::move(flags)}
  {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
  }

  void compile_and_link(const boost::filesystem::path &source);

  std::unique_ptr<llvm::Module> take_compilation()
  {
    return std::move(m_currentCompilation);
  }

  auto context()
  {
    return m_context;
  }

  auto move_to_jit()
  {
    auto jit = llvm::orc::SimpleJIT::Create();
    if (!jit) {
      throw std::runtime_error("Some error happened, but it's too much of a pain to get it out of the Expected object");
    }
    move_to_jit(*jit.get());
    return std::move(*jit);
  }

  void move_to_jit(llvm::orc::SimpleJIT &jit) {
    jit.addModule(take_compilation(), context());
  }

  void write_bitcode(const boost::filesystem::path &loc);
  void write_object_file(const boost::filesystem::path &loc);

private:
  std::vector<boost::filesystem::path> m_include_paths;
  std::vector<std::string> m_flags;
  std::unique_ptr<llvm::Module> m_currentCompilation;
  llvm::orc::ThreadSafeContext m_context{std::make_unique<llvm::LLVMContext>()};

  static std::unique_ptr<llvm::Module> compile(const boost::filesystem::path &source,
                                               llvm::LLVMContext &ctx,
                                               const std::vector<boost::filesystem::path> &include_paths,
                                               const std::vector<std::string> &flags);
};

} // namespace nrel

#endif
