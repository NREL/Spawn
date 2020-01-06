#include <algorithm>
#include <iostream>

#include "../compiler/JIT.hpp"
#include "../compiler/compiler.hpp"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/raw_os_ostream.h"

int main(int argc, const char *argv[])
{
  puts("Invoking thingsy");

//  if (argc < 2) {
//    std::cerr << "missing file name to compile\n";
//    return EXIT_FAILURE;
//  }

  const std::vector<boost::filesystem::path> include_paths{};
  const std::vector<std::string> flags{"-v"};
  spawn::Compiler compiler(include_paths, flags);
  compiler.invoke_compiler({argv[0], "-shared", "/home/jason/Spawn/test/go.c", "-ooutput.so"});

  const std::vector<boost::filesystem::path> source_files{std::next(argv), std::next(argv, argc)};




  std::for_each(begin(source_files), end(source_files), [&](const auto &path) { compiler.compile_and_link(path); });

  compiler.write_bitcode("a.out.bc");
  compiler.write_object_file("a.out");



  auto jit = compiler.move_to_jit();
  auto go = jit->get_function<int()>("go");
  go();

  llvm::llvm_shutdown();

  return EXIT_SUCCESS;
}
