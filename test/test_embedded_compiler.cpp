#include "../util/dynamiclibrary.hpp"
#include "../util/filesystem.hpp"
#include "../util/temp_directory.hpp"

#include "../compiler/compiler.hpp"
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

TEST_CASE("Sanity Test Embedded Compiler")
{
  spdlog::set_level(spdlog::level::trace);
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, false);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << "int main() {}\n" << std::flush;
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "sanity_test_embedded_compiler");

  compiler.write_shared_object_file(object_path, {"/"}, {});

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);
}

TEST_CASE("Sanity Test Embedded Compiler with c_bridge")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  CHECK(spawn_fs::exists(compiler.get_cbridge_path()));
  CHECK(spawn_fs::is_directory(compiler.get_cbridge_path()));
  CHECK(spawn_fs::is_directory(compiler.get_cbridge_path() / "c_bridge"));

#ifdef _MSC_VER
  CHECK(spawn_fs::exists(compiler.get_cbridge_path() / "c_bridge" / "c_bridge.lib"));
  CHECK(spawn_fs::is_regular_file(compiler.get_cbridge_path() / "c_bridge" / "c_bridge.lib"));
  CHECK(spawn_fs::file_size(compiler.get_cbridge_path() / "c_bridge" / "c_bridge.lib") > 0);

  CHECK(spawn_fs::exists(compiler.get_cbridge_path() / "c_bridge" / "c_bridge.dll"));
  CHECK(spawn_fs::is_regular_file(compiler.get_cbridge_path() / "c_bridge" / "c_bridge.dll"));
  CHECK(spawn_fs::file_size(compiler.get_cbridge_path() / "c_bridge" / "c_bridge.dll") > 0);

  CHECK(spawn_fs::exists(compiler.get_cbridge_path() / "lld-link.exe"));
  CHECK(spawn_fs::is_regular_file(compiler.get_cbridge_path() / "lld-link.exe"));
  CHECK(spawn_fs::file_size(compiler.get_cbridge_path() / "lld-link.exe") > 0);

#else
  CHECK(spawn_fs::exists(compiler.get_cbridge_path() / "c_bridge" / "libc_bridge.so"));
  CHECK(spawn_fs::is_regular_file(compiler.get_cbridge_path() / "c_bridge" / "libc_bridge.so"));
  CHECK(spawn_fs::file_size(compiler.get_cbridge_path() / "c_bridge" / "libc_bridge.so") > 0);
#endif

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << "int main() {}" << std::endl; // we want a flush here
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "sanity_test_embedded_compiler");

  compiler.write_shared_object_file(object_path, {"/"}, {});

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
  spawn::Compiler compiler(include_paths, flags, true);

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
)" << std::flush;
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test_with_return");

  compiler.write_shared_object_file(object_path, {"/"}, {});

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
  spawn::Compiler compiler(include_paths, flags, true);

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
)" << std::flush;
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test_with_param");

  compiler.write_shared_object_file(object_path, {"/"}, {});

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
  spawn::Compiler compiler(include_paths, flags, false);

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
)" << std::flush;
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-cmath");

  compiler.write_shared_object_file(object_path, {"/"}, {});

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
  spawn::Compiler compiler(include_paths, flags, true);

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

DLLEXPORT double get_val(double input) {
  return input * 3.2;
}
)" << std::flush;
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-bootstrapped");

  compiler.write_shared_object_file(object_path, {"/"}, {});

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto func = dl.load_symbol<double(double)>("get_val");

  CHECK(func(-42.0) == -42.0 * 3.2);
  CHECK(func(42.0) == 42.0 * 3.2);
}

TEST_CASE("Test embedded compiler with reloaded DLL with same symbols")
{
  SECTION("First DLL")
  {
    const std::vector<spawn_fs::path> include_paths{};
    const std::vector<std::string> flags{};
    spawn::Compiler compiler(include_paths, flags, true);

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

DLLEXPORT double get_val(double input) {
  return input * 3.2;
}
)" << std::flush;
    }

    compiler.compile_and_link(test_file_path);

    const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-dll1");

    compiler.write_shared_object_file(object_path, {"/"}, {});

    CHECK(spawn_fs::exists(object_path));
    CHECK(spawn_fs::is_regular_file(object_path));

    const auto file_size = spawn_fs::file_size(object_path);

    CHECK(file_size > 0);

    spawn::util::Dynamic_Library dl(object_path);
    const auto func = dl.load_symbol<double(double)>("get_val");

    CHECK(func(-42.0) == -42.0 * 3.2);
    CHECK(func(42.0) == 42.0 * 3.2);
  }

  SECTION("Second DLL")
  {
    const std::vector<spawn_fs::path> include_paths{};
    const std::vector<std::string> flags{};
    spawn::Compiler compiler(include_paths, flags, true);

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

DLLEXPORT double get_val(double input) {
  return input * 2.3;
}
)" << std::flush;
    }

    compiler.compile_and_link(test_file_path);

    const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-dll2");

    compiler.write_shared_object_file(object_path, {"/"}, {});

    CHECK(spawn_fs::exists(object_path));
    CHECK(spawn_fs::is_regular_file(object_path));

    const auto file_size = spawn_fs::file_size(object_path);

    CHECK(file_size > 0);

    spawn::util::Dynamic_Library dl(object_path);
    const auto func = dl.load_symbol<double(double)>("get_val");

    CHECK(func(-42.0) == -42.0 * 2.3);
    CHECK(func(42.0) == 42.0 * 2.3);
  }
}

