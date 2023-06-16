include(cmake/build_and_import_open_modelica.cmake)

add_executable(
  test_open_modelica
  test/catch.cpp
  test/test_compiler.cpp
)

target_link_libraries(
  test_open_modelica
  PRIVATE
  OpenModelicaCompiler
  OpenModelicaRuntimeC
  omcgc
  Threads::Threads
  CONAN_PKG::catch2
  CONAN_PKG::fmt
  CONAN_PKG::nlohmann_json
  compile_options
)

catch_discover_tests(test_open_modelica TEST_PREFIX "spawn.open_modelica.")
