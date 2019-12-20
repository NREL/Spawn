project(Compiler)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG)

# include_directories(${LLVM_INCLUDE_DIRS})
# include_directories(${CLANG_INCLUDE_DIRS})

add_library(compiler compiler.hpp compiler.cpp)

target_link_libraries(
  compiler
  LLVMMCJIT
  LLVMMIRParser
  LLVMX86AsmParser
  # LLVMX86AsmPrinter
  LLVMX86CodeGen
  libclang
  clangBasic
  clangLex
  clangParse
  clangCodeGen
  CONAN_PKG::boost)


add_executable(jit_compiler compiler2.cpp)

target_link_libraries(
  jit_compiler
  LLVMMCJIT
  LLVMMIRParser
  LLVMX86AsmParser
  # LLVMX86AsmPrinter
  LLVMX86CodeGen
  LLVMOrcJIT
  libclang
  clangBasic
  clangLex
  clangParse
  clangCodeGen
  CONAN_PKG::boost)