TEST_CASE("Test embedded compiler with bootstrapped DLL and stdlib")
{
  spdlog::set_level(spdlog::level::trace);
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  compiler.add_c_bridge_to_path();

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file <<
        R"(
#include "c_bridge.h"

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif


DLLEXPORT double get_cos(double input) {
  return cos(input);
}
)" << std::flush;
  }

  compiler.compile_and_link(test_file_path);

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-cmath-bootstrapped");

  compiler.write_shared_object_file(object_path, {"/"}, {});

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto func = dl.load_symbol<double(double)>("get_cos");

  CHECK(func(-42.0) == std::cos(-42.0));
  CHECK(func(42.0) == std::cos(42.0));
}

TEST_CASE("Test embedded compiler two different DLLs from one compiler")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  spawn::util::Temp_Directory td;

  auto dll1 = [&]() {
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

DLLEXPORT double get_val(double input) {
  return input * 3.2;
}
)" << std::flush;
    }

    compiler.compile_and_link(test_file_path);
    const auto object1_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-dll1");

    compiler.write_shared_object_file(object1_path, {"/"}, {});

    CHECK(spawn_fs::exists(object1_path));
    CHECK(spawn_fs::is_regular_file(object1_path));

    const auto file_size = spawn_fs::file_size(object1_path);

    CHECK(file_size > 0);

    return spawn::util::Dynamic_Library(object1_path);
  }();

  const auto func_1 = dll1.load_symbol<double(double)>("get_val");

  CHECK(func_1(-42.0) == -42.0 * 3.2);
  CHECK(func_1(42.0) == 42.0 * 3.2);

  auto dll2 = [&]() {
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

DLLEXPORT double get_val(double input) {
  return input * 2.3;
}
)" << std::flush;
    }


    // If we choose to reuse the same Compiler object, we must manually reset
    // it to avoid multiply defined symbols
    compiler.reset();

    compiler.compile_and_link(test_file_path);

    const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-dll2");

    compiler.write_shared_object_file(object_path, {"/"}, {});

    CHECK(spawn_fs::exists(object_path));
    CHECK(spawn_fs::is_regular_file(object_path));

    const auto file_size = spawn_fs::file_size(object_path);

    CHECK(file_size > 0);

    return spawn::util::Dynamic_Library(object_path);
  }();

  const auto func_2 = dll2.load_symbol<double(double)>("get_val");

  CHECK(func_2(-42.0) == -42.0 * 2.3);
  CHECK(func_2(42.0) == 42.0 * 2.3);

  CHECK(func_1(-42.0) == -42.0 * 3.2);
  CHECK(func_1(42.0) == 42.0 * 3.2);

}




TEST_CASE("Test embedded compiler source helper interface")
{
  spdlog::set_level(spdlog::level::trace);
  spawn::Compiler compiler({}, {}, true);

  compiler.add_c_bridge_to_path();

  compiler.compile_and_link(std::string_view{R"(
#include "c_bridge.h"

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif


DLLEXPORT double get_cos(double input) {
  return cos(input);
}
)"});

  spawn::util::Temp_Directory td;

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-cmath-bootstrapped");

  compiler.write_shared_object_file(object_path, {"/"}, {});

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto func = dl.load_symbol<double(double)>("get_cos");

  CHECK(func(-42.0) == std::cos(-42.0));
  CHECK(func(42.0) == std::cos(42.0));
}



TEST_CASE("Test embedded compiler c_bridge")
{
  spdlog::set_level(spdlog::level::trace);
  spawn::Compiler compiler({}, {}, true);

  compiler.add_c_bridge_to_path();

  compiler.compile_and_link(std::string_view{R"(
#include <math.h>

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

DLLEXPORT double get_cos(double input) {
  return cos(input);
}
)"});

  compiler.compile_and_link(std::string_view{R"(
#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

static char result_buffer[255];

DLLEXPORT const char * get_hello_string(const char *name, int value) {
  memset(result_buffer, 0, sizeof(result_buffer));
  snprintf(result_buffer, sizeof(result_buffer), "Hello %s, %i!", name, value);
  return result_buffer;
}
)"});


  spawn::util::Temp_Directory td;

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test");

  compiler.write_shared_object_file(object_path, {"/"}, {});

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));
  CHECK(spawn_fs::file_size(object_path) > 0);

  spawn::util::Dynamic_Library dl(object_path);
  const auto get_cos = dl.load_symbol<double(double)>("get_cos");
  const auto get_hello_string = dl.load_symbol<const char *(const char *, int)>("get_hello_string");

  CHECK(get_cos(-42.0) == std::cos(-42.0));
  CHECK(get_cos(42.0) == std::cos(42.0));
  CHECK(get_hello_string("Jason", 42) == std::string_view{"Hello Jason, 42!"});
}



