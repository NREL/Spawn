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
  llvm::raw_fd_ostream dest(loc.c_str(), EC, llvm::sys::fs::OF_None);

  if (m_target_machine->addPassesToEmitFile(pass, dest, nullptr, ft)) {
    throw std::runtime_error("TargetMachine can't emit a file of this type");
  }

  pass.run(*m_currentCompilation);
  dest.flush();
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

static clang::DiagnosticOptions *CreateAndPopulateDiagOpts(llvm::ArrayRef<const char *> argv)
{
  auto *DiagOpts = new clang::DiagnosticOptions;
  std::unique_ptr<llvm::opt::OptTable> Opts(clang::driver::createDriverOptTable());
  unsigned MissingArgIndex, MissingArgCount;
  llvm::opt::InputArgList Args = Opts->ParseArgs(argv.slice(1), MissingArgIndex, MissingArgCount);
  // We ignore MissingArgCount and the return value of ParseDiagnosticArgs.
  // Any errors that would be diagnosed here will also be diagnosed later,
  // when the DiagnosticsEngine actually exists.
  (void)ParseDiagnosticArgs(*DiagOpts, Args);
  return DiagOpts;
}

static const char *GetStableCStr(std::set<std::string> &SavedStrings, llvm::StringRef S)
{
  return SavedStrings.insert(S).first->c_str();
}

static void insertTargetAndModeArgs(const clang::driver::ParsedClangName &NameParts,
                                    llvm::SmallVectorImpl<const char *> &ArgVector,
                                    std::set<std::string> &SavedStrings)
{
  // Put target and mode arguments at the start of argument list so that
  // arguments specified in command line could override them. Avoid putting
  // them at index 0, as an option like '-cc1' must remain the first.
  int InsertionPoint = 0;
  if (ArgVector.size() > 0) ++InsertionPoint;

  if (NameParts.DriverMode) {
    // Add the mode flag to the arguments.
    ArgVector.insert(ArgVector.begin() + InsertionPoint, GetStableCStr(SavedStrings, NameParts.DriverMode));
  }

  if (NameParts.TargetIsValid) {
    const char *arr[] = {"-target", GetStableCStr(SavedStrings, NameParts.TargetPrefix)};
    ArgVector.insert(ArgVector.begin() + InsertionPoint, std::begin(arr), std::end(arr));
  }
}

int cc1_main(llvm::ArrayRef<const char *> Argv, const char *Argv0, void *MainAddr) {
//  ensureSufficientStack();

  for (const auto &arg : Argv) {
    std::cout << "cc1_main arg: " << arg << '\n';
  }

  std::unique_ptr<clang::CompilerInstance> Clang(new clang::CompilerInstance());
  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());

  // Register the support for object-file-wrapped Clang modules.
  auto PCHOps = Clang->getPCHContainerOperations();
  PCHOps->registerWriter(llvm::make_unique<clang::ObjectFilePCHContainerWriter>());
  PCHOps->registerReader(llvm::make_unique<clang::ObjectFilePCHContainerReader>());

  // Initialize targets first, so that --version shows registered targets.
//  llvm::InitializeAllTargets();
//  llvm::InitializeAllTargetMCs();
//  llvm::InitializeAllAsmPrinters();
//  llvm::InitializeAllAsmParsers();

#ifdef LINK_POLLY_INTO_TOOLS
  llvm::PassRegistry &Registry = *llvm::PassRegistry::getPassRegistry();
  polly::initializePollyPasses(Registry);
#endif

  // Buffer diagnostics from argument parsing so that we can output them using a
  // well formed diagnostic object.
  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = new clang::DiagnosticOptions();
  clang::TextDiagnosticBuffer *DiagsBuffer = new clang::TextDiagnosticBuffer;
  clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagsBuffer);
  bool Success = clang::CompilerInvocation::CreateFromArgs(
      Clang->getInvocation(), Argv.begin(), Argv.end(), Diags);

  // Infer the builtin include path if unspecified.
  if (Clang->getHeaderSearchOpts().UseBuiltinIncludes &&
      Clang->getHeaderSearchOpts().ResourceDir.empty())
    Clang->getHeaderSearchOpts().ResourceDir =
        clang::CompilerInvocation::GetResourcesPath(Argv0, MainAddr);

  // Create the actual diagnostics engine.
  Clang->createDiagnostics();
  if (!Clang->hasDiagnostics())
    return 1;

  // Set an error handler, so that any LLVM backend diagnostics go through our
  // error handler.
  //llvm::install_fatal_error_handler(LLVMErrorHandler,
   //                                 static_cast<void*>(&Clang->getDiagnostics()));

  DiagsBuffer->FlushDiagnostics(Clang->getDiagnostics());
  if (!Success)
    return 1;

  Success = ExecuteCompilerInvocation(Clang.get());

  // If any timers were active but haven't been destroyed yet, print their
  // results now.  This happens in -disable-free mode.
  llvm::TimerGroup::printAll(llvm::errs());


  // Our error handler depends on the Diagnostics object, which we're
  // potentially about to delete. Uninstall the handler now so that any
  // later errors use the default handling behavior instead.
  llvm::remove_fatal_error_handler();

  // When running with -disable-free, don't do any destruction or shutdown.
  if (Clang->getFrontendOpts().DisableFree) {
    llvm::BuryPointer(std::move(Clang));
    return !Success;
  }

  return !Success;
}


