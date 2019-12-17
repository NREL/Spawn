#include <iostream>

#include "../compiler/compiler.hpp"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/Linker/Linker.h"

int main(int argc, const char *argv[])
{
  if (argc != 3) {
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

  std::vector<std::string> args;
  args.push_back("-v");

  llvm::LLVMContext context;
  const auto result1 = compile(argv[1], context, include_paths, args);
  auto result2 = compile(argv[2], context, include_paths, args);
  std::ofstream ofs("a.out");
  llvm::Linker::linkModules(*result1, std::move(result2));
  llvm::raw_os_ostream ros(ofs);
  llvm::WriteBitcodeToFile(*result1, ros);

  return EXIT_SUCCESS;
}
