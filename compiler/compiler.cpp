#include "clang/Basic/DiagnosticOptions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/CodeGen/ObjectFilePCHContainerOperations.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Options.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticBuffer.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/FrontendTool/Utils.h"

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
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/raw_ostream.h"

#include <fmt/format.h>

#include "compiler/embedded_files.hxx"
#include "utility.hpp"

#include <iostream>
#if defined _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#include <stdio.h>
#endif

#include <iterator>

#include "compiler.hpp"

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

namespace spawn {
void Compiler::write_shared_object_file(const boost::filesystem::path &loc, std::vector<boost::filesystem::path> additional_libs)
{
  Temp_Directory td;
  const auto temporary_object_file_location = td.dir() / "temporary_object.o";
  write_object_file(temporary_object_file_location);

  std::string libargs;
  std::for_each(
      std::begin(additional_libs),
      std::end(additional_libs),
      [&libargs](const auto & p) {
        return libargs.append(p.native() + " ");
      }
  );

  system(fmt::format("ld -shared {} {} -o {} -shared", temporary_object_file_location.native(), libargs, loc.native()).c_str());
}

void Compiler::compile_and_link(const boost::filesystem::path &source)
{

  auto do_compile = [&]() { return compile(source, *m_context.getContext(), m_include_paths, m_flags); };

  if (!m_currentCompilation) {
    m_currentCompilation = do_compile();
  } else {
    llvm::Linker::linkModules(*m_currentCompilation, do_compile());
  }
}

void Compiler::write_bitcode(const boost::filesystem::path &loc)
{
  if (!m_currentCompilation) {
    throw std::runtime_error("No current compilation available to write");
  }

  std::ofstream ofs(loc.native());
  llvm::raw_os_ostream ros(ofs);
  llvm::WriteBitcodeToFile(*m_currentCompilation, ros);
}

void Compiler::write_object_file(const boost::filesystem::path &loc)
{
  if (!m_currentCompilation) {
    throw std::runtime_error("No current compilation available to write");
  }

  std::string err;

  llvm::legacy::PassManager pass;
  std::string error;

  llvm::TargetMachine::CodeGenFileType ft = llvm::TargetMachine::CGFT_ObjectFile;

  std::error_code EC;
  llvm::raw_fd_ostream destination(loc.c_str(), EC, llvm::sys::fs::OF_None);

  if (m_target_machine->addPassesToEmitFile(pass, destination, nullptr, ft)) {
    throw std::runtime_error("TargetMachine can't emit a file of this type");
  }

  pass.run(*m_currentCompilation);
  destination.flush();
}

class Embedded_Headers
{
public:
  Embedded_Headers()
  {
    for (const auto &file : spawn::embedded_files::fileNames()) {
      spawn::embedded_files::extractFile(file, td.dir().native());
    }
  }
  boost::filesystem::path include_path() const
  {
    return td.dir() / "include";
  }

private:
  spawn::Temp_Directory td;
};

std::unique_ptr<llvm::Module> Compiler::compile(const boost::filesystem::path &source,
                                                llvm::LLVMContext &ctx,
                                                const std::vector<boost::filesystem::path> &include_paths,
                                                const std::vector<std::string> &flags)
{
  void *MainAddr = (void *)(intptr_t)getExecutablePath;
  std::string Path = getExecutablePath();
  clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts{new clang::DiagnosticOptions()};
  clang::TextDiagnosticPrinter *DiagClient{new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts)};

  clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID{new clang::DiagnosticIDs()};
  clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  // Use ELF on windows for now.
  std::string TripleStr = llvm::sys::getProcessTriple();
  llvm::Triple T(TripleStr);
  if (T.isOSBinFormatCOFF()) T.setObjectFormat(llvm::Triple::ELF);

  clang::driver::Driver TheDriver(Path, T.str(), Diags);
  TheDriver.setTitle("clang interpreter");
  TheDriver.setCheckInputsExist(false);

  Embedded_Headers embedded_headers;
  // a place for the strings to live
  std::vector<std::string> str_args;
  str_args.push_back(source.native());
  str_args.push_back("-I" + embedded_headers.include_path().native());
  std::transform(include_paths.begin(), include_paths.end(), std::back_inserter(str_args), [](const auto &str) {
    return "-I" + str.native();
  });
  str_args.push_back("-fsyntax-only");
  str_args.push_back("-Wno-expansion-to-defined");
  str_args.push_back("-Wno-nullability-completeness");
  std::copy(flags.begin(), flags.end(), std::back_inserter(str_args));
  str_args.push_back(source.native());

  // the strings to pass to the compiler driver
  clang::SmallVector<const char *, 64> Args; //(argv, argv + argc);
  std::transform(
      str_args.begin(), str_args.end(), std::back_inserter(Args), [](const auto &str) { return str.c_str(); });

  std::unique_ptr<clang::driver::Compilation> C(TheDriver.BuildCompilation(Args));

  if (!C) {
    return {};
  }

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const auto &Jobs = C->getJobs();
  if (Jobs.size() != 1 || !clang::isa<clang::driver::Command>(*Jobs.begin())) {
    clang::SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags.Report(clang::diag::err_fe_expected_compiler_job) << OS.str();
    return {};
  }

  const auto &Cmd = clang::cast<clang::driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd.getCreator().getName()) != "clang") {
    Diags.Report(clang::diag::err_fe_expected_clang_command);
    return {};
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const auto &CCArgs = Cmd.getArguments();
  std::unique_ptr<clang::CompilerInvocation> CI{new clang::CompilerInvocation};
  clang::CompilerInvocation::CreateFromArgs(
      *CI, const_cast<const char **>(CCArgs.data()), const_cast<const char **>(CCArgs.data()) + CCArgs.size(), Diags);

  // Show the invocation, with -v.
  if (CI->getHeaderSearchOpts().Verbose) {
    llvm::errs() << "clang invocation:\n";
    Jobs.Print(llvm::errs(), "\n", true);
    llvm::errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  clang::CompilerInstance Clang;
  Clang.setInvocation(std::move(CI));

  // Create the compilers actual diagnostics engine.
  Clang.createDiagnostics();
  if (!Clang.hasDiagnostics()) return {};

  // Infer the builtin include path if unspecified.
  // if (Clang.getHeaderSearchOpts().UseBuiltinIncludes &&
  //    Clang.getHeaderSearchOpts().ResourceDir.empty())
  Clang.getHeaderSearchOpts().ResourceDir =
      clang::CompilerInvocation::GetResourcesPath(getExecutablePath().c_str(), MainAddr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  std::unique_ptr<clang::CodeGenAction> Act(new clang::EmitLLVMOnlyAction(&ctx));
  if (!Clang.ExecuteAction(*Act)) {
    return {};
  }

  return Act->takeModule();
}


} // namespace spawn
