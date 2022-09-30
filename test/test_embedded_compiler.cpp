#include "../util/dynamiclibrary.hpp"
#include "../util/filesystem.hpp"
#include "../util/temp_directory.hpp"

#include "../compiler/compiler.hpp"
#include <catch2/catch.hpp>
#include <spdlog/spdlog.h>

//// START HERE
//
// This is the closest to how an actual use case
// should look.
//
// We simply set up the compiler, throw some C files at it,
// link those into a shared object, and load and use said
// shared object
TEST_CASE("Test embedded compiler c_bridge", "[embedded_compiler]")
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

// This test makes sure that all of the little details are as
// expected for the setup of the compiler
TEST_CASE("Sanity Test Embedded Compiler", "[embedded_compiler]")
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

// Makes sure all of the c_bridge components are installed and sane
TEST_CASE("Sanity Test Embedded Compiler with c_bridge", "[embedded_compiler]")
{
  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  CHECK(spawn_fs::exists(compiler.get_c_bridge_path()));
  CHECK(spawn_fs::is_directory(compiler.get_c_bridge_path()));
  CHECK(spawn_fs::is_directory(compiler.get_c_bridge_path() / "c_bridge"));

  const auto check_c_bridge_file = [&](const auto... components) {
    const auto path = compiler.get_c_bridge_path() / (spawn_fs::path(components) / ...);
    CHECK(spawn_fs::exists(path));
    CHECK(spawn_fs::is_regular_file(path));
    CHECK(spawn_fs::file_size(path) > 0);
  };

#ifdef _MSC_VER
  check_c_bridge_file("c_bridge", "c_bridge.lib");
  check_c_bridge_file("c_bridge", "c_bridge.dll");
  check_c_bridge_file("lld-link.exe");
#else
  check_c_bridge_file("c_bridge", "libc_bridge.so");
#endif

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << R"(
#include <linux/limits.h>

#ifndef C_BRIDGE_STDLIB
#error "wrong cstdlib!"
#endif

int main() {}
)" << std::flush;
  }

  compiler.compile_and_link(test_file_path);

  const auto test_header = [&](std::string_view header_name) {
    spdlog::info("Testing header '{}'", header_name);
    check_c_bridge_file("c_bridge", "include", header_name);
    const std::string test_file = fmt::format(R"(
#include <{}>

#ifndef C_BRIDGE_STDLIB
#error "wrong cstdlib!"
#endif
)",
                                              header_name);

    compiler.compile_and_link(std::string_view(test_file));
  };

  test_header("assert.h");
  test_header("complex.h");
  test_header("ctype.h");
  test_header("c_bridge.h");
  test_header("dlfcn.h");
  test_header("errno.h");
  test_header("fcntl.h");
  test_header("fenv.h");
  test_header("float.h");
  test_header("inttypes.h");
  test_header("iso646.h");
  test_header("limits.h");
  test_header("linux/limits.h");
  test_header("locale.h");
  test_header("math.h");
  test_header("setjmp.h");
  test_header("signal.h");
  test_header("stdalign.h");
  test_header("stdarg.h");
  test_header("stdatomic.h");
  test_header("stdbool.h");
  test_header("stddef.h");
  test_header("stdint.h");
  test_header("stdio.h");
  test_header("stdlib.h");
  test_header("stdnoreturn.h");
  test_header("string.h");
  test_header("sys/stat.h");
  test_header("sys/types.h");
  test_header("tgmath.h");
  test_header("threads.h");
  test_header("time.h");
  test_header("uchar.h");
  test_header("wchar.h");
  test_header("wctype.h");

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "sanity_test_embedded_compiler");

  compiler.write_shared_object_file(object_path, {"/"}, {});

  CHECK(spawn_fs::exists(object_path));
  CHECK(spawn_fs::is_regular_file(object_path));

  const auto file_size = spawn_fs::file_size(object_path);

  CHECK(file_size > 0);
}

// Tests loadable module that needs no dependencies, uses c_bridge
TEST_CASE("Test embedded compiler simple loadable module", "[embedded_compiler]")
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

TEST_CASE("Test embedded compiler simple loadable module with param", "[embedded_compiler]")
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

