#include <algorithm>
#include <iostream>

#include "../compiler/JIT.hpp"
#include "../compiler/compiler.hpp"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Support/raw_os_ostream.h"

int main(int argc, const char *argv[])
{
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  if (argc < 2) {
    std::cerr << "missing file name to compile\n";
    return EXIT_FAILURE;
  }

  std::vector<boost::filesystem::path> include_paths;
  //  include_paths.push_back("/Users/kbenne/development/EnergyPlus/build-third-party/jmodelica-prefix/src/jmodelica-install/include/RuntimeLibrary/");
  //  include_paths.push_back("/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.11.sdk/usr/include");
  //  include_paths.push_back("/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.12.sdk/usr/include");
  //  include_paths.push_back("/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/7.0.2/include");
  //  include_paths.push_back("/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/lib/clang/8.1.0/include");
  //  include_paths.push_back("/usr/lib/gcc/x86_64-linux-gnu/5/include");
  include_paths.push_back("/usr/local/include");
  //  include_paths.push_back("/usr/lib/gcc/x86_64-linux-gnu/5/include-fixed");
  //  include_paths.push_back("/usr/include/x86_64-linux-gnu");
  include_paths.push_back("/usr/include");
  include_paths.push_back("/usr/local/lib/clang/9.0.0/include");

  nrel::Compiler compiler(include_paths, {"-v"});

  std::for_each(std::next(argv), std::next(argv, argc), [&](const auto &path) { compiler.compile_and_link(path); });

  //compiler.write_bitcode("a.out.bc");
//  compiler.write_object_file("a.out");

  llvm::raw_os_ostream cerr(std::cerr);
  auto jit = llvm::orc::SimpleJIT::Create();
  if (!jit) {
    std::cerr << "Error creating JIT\n";
    cerr << jit.takeError();
    return EXIT_FAILURE;
  }

  jit.get()->addModule(llvm::orc::ThreadSafeModule(compiler.take_compilation(), compiler.take_context()));

  auto go = (int (*)())jit.get()->getSymbolAddress("go").get();
  go();

  return EXIT_SUCCESS;
}
