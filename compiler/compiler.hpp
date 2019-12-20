#ifndef NREL_SPAWN_COMPILER_HPP
#define NREL_SPAWN_COMPILER_HPP

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
  }

  void compile_and_link(const boost::filesystem::path &source);

  std::unique_ptr<llvm::Module> take_compilation()
  {
    return std::move(m_currentCompilation);
  }

  std::unique_ptr<llvm::LLVMContext> take_context()
  {
    return std::move(m_currentContext);
  }

  void write_bitcode(const boost::filesystem::path &loc);
  void write_object_file(const boost::filesystem::path &loc);

private:
  std::vector<boost::filesystem::path> m_include_paths;
  std::vector<std::string> m_flags;
  std::unique_ptr<llvm::Module> m_currentCompilation;
  std::unique_ptr<llvm::LLVMContext> m_currentContext;

  static std::pair<std::unique_ptr<llvm::Module>, std::unique_ptr<llvm::LLVMContext>>
  compile(const boost::filesystem::path &source, const std::vector<boost::filesystem::path> &include_paths, const std::vector<std::string> &flags);
};

} // namespace nrel

#endif
