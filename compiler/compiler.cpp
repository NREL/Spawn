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
#include "llvm/Target/TargetMachine.h"

#include "lld/Common/Driver.h"
#include "lld/Common/ErrorHandler.h"

#include <fmt/format.h>
#include <spdlog/spdlog.h>

#include "../util/filesystem.hpp"
#include "../util/temp_directory.hpp"

#include <iostream>
#if defined _WIN32
#include <windows.h>
#else
#include <cstdio>
#include <dlfcn.h>
#endif

#include <codecvt>
#include <iterator>
#include <sstream>
#include <stdexcept>

#include "compiler.hpp"
#include "compiler/embedded_files.hxx"

std::string toString(const std::wstring &utf16_string)
{
#if _MSC_VER >= 1900
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> convert;
  return convert.to_bytes(utf16_string.data(), utf16_string.data() + utf16_string.size());
#else
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  const std::u16string u16string{utf16_string.begin(), utf16_string.end()};
  return convert.to_bytes(u16string);
#endif
}

std::string toString(const std::string &str)
{
  return str;
}

std::string toString(const spawn_fs::path &path)
{
  return toString(path.native());
}

std::string getExecutablePath()
{
#if defined _WIN32
  TCHAR szPath[MAX_PATH];
  if (GetModuleFileName(nullptr, szPath, MAX_PATH)) {
    return std::string(szPath);
  }
#else
  Dl_info info;
  if (dladdr("main", &info) == 0) {
    return std::string(info.dli_fname);
  }
#endif
  return std::string();
}

namespace spawn {

spawn_fs::path Compiler::shared_object_extension()
{
#ifdef _MSC_VER
  return ".dll";
#else
  return ".so";
#endif
}

Compiler::Compiler(std::vector<spawn_fs::path> include_paths,
                   std::vector<std::string> flags,
                   bool use_cbridge_instead_of_stdlib)
    : m_include_paths{std::move(include_paths)}, m_flags{std::move(flags)}, m_use_cbridge_instead_of_stdlib{
                                                                                use_cbridge_instead_of_stdlib}
{
  for (const auto &file : spawnmodelica_compiler::embedded_files::fileNames()) {
    spawnmodelica_compiler::embedded_files::extractFile(file, m_embeddedFiles.dir().string());
  }
}

void Compiler::add_c_bridge_to_path()
{
#ifdef _MSC_VER
  constexpr std::size_t ENVSIZE = 2048;
  TCHAR path[ENVSIZE];

  const auto result = GetEnvironmentVariable("PATH", path, ENVSIZE);
  if (result <= ENVSIZE) {
    std::basic_string<TCHAR> newpath(path, result);
    newpath = (m_embeddedFiles.dir() / "c_bridge").string() + ";" + newpath;
    SetEnvironmentVariable("PATH", newpath.c_str());
  }
#else
  const auto library_path = []() -> std::string {
    const char *path = getenv("LD_LIBRARY_PATH");
    if (path != nullptr) {
      return path;
    } else {
      return "";
    }
  }();

  setenv("LD_LIBRARY_PATH", ((m_embeddedFiles.dir() / "c_bridge").string() + ":" + library_path).c_str(), 1);
#endif
}

void Compiler::write_shared_object_file(const spawn_fs::path &loc,
                                        const spawn_fs::path &sysroot,
                                        const std::vector<spawn_fs::path> &additional_libs)
{
  {
    spawn::util::Temp_Directory td;
    const spawn_fs::path test_file_path = td.dir() / "test.c";

    std::ofstream test_file(test_file_path);
    test_file <<
        R"(
#ifdef _MSC_VER
#define BOOL int
#define WINAPI __stdcall

// just fake it
BOOL WINAPI _DllMainCRTStartup(void *hinstDLL, unsigned long fdwReason, void *lpReserved)
{
  return 1;
}

#undef WINAPI
#undef BOOL
#endif
)" << std::flush;

    compile_and_link(test_file_path);
  }