TEST_CASE("Test embedded compiler with loadable module with cmath", "[embedded_compiler]")
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

TEST_CASE("Test embedded compiler with bootstrapped DLL", "[embedded_compiler]")
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

TEST_CASE("Test embedded compiler with reloaded DLL with same symbols", "[embedded_compiler]")
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

TEST_CASE("Test embedded compiler with bootstrapped DLL and stdlib", "[embedded_compiler]")
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

TEST_CASE("Test embedded compiler two different DLLs from one compiler", "[embedded_compiler]")
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

TEST_CASE("Test embedded compiler source helper interface", "[embedded_compiler]")
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

TEST_CASE("Test compile error is catchable", "[embedded_compiler]")
{
  spdlog::set_level(spdlog::level::trace);

  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << "bassdf\n" << std::flush;
  }

  CHECK_THROWS(compiler.compile_and_link(test_file_path));
}

TEST_CASE("Test link error is catchable", "[embedded_compiler]")
{
  spdlog::set_level(spdlog::level::trace);

  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << "void func() {}\n" << std::flush;
  }

  CHECK_NOTHROW(compiler.compile_and_link(test_file_path));

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-cmath-bootstrapped");

  CHECK_THROWS(compiler.write_shared_object_file(object_path, {"/"}, {"missinglib"}));
}

TEST_CASE("Test link error is sane", "[embedded_compiler]")
{
  spdlog::set_level(spdlog::level::trace);

  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << "void func() {}\n" << std::flush;
  }

  CHECK_NOTHROW(compiler.compile_and_link(test_file_path));

  const auto object_path = spawn::Compiler::append_shared_object_extension(td.dir() / "test-cmath-bootstrapped");

  try {
    compiler.write_shared_object_file(object_path, {"/"}, {"missinglib"});
    REQUIRE_FALSE(true); // should be unreachable
  } catch (const std::exception &e) {
    // make sure compile error contains the string we know is bad.
    REQUIRE(std::string(e.what()).find("missinglib") != std::string::npos);
  }
}

TEST_CASE("Test compile error is sane", "[embedded_compiler]")
{
  spdlog::set_level(spdlog::level::trace);

  const std::vector<spawn_fs::path> include_paths{};
  const std::vector<std::string> flags{};
  spawn::Compiler compiler(include_paths, flags, true);

  spawn::util::Temp_Directory td;

  const spawn_fs::path test_file_path = td.dir() / "test.c";

  {
    std::ofstream test_file(test_file_path);
    test_file << "bassdf\n" << std::flush;
  }

  try {
    compiler.compile_and_link(test_file_path);
    REQUIRE_FALSE(true); // should be unreachable
  } catch (const std::exception &e) {
    // make sure compile error contains the string we know is bad.
    REQUIRE(std::string(e.what()).find("bassdf") != std::string::npos);
  }
}

// this tests that the library is properly unloaded as well
TEST_CASE("Test Temp Directory Cleanup", "[embedded_compiler]")
{
  spdlog::set_level(spdlog::level::trace);

  spawn_fs::path temp_directory;

  {
    const std::vector<spawn_fs::path> include_paths{};
    const std::vector<std::string> flags{};
    spawn::Compiler compiler(include_paths, flags, true);

    spawn::util::Temp_Directory td;
    temp_directory = td.dir();

    const spawn_fs::path test_file_path = td.dir() / "test.c";

    {
      std::ofstream test_file(test_file_path);
      test_file << "void func() {}\n" << std::flush;
    }

    compiler.compile_and_link(test_file_path);

    const auto object_path =
        spawn::Compiler::append_shared_object_extension(td.dir() / "sanity_test_embedded_compiler");

    compiler.write_shared_object_file(object_path, {"/"}, {});

    CHECK(spawn_fs::exists(object_path));
    CHECK(spawn_fs::is_regular_file(object_path));

    const auto file_size = spawn_fs::file_size(object_path);

    CHECK(file_size > 0);

    CHECK(spawn_fs::exists(temp_directory));
    CHECK(spawn_fs::is_directory(temp_directory));

    spawn::util::Dynamic_Library dl(object_path);
  }

  CHECK(!spawn_fs::exists(temp_directory));
}
