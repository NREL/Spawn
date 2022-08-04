#include "../util/dynamiclibrary.hpp"
#include "../util/filesystem.hpp"
#include "../util/temp_directory.hpp"

#include "../compiler/compiler.hpp"
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

TEST_CASE("Sanity Test Embedded Compiler")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << "int main() {}" << std::endl; // we want a flush here
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "sanity_test_embedded_compiler");

  compiler.write_shared_object_file(object_path, td.dir(), {}, true);

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);
}

TEST_CASE("Test embedded compiler simple loadable module")
{
  spdlog::set_level(spdlog::level::trace);
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  spawn::util::Temp_Directory td;


  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);

    test_file << R"(
#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

DLLEXPORT int get_value() { 
  return 42;
}
)" << std::endl; // we want a flush here
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test_with_return");

  compiler.write_shared_object_file(object_path, td.dir(), {}, true);

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);

  CHECK_THROWS(dl.load_symbol<void()>("unknown_symbol"));

  const auto func = dl.load_symbol<int()>("get_value");

  CHECK(func() == 42);
}

TEST_CASE("Test embedded compiler simple loadable module with param")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << R"(
#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

DLLEXPORT int get_value_1(int input) { 
  return 42 * input;
}
)" << std::endl; // we want a flush here
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test_with_param");

  compiler.write_shared_object_file(object_path, td.dir(), {}, true);

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto func = dl.load_symbol<int(int)>("get_value_1");

  CHECK(func(4) == 42 * 4);
}

TEST_CASE("Test embedded compiler with loadable module with cmath")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    // Note: fabs was ruled out because it gets eliminated in the resulting binary
    test_file <<
        R"(
#include <math.h>

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

DLLEXPORT double get_cos(double input) { return cos(input); }
)" << std::endl; // we want a flush here
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-cmath");

  compiler.write_shared_object_file(object_path, td.dir(), {}, true);

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto func = dl.load_symbol<double(double)>("get_cos");

  CHECK(func(-42.0) == std::cos(-42.0));
  CHECK(func(42.0) == std::cos(42.0));
}


TEST_CASE("Test embedded compiler with bootstrapped DLL")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  const auto c = cos(42.0);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file <<
        R"(
#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)

#define BOOL int
#define WINAPI __stdcall
#define __cdecl __attribute__((__cdecl__))

// just fake it
BOOL WINAPI _DllMainCRTStartup(void *hinstDLL, unsigned long fdwReason, void *lpReserved)
{
  return 1;
}


#else
#define DLLEXPORT
#endif

DLLEXPORT double get_val(double input) {
  return input * 3.2;
}
)" << std::endl; // we want a flush here
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-bootstrapped");

  compiler.write_shared_object_file(object_path, td.dir(), {}, false);

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto func = dl.load_symbol<double(double)>("get_val");

  CHECK(func(-42.0) == -42.0 * 3.2);
  CHECK(func(42.0) == 42.0 * 3.2);
}


TEST_CASE("Test embedded compiler with bootstrapped DLL and stdlib")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags);

  const auto c = cos(42.0);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

 
  {
    std::ofstream test_file(test_file_path);
    test_file <<
        R"(
#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#define BOOL int
#define WINAPI __stdcall
#define __cdecl __attribute__((__cdecl__))

BOOL WINAPI _DllMainCRTStartup(void *hinstDLL, unsigned long fdwReason, void *lpReserved)
{
  return 1;
}

double __cdecl cos(double);

DLLEXPORT double get_cos(double input) {
  return cos(input);
}
)" << std::endl; // we want a flush here
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-cmath-bootstrapped");

  compiler.write_shared_object_file(object_path, td.dir(), {}, false);

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto func = dl.load_symbol<double(double)>("get_cos");

  CHECK(func(-42.0) == std::cos(-42.0));
  CHECK(func(42.0) == std::cos(42.0));
}