  util::Temp_Directory td;
  const auto temporary_object_file_location = td.dir() / "temporary_object.obj";
  write_object_file(temporary_object_file_location);

  std::vector<std::string> str_args{
#ifdef _MSC_VER
      "ld-link",
      "/dll",
      "/subsystem:windows",
      "/machine:x64",
      "/dynamicbase",
      "/nxcompat",
      "/opt:ref",
      "/nologo",
      "/tlbid:1",
      "/opt:icf",
      "/force:unresolved",
#else
      "ld.lld-10",
      "-shared",
      fmt::format("--sysroot={}", toString(sysroot)),
      fmt::format("-L{}", toString(sysroot / "usr/lib/")),
      fmt::format("-L{}", toString(sysroot / "usr/lib/x86_64-linux-gnu/")),
#endif
      toString(temporary_object_file_location)};

  for (const auto &lib : additional_libs) {
    str_args.push_back(toString(lib));
  }

  if (m_use_cbridge_instead_of_stdlib) {
    str_args.insert(str_args.end(),
                    {
#ifdef _MSC_VER
                        (m_embeddedFiles.dir() / "c_bridge" / "c_bridge.lib").string()
#else
                        "-L" + (m_embeddedFiles.dir() / "c_bridge").string(),
                        "-lc_bridge",
                        "-rpath=" + (m_embeddedFiles.dir() / "c_bridge").string()

#endif
                    });

  } else {
    str_args.insert(
        str_args.end(),
        {
#ifdef _MSC_VER
            "c:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC/14.31.31103/lib/x64/libcmt.lib",
            "c:/Program Files/Microsoft Visual "
            "Studio/2022/Community/VC/Tools/MSVC/14.31.31103/lib/x64/libvcruntime.lib",
            "c:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/ucrt/x64/libucrt.lib",
            "c:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64/kernel32.lib",
            "c:/Program Files (x86)/Windows Kits/10/Lib/10.0.19041.0/um/x64/uuid.lib",
#else
            "-L/usr/lib",
            "-L/lib",
            "-L/lib/x86_64-linux-gnu",
            "-lm",
            "-lc",
            "-ldl",
            "-lpthread",

#endif
        });
  }

  str_args.insert(str_args.end(),
                  {
#ifdef _MSC_VER
                      fmt::format("/out:{}", toString(loc)),
#else
                      "-o",
                      toString(loc),
#endif
                  });

  for (const auto &arg : str_args) {
    spdlog::trace("embedded lld argument: {}", arg);
  }

  clang::SmallVector<const char *, 64> Args{};

  std::transform(
      str_args.begin(), str_args.end(), std::back_inserter(Args), [](const auto &str) { return str.c_str(); });

  spdlog::info("linking to: {}", toString(loc));

  bool success = true;

  spdlog::trace("Temp obj file size: {}", spawn_fs::file_size(temporary_object_file_location));

  std::stringstream out_ss;
  std::stringstream err_ss;

  { // scope to ensure error stream buffer is flushed
    llvm::raw_os_ostream err(err_ss);
    llvm::raw_os_ostream out(out_ss);

#ifdef _MSC_VER
    // success = lld::coff::link(Args, false, out, err);
    std::string lld_cmd = "\"C:/Program Files/LLVM/bin/lld-link.exe\"";
    auto newargs = str_args;
    newargs.erase(newargs.begin());
    for (const auto &arg : newargs) {
      lld_cmd += fmt::format(" \"{}\"", arg);
    }
    lld_cmd = fmt::format("\"{}\"", lld_cmd);

    spdlog::trace("Calling: `{}`", lld_cmd);
    system(lld_cmd.c_str());
#else
    success = lld::elf::link(Args, false /*canExitEarly*/, out, err);
#endif

    if (!success) {
      spdlog::error("Linking errors with {} errors", lld::errorHandler().errorCount);
    }
  }

