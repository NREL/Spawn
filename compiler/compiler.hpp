#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include <boost/filesystem.hpp>

std::unique_ptr<llvm::Module> compile(const boost::filesystem::path &source,
                                      llvm::LLVMContext &context,
                                      const std::vector<boost::filesystem::path> &include_paths,
                                      const std::vector<std::string> &args);
