#include <algorithm>
#include <iostream>

#include "../compiler/JIT.hpp"
#include "../compiler/compiler.hpp"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/raw_os_ostream.h"

int main(int argc, const char *argv[])
{
  if (argc < 2) {
    std::cerr << "missing file name to compile\n";
    return EXIT_FAILURE;
  }

  const std::vector<boost::filesystem::path> include_paths{};
  const std::vector<std::string> flags{"-v"};
  nrel::Compiler compiler(include_paths, flags);

  std::for_each(std::next(argv), std::next(argv, argc), [&](const auto &path) { compiler.compile_and_link(path); });

  compiler.write_bitcode("a.out.bc");
  compiler.write_object_file("a.out");

  auto jit = compiler.move_to_jit();
  auto go = jit->get_function<int()>("go");
  go();

  llvm::llvm_shutdown();

  return EXIT_SUCCESS;
}
