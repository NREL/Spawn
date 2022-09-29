#include "clang/Basic/DiagnosticOptions.h"
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/FrontendTool/Utils.h"

#include "llvm/ADT/SmallString.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/Linker/Linker.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_os_ostream.h"
#include "llvm/Support/raw_ostream.h"

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
#include <dlfcn.h>
#endif

#include <stdexcept>

#include "compiler.hpp"
#include "compiler/embedded_files.hxx"

// TODO:
//  * I don't like the #ifdef's but honestly don't see a better way to handle it at the moment

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
    return {info.dli_fname};
  }
#endif
  return {};
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

// finds a file and adds the directory in which the file was found to the front of the starting_dirs
// searched on the next iteration.
spawn_fs::path find_file(std::vector<spawn_fs::path> &starting_dirs, const spawn_fs::path &file_to_find)
{
  for (const auto &starting_dir : starting_dirs) {
    try {
      for (const auto &entry :
           spawn_fs::recursive_directory_iterator(starting_dir, spawn_fs::directory_options::skip_permission_denied)) {
        if (spawn_fs::is_directory(entry.path())) {
          auto potential_path = entry.path() / file_to_find;
          if (spawn_fs::exists(potential_path)) {
            spdlog::info("File: '{}' found: '{}'", file_to_find.string(), potential_path.string());
            starting_dirs.insert(starting_dirs.begin(), entry.path());
            return potential_path;
          }
        }
      }
    } catch (const std::exception &e) {
      spdlog::info("error during directory iteration: '{}'", e.what());
    }
  }

  return {};
}

Compiler::Compiler(std::vector<spawn_fs::path> include_paths,
                   std::vector<std::string> flags,
                   bool use_c_bridge_instead_of_stdlib)
    : m_include_paths{std::move(include_paths)}, m_flags{std::move(flags)}, m_use_c_bridge_instead_of_stdlib{
                                                                                use_c_bridge_instead_of_stdlib}
{
  for (const auto &file : spawnmodelica_compiler::embedded_files::fileNames()) {
    spawnmodelica_compiler::embedded_files::extractFile(file, m_embeddedFiles.dir().string());
  }
}

void Compiler::add_c_bridge_to_path() const
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
    const char *path = getenv("LD_LIBRARY_PATH"); // NOLINT function is thread unsafe
    if (path != nullptr) {
      return path;
    } else {
      return "";
    }
  }();

  const auto new_ld_library_path =
      fmt::format("{}:{}", (get_c_bridge_path() / "c_bridge").string(), library_path);
  setenv("LD_LIBRARY_PATH", new_ld_library_path.c_str(), 1); // NOLINT
#endif
}

void Compiler::write_shared_object_file(const spawn_fs::path &loc,
                                        const spawn_fs::path &sysroot,
                                        const std::vector<spawn_fs::path> &additional_libs)
{
#ifdef _MSC_VER
  if (m_use_c_bridge_instead_of_stdlib) {
    spawn::util::Temp_Directory td;
    const spawn_fs::path test_file_path = td.dir() / "test.c";

    std::ofstream test_file(test_file_path);
    test_file <<
        R"(
#ifdef _MSC_VER
#define BOOL int
#define WINAPI __stdcall

// stub in the missing _DllMainCRTStartup for windows
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
#endif

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
      "-rpath=$ORIGIN",
      fmt::format("--sysroot={}", sysroot.string()),
      fmt::format("-L{}", (sysroot / "usr/lib/").string()),
      fmt::format("-L{}", (sysroot / "usr/lib/x86_64-linux-gnu/").string()),
      "--allow-multiple-definition",
#endif
      temporary_object_file_location.string()};

  for (const auto &lib : additional_libs) {
    str_args.push_back(lib.string());
  }

  if (m_use_c_bridge_instead_of_stdlib) {
    str_args.insert(str_args.end(),
                    {
#ifdef _MSC_VER
                        (m_embeddedFiles.dir() / "c_bridge" / "c_bridge.lib").string()
#else
                        "-L" + (m_embeddedFiles.dir() / "c_bridge").string(),
                        "-lc_bridge",
                        // fix up rpath to installed c_bridge location
                        "-rpath=" + (m_embeddedFiles.dir() / "c_bridge").string()

#endif
                    });

  } else {

    std::vector<spawn_fs::path> search_paths{"c:/Program Files/Microsoft Visual Studio/2022",
                                             "c:/Program Files/Microsoft Visual Studio",
                                             "c:/Program Files (x86)/Windows Kits/10/Lib",
                                             "c:/Program Files (x86)/Windows Kits"};

    str_args.insert(str_args.end(),
                    {
#ifdef _MSC_VER
                        find_file(search_paths, spawn_fs::path{"x64/libcmt.lib"}).string(),
                        find_file(search_paths, spawn_fs::path{"x64/libvcruntime.lib"}).string(),
                        find_file(search_paths, spawn_fs::path{"x64/libucrt.lib"}).string(),
                        find_file(search_paths, spawn_fs::path{"x64/kernel32.lib"}).string(),
                        find_file(search_paths, spawn_fs::path{"x64/uuid.lib"}).string(),
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
                      fmt::format("/out:{}", loc.string()),
#else
                      "-o",
                      loc.string(),
#endif
                  });

  for (const auto &arg : str_args) {
    spdlog::trace("embedded lld argument: {}", arg);
  }

  clang::SmallVector<const char *, 64> Args{};

  std::transform(
      str_args.begin(), str_args.end(), std::back_inserter(Args), [](const auto &str) { return str.c_str(); });

  spdlog::info("linking to: {}", loc.string());

  bool success = true;

  spdlog::trace("Temp obj file size: {}", spawn_fs::file_size(temporary_object_file_location));

  std::stringstream out_ss;
  std::stringstream err_ss;

  { // scope to ensure error stream buffer is flushed
    llvm::raw_os_ostream err(err_ss);
    llvm::raw_os_ostream out(out_ss);

#ifdef _MSC_VER
    // success = lld::coff::link(Args, false, out, err);
    // For now we are calling into an embedded lld-link.exe
    std::string lld_cmd = '\"' + (m_embeddedFiles.dir() / "lld-link.exe").string() + '\"';
    auto newargs = str_args;
    newargs.erase(newargs.begin());
    for (const auto &arg : newargs) {
      lld_cmd += fmt::format(" \"{}\"", arg);
    }
    lld_cmd = fmt::format("\"{}\"", lld_cmd);

    spdlog::trace("Calling: `{}`", lld_cmd);
    success = (system(lld_cmd.c_str()) == 0);
#else
    success = lld::elf::link(Args, false /*canExitEarly*/, out, err);
#endif

    if (!success) {
      spdlog::error("Linking errors with {} errors", lld::errorHandler().errorCount);
    }
  }

  const auto errors = err_ss.str();

  if (!success) {
    throw std::runtime_error(fmt::format("Error with linking {}, errors '{}'", loc.string(), errors));
  } else if (!errors.empty()) {
    spdlog::warn("Linking warnings: '{}'", errors);
  }
}

void Compiler::compile_and_link(const std::string_view source)
{
  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << source << std::flush;
  }

  compile_and_link(test_file_path);
}

