project(Compiler)

find_package(LLVM REQUIRED CONFIG)
find_package(Clang REQUIRED CONFIG )

#include_directories(${LLVM_INCLUDE_DIRS})
#include_directories(${CLANG_INCLUDE_DIRS})

add_library( compiler
  compiler.hpp
  compiler.cpp
)

target_link_libraries(compiler
  LLVMMCJIT
  LLVMMIRParser
  LLVMX86AsmParser
  #  LLVMX86AsmPrinter
  LLVMX86CodeGen
  libclang
  clangBasic
  clangLex
  clangParse
  clangCodeGen
)

