#include <algorithm>
#include <iostream>

#include "../compiler/compiler.hpp"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/raw_os_ostream.h"

#ifdef _MSC_VER
#define EXPORT_SYMBOL __declspec(dllexport)
#else
#define EXPORT_SYMBOL
#endif

extern "C" {
static int call_count = 0;
EXPORT_SYMBOL void call()
{
  ++call_count;
}
}

int main(int argc, const char *argv[])
{
  if (argc < 2) {
    std::cerr << "missing file name to compile\n";
    return EXIT_FAILURE;
  }

  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{"-v"};
  spawn::Compiler compiler(include_paths, flags, true);

  const std::vector<spawn_fs::path> source_files{std::next(argv), std::next(argv, argc)};

  std::for_each(begin(source_files), end(source_files), [&](const auto &path) { compiler.compile_and_link(path); });

  compiler.write_bitcode("a.bc");
  compiler.write_object_file("a.o");
  compiler.write_shared_object_file("a.so", {});

  /*
  auto jit = compiler.move_to_jit();
  auto go = jit->get_function<int()>("go");
  go();
  */

  // llvm::llvm_shutdown();

  std::cout << "call_count: " << call_count << '\n';
  assert(call_count == 1);

  return EXIT_SUCCESS;
}