void Compiler::compile_and_link(const spawn_fs::path &source)
{
  auto include_paths = m_include_paths;
  auto flags = m_flags;

  if (m_use_c_bridge_instead_of_stdlib) {
    include_paths.insert(include_paths.begin(), (m_embeddedFiles.dir() / "c_bridge/include").string());
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

  llvm::legacy::PassManager pass;
  llvm::CodeGenFileType ft = llvm::CGFT_ObjectFile;

  std::error_code EC;
  std::string sloc(loc.string());
  llvm::raw_fd_ostream destination(sloc, EC, llvm::sys::fs::OF_None);

  // confusingly, addPassesToEmitFile returns `false` on success
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

  clang::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts{new clang::DiagnosticOptions()};
  clang::TextDiagnosticPrinter *DiagClient{new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts)};

  clang::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID{new clang::DiagnosticIDs()};
  clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  const std::string Path = getExecutablePath();
  clang::driver::Driver TheDriver(Path, llvm::sys::getProcessTriple(), Diags);

  TheDriver.setTitle("clang interpreter");
  TheDriver.setCheckInputsExist(false);

  // a place for the compiler arguments to live
  std::vector<std::string> str_args;
  str_args.push_back(source.string());
  std::transform(include_paths.begin(), include_paths.end(), std::back_inserter(str_args), [](const auto &str) {
    return "-I" + str.string();
  });

  str_args.emplace_back("-fsyntax-only");
  // str_args.emplace_back("-fPIC");
  str_args.emplace_back("-g");
  str_args.emplace_back("-Wno-incomplete-setjmp-declaration");
  str_args.emplace_back("-Wno-expansion-to-defined");
  str_args.emplace_back("-Wno-nullability-completeness");
  std::copy(flags.begin(), flags.end(), std::back_inserter(str_args));
  str_args.push_back(source.string());

  // the strings to pass to the compiler driver
  clang::SmallVector<const char *, 64> Args; //(argv, argv + argc);
  std::transform(
      str_args.begin(), str_args.end(), std::back_inserter(Args), [](const auto &str) { return str.c_str(); });

  std::unique_ptr<clang::driver::Compilation> compilation(TheDriver.BuildCompilation(Args));

  if (!compilation) {
    throw std::runtime_error("Unable to create compilation");
  }

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const auto &Jobs = compilation->getJobs();
  if (Jobs.size() != 1 || !clang::isa<clang::driver::Command>(*Jobs.begin())) {
    clang::SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    const auto job_string = OS.str();
    Diags.Report(clang::diag::err_fe_expected_compiler_job) << job_string;
    throw std::runtime_error(fmt::format("Failed to create exactly 1 compilation job '{}'", std::string(job_string)));
  }

  const auto &Cmd = clang::cast<clang::driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd.getCreator().getName()) != "clang") {
    Diags.Report(clang::diag::err_fe_expected_clang_command);
    throw std::runtime_error(fmt::format("Unexpected compiler job creator '{}'", Cmd.getCreator().getName()));
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
    throw std::runtime_error("Unable to constructor clang diagnostics engine");
  }

  // Infer the builtin include path if unspecified.
  // if (Clang.getHeaderSearchOpts().UseBuiltinIncludes &&
  //    Clang.getHeaderSearchOpts().ResourceDir.empty())
  Clang.getHeaderSearchOpts().ResourceDir =
      clang::CompilerInvocation::GetResourcesPath(getExecutablePath().c_str(), MainAddr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  std::unique_ptr<clang::CodeGenAction> Act(new clang::EmitLLVMOnlyAction(&ctx));

  // this one returns true on success
  if (!Clang.ExecuteAction(*Act)) {
    throw std::runtime_error("Unable to execute compilation");
  }

  return Act->takeModule();
}

} // namespace spawn
