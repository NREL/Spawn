if(ENABLE_COMPILER)
  include(cmake/build_and_import_open_modelica.cmake)

  add_library(
    spawn_open_modelica
    open_modelica_engine.hpp
    open_modelica_engine.cpp
  )

  target_compile_options(
    spawn_open_modelica
    PRIVATE
    -Wno-error=missing-field-initializers
  )

  target_link_libraries(
    spawn_open_modelica
    PUBLIC
    spawn_utils
    spawn_mbl
    OpenModelicaCompiler
    OpenModelicaRuntimeC
    omcgc
    Catch2::Catch2
    fmt::fmt
    nlohmann_json::nlohmann_json
    compile_options
  )
  
  add_executable(
    test_open_modelica
    test/catch.cpp
    test/test_compiler.cpp
  )

  target_compile_options(
    test_open_modelica
    PRIVATE
    -Wno-error=missing-field-initializers
  )
  
  target_link_libraries(
    test_open_modelica
    PRIVATE
    spawn_open_modelica
  )
  
  catch_discover_tests(test_open_modelica TEST_PREFIX "spawn.open_modelica.")
endif()