  const auto errors = err_ss.str();

  if (!success) {
    throw std::runtime_error(fmt::format("Error with linking {}, errors '{}'", toString(loc), errors));
  }

  if (success && !errors.empty()) {
    spdlog::warn("Linking warnings: '{}'", errors);
  }
}

void Compiler::compile_and_link(const spawn_fs::path &source)
{
  auto include_paths = m_include_paths;
  if (m_use_cbridge_instead_of_stdlib) {
    include_paths.insert(include_paths.begin(), (m_embeddedFiles.dir() / "c_bridge/include").string());
  }

  auto flags = m_flags;

  if (m_use_cbridge_instead_of_stdlib) {
    flags.insert(flags.begin(), "-nobuiltininc");
  }

  auto do_compile = [&]() { return compile(source, *m_context.getContext(), include_paths, m_flags); };

  if (!m_currentCompilation) {
    m_currentCompilation = do_compile();
  } else {
    llvm::Linker::linkModules(*m_currentCompilation, do_compile());
  }
}

void Compiler::write_bitcode(const spawn_fs::path &loc)
{
  if (!m_currentCompilation) {
    throw std::runtime_error("No current compilation available to write");
  }

  std::ofstream ofs(loc.native());
  llvm::raw_os_ostream ros(ofs);

  llvm::WriteBitcodeToFile(*m_currentCompilation, ros);
}

void Compiler::write_object_file(const spawn_fs::path &loc)
{
  if (!m_currentCompilation) {
    throw std::runtime_error("No current compilation available to write");
  }

  std::string err;

  llvm::legacy::PassManager pass;
  std::string error;

  llvm::CodeGenFileType ft = llvm::CGFT_ObjectFile;

  std::error_code EC;
  std::string sloc = toString(loc);
  llvm::raw_fd_ostream destination(sloc, EC, llvm::sys::fs::OF_None);

  if (m_target_machine->addPassesToEmitFile(pass, destination, nullptr, ft)) {
    throw std::runtime_error("TargetMachine can't emit a file of this type");
  }

  pass.run(*m_currentCompilation);
  destination.flush();
}

std::unique_ptr<llvm::Module> Compiler::compile(const spawn_fs::path &source,
                                                llvm::LLVMContext &ctx,
                                                const std::vector<spawn_fs::path> &include_paths,
                                                const std::vector<std::string> &flags)
{
  void *MainAddr = reinterpret_cast<void *>(getExecutablePath); // NOLINT we have to get a void * out of this somehow
  std::string Path = getExecutablePath();
  clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts{new clang::DiagnosticOptions()};
  clang::TextDiagnosticPrinter *DiagClient{new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts)};

  clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID{new clang::DiagnosticIDs()};
  clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  // Examples say to use ELF on Windows, but that doesn't actually work, so we are not
  clang::driver::Driver TheDriver(Path, llvm::sys::getProcessTriple(), Diags);
  TheDriver.setTitle("clang interpreter");
  TheDriver.setCheckInputsExist(false);

  // a place for the strings to live
  std::vector<std::string> str_args;
  str_args.push_back(toString(source));
  std::transform(include_paths.begin(), include_paths.end(), std::back_inserter(str_args), [](const auto &str) {
    return "-I" + toString(str);
  });

  str_args.emplace_back("-fsyntax-only");
  // str_args.emplace_back("-fPIC");
  str_args.emplace_back("-g");
  str_args.emplace_back("-Wno-incomplete-setjmp-declaration");
  str_args.emplace_back("-Wno-expansion-to-defined");
  str_args.emplace_back("-Wno-nullability-completeness");
  std::copy(flags.begin(), flags.end(), std::back_inserter(str_args));
  str_args.push_back(toString(source));

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
  clang::CompilerInvocation::CreateFromArgs(*CI, CCArgs, Diags);

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
  if (!Clang.hasDiagnostics()) {
    return {};
  }

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