static int ExecuteCC1Tool(llvm::ArrayRef<const char *> argv)
{
  void *GetExecutablePathVP = (void *)(intptr_t)getExecutablePath;
  return cc1_main(argv.slice(2), argv[0], GetExecutablePathVP);
}

int Compiler::invoke_compiler(const std::vector<std::string> &args)
{
  puts("0");
  for (const auto &arg : args) {
    std::cout << "0 invoke_compiler arg: " << arg << '\n';
  }

  // llvm::InitLLVM X(argc_, argv_);
  llvm::SmallVector<const char *, 256> argv{};
  std::transform(args.begin(), args.end(), std::back_inserter(argv), [](const auto &str) { return str.c_str(); });

  for (const auto &arg : argv) {
    std::cout << "1 invoke_compiler arg: " << arg << '\n';
  }

  //  if (llvm::sys::Process::FixupStandardFileDescriptors())
  //    return 1;

  //  llvm::InitializeAllTargets();
  puts("1");
  auto TargetAndMode = clang::driver::ToolChain::getTargetAndModeFromProgramName(argv[0]);

  llvm::BumpPtrAllocator A;
  llvm::StringSaver Saver(A);

  // Parse response files using the GNU syntax, unless we're in CL mode. There
  // are two ways to put clang in CL compatibility mode: argv[0] is either
  // clang-cl or cl, or --driver-mode=cl is on the command line. The normal
  // command line parsing can't happen until after response file parsing, so we
  // have to manually search for a --driver-mode=cl argument the hard way.
  // Finally, our -cc1 tools don't care which tokenization mode we use because
  // response files written by clang will tokenize the same way in either mode.
  bool ClangCLMode = false;
  if (llvm::StringRef(TargetAndMode.DriverMode).equals("--driver-mode=cl") ||
      llvm::find_if(argv, [](const char *F) { return F && strcmp(F, "--driver-mode=cl") == 0; }) != argv.end()) {
    ClangCLMode = true;
  }
  enum
  {
    Default,
    POSIX,
    Windows
  } RSPQuoting = Default;
  for (const char *F : argv) {
    if (strcmp(F, "--rsp-quoting=posix") == 0)
      RSPQuoting = POSIX;
    else if (strcmp(F, "--rsp-quoting=windows") == 0)
      RSPQuoting = Windows;
  }

  // Determines whether we want nullptr markers in argv to indicate response
  // files end-of-lines. We only use this for the /LINK driver argument with
  // clang-cl.exe on Windows.
  bool MarkEOLs = ClangCLMode;
  puts("2");
  llvm::cl::TokenizerCallback Tokenizer;
  if (RSPQuoting == Windows || (RSPQuoting == Default && ClangCLMode)) {
    Tokenizer = &llvm::cl::TokenizeWindowsCommandLine;
  } else {
    Tokenizer = &llvm::cl::TokenizeGNUCommandLine;
  }

  if (MarkEOLs && argv.size() > 1 && llvm::StringRef(argv[1]).startswith("-cc1")) {
    MarkEOLs = false;
  }

  llvm::cl::ExpandResponseFiles(Saver, Tokenizer, argv, MarkEOLs);

  puts("3");

  // Handle -cc1 integrated tools, even if -cc1 was expanded from a response
  // file.
  auto FirstArg = std::find_if(argv.begin() + 1, argv.end(), [](const char *A) { return A != nullptr; });
  if (FirstArg != argv.end() && llvm::StringRef(*FirstArg).startswith("-cc1")) {
    // If -cc1 came from a response file, remove the EOL sentinels.
    if (MarkEOLs) {
      auto newEnd = std::remove(argv.begin(), argv.end(), nullptr);
      argv.resize(newEnd - argv.begin());
    }

    for (const auto &arg : argv) {
      std::cout << "invoke_compiler arg: " << arg << '\n';
    }
    puts("4");
    return ExecuteCC1Tool(argv);
  }

  bool CanonicalPrefixes = true;
  for (int i = 1, size = argv.size(); i < size; ++i) {
    // Skip end-of-line response file markers
    if (argv[i] == nullptr) continue;
    if (llvm::StringRef(argv[i]) == "-no-canonical-prefixes") {
      CanonicalPrefixes = false;
      break;
    }
  }

  // Handle CL and _CL_ which permits additional command line options to be
  // prepended or appended.
  if (ClangCLMode) {
    throw std::runtime_error("CL mode not supported");
    /*
    // Arguments in "CL" are prepended.
    llvm::Optional<std::string> OptCL = llvm::sys::Process::GetEnv("CL");
    if (OptCL.hasValue()) {
      llvm::SmallVector<const char *, 8> PrependedOpts;
      clang::getCLEnvVarOptions(OptCL.getValue(), Saver, PrependedOpts);

      // Insert right after the program name to prepend to the argument list.
      argv.insert(argv.begin() + 1, PrependedOpts.begin(), PrependedOpts.end());
    }
    // Arguments in "_CL_" are appended.
    llvm::Optional<std::string> Opt_CL_ = llvm::sys::Process::GetEnv("_CL_");
    if (Opt_CL_.hasValue()) {
      SmallVector<const char *, 8> AppendedOpts;
      getCLEnvVarOptions(Opt_CL_.getValue(), Saver, AppendedOpts);

      // Insert at the end of the argument list to append.
      argv.append(AppendedOpts.begin(), AppendedOpts.end());
    }
     */
  }

  std::set<std::string> SavedStrings;
  // Handle CCC_OVERRIDE_OPTIONS, used for editing a command line behind the
  // scenes.
  //  if (const char *OverrideStr = ::getenv("CCC_OVERRIDE_OPTIONS")) {
  // FIXME: Driver shouldn't take extra initial argument.
  //   ApplyQAOverride(argv, OverrideStr, SavedStrings);
  // }

  std::string Path = getExecutablePath();

  llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts = CreateAndPopulateDiagOpts(argv);

  clang::TextDiagnosticPrinter *DiagClient = new clang::TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);
  //  FixupDiagPrefixExeName(DiagClient, Path);

  llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());

  clang::DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  if (!DiagOpts->DiagnosticSerializationFile.empty()) {
    throw std::runtime_error("Not supporing serialized diagnostics");
    //    auto SerializedConsumer =
    //        clang::serialized_diags::create(DiagOpts->DiagnosticSerializationFile,
    //                                        &*DiagOpts, /*MergeChildRecords=*/true);
    //    Diags.setClient(new ChainedDiagnosticConsumer(
    //        Diags.takeClient(), std::move(SerializedConsumer)));
  }

  ProcessWarningOptions(Diags, *DiagOpts, /*ReportDiags=*/false);

  clang::driver::Driver TheDriver(Path, llvm::sys::getDefaultTargetTriple(), Diags);
  // SetInstallDir(argv, TheDriver, CanonicalPrefixes);
  TheDriver.setTargetAndMode(TargetAndMode);

  insertTargetAndModeArgs(TargetAndMode, argv, SavedStrings);

  //  SetBackdoorDriverOutputsFromEnvVars(TheDriver);

  std::unique_ptr<clang::driver::Compilation> C(TheDriver.BuildCompilation(argv));
  int Res = 1;
  if (C && !C->containsError()) {
    const auto &Jobs = C->getJobs();

    int jobnum = 0;
    for (const auto &job : Jobs) {
      std::cout << "job: " << jobnum << " name: " << job.getExecutable();
      for (const auto &param : job.getArguments()) {
        std::cout << "job: " << jobnum << " param: " << param << '\n';
      }
      ++jobnum;
    }
    abort();


    llvm::SmallVector<std::pair<int, const clang::driver::Command *>, 4> FailingCommands;
    Res = TheDriver.ExecuteCompilation(*C, FailingCommands);

    // Force a crash to test the diagnostics.
    if (TheDriver.GenReproducer) {
      throw std::runtime_error("not forcing crash for diags");
      //      Diags.Report(clang::diag::err_drv_force_crash) << !::getenv("FORCE_CLANG_DIAGNOSTICS_CRASH");

      // Pretend that every command failed.
      //    FailingCommands.clear();
      //  for (const auto &J : C->getJobs())
      //  if (const Command *C = dyn_cast<Command>(&J)) FailingCommands.push_back(std::make_pair(-1, C));
    }

    for (const auto &P : FailingCommands) {
      int CommandRes = P.first;
      const auto *FailingCommand = P.second;
      if (!Res) Res = CommandRes;

      // If result status is < 0, then the driver command signalled an error.
      // If result status is 70, then the driver command reported a fatal error.
      // On Windows, abort will return an exit code of 3.  In these cases,
      // generate additional diagnostic information if possible.
      bool DiagnoseCrash = CommandRes < 0 || CommandRes == 70;
#ifdef _WIN32
      DiagnoseCrash |= CommandRes == 3;
#endif
      if (DiagnoseCrash) {
        TheDriver.generateCompilationDiagnostics(*C, *FailingCommand);
        break;
      }
    }
  }

  Diags.getClient()->finish();

  // If any timers were active but haven't been destroyed yet, print their
  // results now.  This happens in -disable-free mode.
  llvm::TimerGroup::printAll(llvm::errs());

#ifdef _WIN32
  // Exit status should not be negative on Win32, unless abnormal termination.
  // Once abnormal termiation was caught, negative status should not be
  // propagated.
  if (Res < 0) Res = 1;
#endif

  // If we have multiple failing commands, we return the result of the first
  // failing command.
  return Res;
}

} // namespace spawn
