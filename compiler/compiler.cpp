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
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/TargetSelect.h"
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

std::string getExecutablePath()
{
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

std::unique_ptr<llvm::Module> compile(const boost::filesystem::path &source,
                                      llvm::LLVMContext &context,
                                      const std::vector<boost::filesystem::path> &include_paths,
                                      const std::vector<std::string> &flags)
{
  void *MainAddr = (void *)(intptr_t)getExecutablePath;
  std::string Path = getExecutablePath();
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter *DiagClient = new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  // Use ELF on windows for now.
  std::string TripleStr = llvm::sys::getProcessTriple();
  llvm::Triple T(TripleStr);
  if (T.isOSBinFormatCOFF()) T.setObjectFormat(llvm::Triple::ELF);

  Driver TheDriver(Path, T.str(), Diags);
  TheDriver.setTitle("clang interpreter");
  TheDriver.setCheckInputsExist(false);

  // a place for the strings to live
  std::vector<std::string> str_args;
  str_args.push_back(source.native());
  std::transform(include_paths.begin(), include_paths.end(), std::back_inserter(str_args), [](const auto &str) { return "-I" + str.native(); });
  str_args.push_back("-fsyntax-only");
  str_args.push_back("-Wno-expansion-to-defined");
  str_args.push_back("-Wno-nullability-completeness");
  std::copy(flags.begin(), flags.end(), std::back_inserter(str_args));
  str_args.push_back(source.native());

  // the strings to pass to the compiler driver
  SmallVector<const char *, 64> Args; //(argv, argv + argc);
  std::transform(str_args.begin(), str_args.end(), std::back_inserter(Args), [](const auto &str) { return str.c_str(); });

  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));

  if (!C) {
    return nullptr;
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
    return nullptr;
  }

  const auto &Cmd = cast<driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd.getCreator().getName()) != "clang") {
    Diags.Report(diag::err_fe_expected_clang_command);
    return nullptr;
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const auto &CCArgs = Cmd.getArguments();
  std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*CI, const_cast<const char **>(CCArgs.data()), const_cast<const char **>(CCArgs.data()) + CCArgs.size(), Diags);

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
  if (!Clang.hasDiagnostics()) return nullptr;

  // Infer the builtin include path if unspecified.
  // if (Clang.getHeaderSearchOpts().UseBuiltinIncludes &&
  //    Clang.getHeaderSearchOpts().ResourceDir.empty())
  Clang.getHeaderSearchOpts().ResourceDir = CompilerInvocation::GetResourcesPath(getExecutablePath().c_str(), MainAddr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  std::unique_ptr<CodeGenAction> Act(new EmitLLVMOnlyAction(&context));
  if (!Clang.ExecuteAction(*Act)) {
    return nullptr;
  }

  return Act->takeModule();
}

