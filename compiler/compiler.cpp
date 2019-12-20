#include "clang/Basic/DiagnosticOptions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/raw_ostream.h"

#include <iostream>
#if defined _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <stdio.h>
#endif

#include <iterator>

#include "compiler.hpp"

using namespace clang;
using namespace clang::driver;

std::string getExecutablePath() {
#if defined _WIN32
  TCHAR szPath[MAX_PATH];
  if (GetModuleFileName(nullptr, szPath, MAX_PATH)) {
    return std::string(szPath);
  }
#else
  Dl_info info;
  if (dladdr("main", &info)) {
    return std::string(info.dli_fname);
  }
#endif
  return std::string();
}

namespace nrel {
void Compiler::compile_and_link(const boost::filesystem::path &source) {

  auto do_compile = [&]() {
    return compile(source, m_include_paths, m_flags);
  };

  if (!m_currentCompilation) {
    auto result = do_compile();
    m_currentCompilation = std::move(result.first);
    m_currentContext = std::move(result.second);
  } else {
    llvm::Linker::linkModules(*m_currentCompilation, do_compile().first);
  }
}

void Compiler::write_bitcode(const boost::filesystem::path &loc) {
  if (!m_currentCompilation) {
    throw std::runtime_error("No current compilation available to write");
  }

  std::ofstream ofs(loc.native());
  llvm::raw_os_ostream ros(ofs);
  llvm::WriteBitcodeToFile(*m_currentCompilation, ros);
}

void Compiler::write_object_file(const boost::filesystem::path &loc) {
  if (!m_currentCompilation) {
    throw std::runtime_error("No current compilation available to write");
  }

  std::string err;
  auto *TheTarget =
      llvm::TargetRegistry::lookupTarget(llvm::sys::getProcessTriple(), err);
  auto *TM = TheTarget->createTargetMachine(llvm::sys::getProcessTriple(),
                                            llvm::sys::getHostCPUName(), {}, {},
                                            {}, {}, {}, {});

  llvm::legacy::PassManager pass;
  std::string error;

  m_currentCompilation->setDataLayout(TM->createDataLayout());

  llvm::TargetMachine::CodeGenFileType ft =
      llvm::TargetMachine::CGFT_AssemblyFile;

  /*
  switch (codegen) {
  case LLVMAssemblyFile:
    ft = TargetMachine::CGFT_AssemblyFile;
    break;
  default:
    ft = TargetMachine::CGFT_ObjectFile;
    break;
  }
  */

  std::error_code EC;
  llvm::raw_fd_ostream dest(loc.c_str(), EC, llvm::sys::fs::OF_None);

  if (TM->addPassesToEmitFile(pass, dest, nullptr, ft)) {
    throw std::runtime_error("TargetMachine can't emit a file of this type");
  }

  pass.run(*m_currentCompilation);
  dest.flush();
}

std::pair<std::unique_ptr<llvm::Module>, std::unique_ptr<llvm::LLVMContext>>
Compiler::compile(const boost::filesystem::path &source,
                  const std::vector<boost::filesystem::path> &include_paths,
                  const std::vector<std::string> &flags) {
  void *MainAddr = (void *)(intptr_t)getExecutablePath;
  std::string Path = getExecutablePath();
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter *DiagClient =
      new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  // Use ELF on windows for now.
  std::string TripleStr = llvm::sys::getProcessTriple();
  llvm::Triple T(TripleStr);
  if (T.isOSBinFormatCOFF())
    T.setObjectFormat(llvm::Triple::ELF);

  Driver TheDriver(Path, T.str(), Diags);
  TheDriver.setTitle("clang interpreter");
  TheDriver.setCheckInputsExist(false);

  // a place for the strings to live
  std::vector<std::string> str_args;
  str_args.push_back(source.native());
  std::transform(include_paths.begin(), include_paths.end(),
                 std::back_inserter(str_args),
                 [](const auto &str) { return "-I" + str.native(); });
  str_args.push_back("-fsyntax-only");
  str_args.push_back("-Wno-expansion-to-defined");
  str_args.push_back("-Wno-nullability-completeness");
  std::copy(flags.begin(), flags.end(), std::back_inserter(str_args));
  str_args.push_back(source.native());

  // the strings to pass to the compiler driver
  SmallVector<const char *, 64> Args; //(argv, argv + argc);
  std::transform(str_args.begin(), str_args.end(), std::back_inserter(Args),
                 [](const auto &str) { return str.c_str(); });

  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));

  if (!C) {
    return {};
  }

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const auto &Jobs = C->getJobs();
  if (Jobs.size() != 1 || !isa<driver::Command>(*Jobs.begin())) {
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags.Report(diag::err_fe_expected_compiler_job) << OS.str();
    return {};
  }

  const auto &Cmd = cast<driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd.getCreator().getName()) != "clang") {
    Diags.Report(diag::err_fe_expected_clang_command);
    return {};
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const auto &CCArgs = Cmd.getArguments();
  std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(
      *CI, const_cast<const char **>(CCArgs.data()),
      const_cast<const char **>(CCArgs.data()) + CCArgs.size(), Diags);

  // Show the invocation, with -v.
  if (CI->getHeaderSearchOpts().Verbose) {
    llvm::errs() << "clang invocation:\n";
    Jobs.Print(llvm::errs(), "\n", true);
    llvm::errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  CompilerInstance Clang;
  Clang.setInvocation(std::move(CI));

  // Create the compilers actual diagnostics engine.
  Clang.createDiagnostics();
  if (!Clang.hasDiagnostics())
    return {};

  // Infer the builtin include path if unspecified.
  // if (Clang.getHeaderSearchOpts().UseBuiltinIncludes &&
  //    Clang.getHeaderSearchOpts().ResourceDir.empty())
  Clang.getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath(getExecutablePath().c_str(),
                                           MainAddr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  std::unique_ptr<CodeGenAction> Act(new EmitLLVMOnlyAction());
  if (!Clang.ExecuteAction(*Act)) {
    return {};
  }

  return std::pair{Act->takeModule(), std::unique_ptr<llvm::LLVMContext>{Act->takeLLVMContext()}};
}

} // namespace nrel
