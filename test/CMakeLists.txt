# automatically enable catch2 to generate ctest targets
if (CONAN_CATCH2_ROOT_DEBUG)
  include(${CONAN_CATCH2_ROOT_DEBUG}/lib/cmake/Catch2/Catch.cmake)
else ()
  include(${CONAN_CATCH2_ROOT}/lib/cmake/Catch2/Catch.cmake)
endif ()


configure_file(paths.in.cpp paths.in2.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(example_fmu_lib MODULE example_fmu_lib.cpp)

add_executable(tests
  catch.cpp
  create_epfmu.hpp
  create_epfmu.cpp
  paths.hpp
  test_epfmi.cpp
  test_spawn_cli.cpp
  test_spawn_lib.cpp
  test_temp_directory.cpp
  test_unzip_file.cpp
  test_floating_zone.cpp
  test_single_family_house_lights_control.cpp
  test_fmi.cpp
  test_mbl_idf.cpp
  test_infiltration.cpp
)

add_dependencies(tests example_fmu_lib)
target_link_libraries(tests PRIVATE CONAN_PKG::catch2 spawn_utils fmu libspawn)


# Make this better
if(WIN32)
  # file(GENERATE...) helps us get the path the the spawn exe
  file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/$<UPPER_CASE:$<CONFIG>>/paths.cpp
          INPUT ${CMAKE_CURRENT_BINARY_DIR}/paths.in2.cpp
          )

  add_library(paths-release
          ${CMAKE_CURRENT_BINARY_DIR}/RELEASE/paths.cpp
          )

  add_library(paths-debug
          ${CMAKE_CURRENT_BINARY_DIR}/DEBUG/paths.cpp
          )

  target_link_libraries(tests
          PRIVATE
          debug paths-debug
          general paths-release
          )
else()
  # file(GENERATE...) helps us get the path the the spawn exe
  file(GENERATE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/paths.cpp
          INPUT ${CMAKE_CURRENT_BINARY_DIR}/paths.in2.cpp
          )

  add_library(paths
          ${CMAKE_CURRENT_BINARY_DIR}/paths.cpp
          )

  target_link_libraries(tests
          PRIVATE
          paths
          )
endif()

catch_discover_tests(
        tests
        TEST_PREFIX
        "unittests.")

